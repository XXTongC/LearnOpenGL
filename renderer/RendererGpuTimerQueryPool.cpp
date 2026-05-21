#include "RendererGpuTimerQueryPool.h"

#include <algorithm>
#include <utility>

#include "core.h"
#include "RendererFrameStats.h"

using namespace GLframework;

namespace
{
	void accumulateGpuPassTime(
		RendererFrameStats& stats,
		RendererFramePassKey key,
		std::uint64_t elapsedNs
	)
	{
		stats.rendererGpuTimingAvailable = true;
		++stats.rendererGpuTimedPassCount;
		stats.rendererGpuFrameTimeNs += elapsedNs;

		switch (key)
		{
		case RendererFramePassKey::BeginFrame:
			stats.rendererGpuBeginFrameTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::ShadowMaps:
			stats.rendererGpuShadowMapsTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRShadowAtlas:
			stats.rendererGpuPbrShadowAtlasTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRDepthPrepass:
			stats.rendererGpuPbrDepthPrepassTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRGBuffer:
			stats.rendererGpuPbrGBufferTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRDeferredLighting:
			stats.rendererGpuPbrDeferredLightingTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRDeferredTiledLightDebug:
			stats.rendererGpuPbrDeferredTiledLightDebugTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRDeferredClusteredLightDebug:
			stats.rendererGpuPbrDeferredClusteredLightDebugTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRGBufferDebug:
			stats.rendererGpuPbrGBufferDebugTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBROpaqueScene:
			stats.rendererGpuPbrOpaqueSceneTimeNs += elapsedNs;
			break;
		case RendererFramePassKey::PBRTransparentScene:
			stats.rendererGpuPbrTransparentSceneTimeNs += elapsedNs;
			break;
		default:
			break;
		}
	}
}

RendererGpuTimerQueryPool::~RendererGpuTimerQueryPool()
{
	reset();
}

void RendererGpuTimerQueryPool::beginFrame(bool enabled, RendererFrameStats& stats)
{
	if (!enabled)
	{
		reset();
		return;
	}

	mEnabled = true;
	mCurrentFrameQueries.clear();
	stats.rendererGpuTimingEnabled = true;
	stats.rendererGpuTimingDeferredReadback = true;

	collectOldestAvailableFrame(stats);
	updatePendingStats(stats);
}

unsigned int RendererGpuTimerQueryPool::beginPass(RendererFramePassKey key)
{
	if (!mEnabled || mOpenQueryId != 0)
	{
		return 0;
	}

	const unsigned int queryId = acquireQuery();
	if (queryId == 0)
	{
		return 0;
	}

	mOpenQueryId = queryId;
	mOpenQueryKey = key;
	glBeginQuery(GL_TIME_ELAPSED, queryId);
	return queryId;
}

void RendererGpuTimerQueryPool::endPass(unsigned int queryId)
{
	if (!mEnabled || queryId == 0 || queryId != mOpenQueryId)
	{
		return;
	}

	glEndQuery(GL_TIME_ELAPSED);
	mCurrentFrameQueries.push_back({ queryId, mOpenQueryKey });
	mOpenQueryId = 0;
	mOpenQueryKey = RendererFramePassKey::BeginFrame;
}

void RendererGpuTimerQueryPool::endFrame(RendererFrameStats& stats)
{
	if (!mEnabled)
	{
		return;
	}

	if (!mCurrentFrameQueries.empty())
	{
		mPendingFrames.push_back(std::move(mCurrentFrameQueries));
		mCurrentFrameQueries.clear();
	}

	updatePendingStats(stats);
}

void RendererGpuTimerQueryPool::reset()
{
	if (mOpenQueryId != 0)
	{
		glEndQuery(GL_TIME_ELAPSED);
		releaseQuery(mOpenQueryId);
		mOpenQueryId = 0;
	}

	releaseFrameQueries(mCurrentFrameQueries);
	while (!mPendingFrames.empty())
	{
		releaseFrameQueries(mPendingFrames.front());
		mPendingFrames.pop_front();
	}

	if (!mReusableQueries.empty())
	{
		glDeleteQueries(static_cast<GLsizei>(mReusableQueries.size()), mReusableQueries.data());
		mReusableQueries.clear();
	}

	mEnabled = false;
}

unsigned int RendererGpuTimerQueryPool::acquireQuery()
{
	if (!mReusableQueries.empty())
	{
		const unsigned int queryId = mReusableQueries.back();
		mReusableQueries.pop_back();
		return queryId;
	}

	GLuint queryId = 0;
	glGenQueries(1, &queryId);
	return queryId;
}

void RendererGpuTimerQueryPool::collectOldestAvailableFrame(RendererFrameStats& stats)
{
	if (mPendingFrames.empty())
	{
		return;
	}

	auto& queries = mPendingFrames.front();
	const bool complete = std::all_of(queries.begin(), queries.end(), [](const PendingQuery& query)
	{
		GLint available = GL_FALSE;
		glGetQueryObjectiv(query.id, GL_QUERY_RESULT_AVAILABLE, &available);
		return available == GL_TRUE;
	});
	if (!complete)
	{
		return;
	}

	for (const auto& query : queries)
	{
		GLuint64 elapsedNs = 0;
		glGetQueryObjectui64v(query.id, GL_QUERY_RESULT, &elapsedNs);
		accumulateGpuPassTime(stats, query.key, static_cast<std::uint64_t>(elapsedNs));
		releaseQuery(query.id);
	}
	mPendingFrames.pop_front();
}

void RendererGpuTimerQueryPool::releaseQuery(unsigned int queryId)
{
	if (queryId != 0)
	{
		mReusableQueries.push_back(queryId);
	}
}

void RendererGpuTimerQueryPool::releaseFrameQueries(std::vector<PendingQuery>& queries)
{
	for (const auto& query : queries)
	{
		releaseQuery(query.id);
	}
	queries.clear();
}

void RendererGpuTimerQueryPool::updatePendingStats(RendererFrameStats& stats) const
{
	int pendingQueries = static_cast<int>(mCurrentFrameQueries.size());
	for (const auto& frame : mPendingFrames)
	{
		pendingQueries += static_cast<int>(frame.size());
	}
	stats.rendererGpuTimingPendingQueries = pendingQueries;
}

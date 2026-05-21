#pragma once

#include <cstdint>
#include <deque>
#include <vector>

#include "RendererFramePassRegistry.h"

namespace GLframework
{
	struct RendererFrameStats;

	class RendererGpuTimerQueryPool
	{
	public:
		~RendererGpuTimerQueryPool();

		void beginFrame(bool enabled, RendererFrameStats& stats);
		unsigned int beginPass(RendererFramePassKey key);
		void endPass(unsigned int queryId);
		void endFrame(RendererFrameStats& stats);
		void reset();

	private:
		struct PendingQuery
		{
			unsigned int id{ 0 };
			RendererFramePassKey key{ RendererFramePassKey::BeginFrame };
		};

		unsigned int acquireQuery();
		void collectOldestAvailableFrame(RendererFrameStats& stats);
		void releaseQuery(unsigned int queryId);
		void releaseFrameQueries(std::vector<PendingQuery>& queries);
		void updatePendingStats(RendererFrameStats& stats) const;

		std::deque<std::vector<PendingQuery>> mPendingFrames{};
		std::vector<PendingQuery> mCurrentFrameQueries{};
		std::vector<unsigned int> mReusableQueries{};
		unsigned int mOpenQueryId{ 0 };
		RendererFramePassKey mOpenQueryKey{ RendererFramePassKey::BeginFrame };
		bool mEnabled{ false };
	};
}

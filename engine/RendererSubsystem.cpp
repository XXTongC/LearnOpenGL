#include "engine/RendererSubsystem.h"

#include <memory>
#include <utility>

#include "engine/EngineContext.h"
#include "engine/RendererBackendRegistryTypes.h"
#include "engine/RendererSubsystemBackendSlot.h"
#include "engine/RendererSubsystemFrameBridgeState.h"
#include "engine/RendererSubsystemFrameExecutionBridge.h"
#include "renderer/RendererFrameStats.h"
#include "renderer/renderer.h"

using namespace GLengine;

RendererSubsystem::RendererSubsystem(GLframework::Renderer* renderer)
	: mRenderer(renderer)
	, mBackendSlot(std::make_unique<RendererSubsystemBackendSlot>())
	, mFrameExecutionBridge(std::make_unique<RendererSubsystemFrameExecutionBridge>())
	, mFrameBridgeState(std::make_unique<RendererSubsystemFrameBridgeState>())
{
}

RendererSubsystem::~RendererSubsystem() = default;

bool RendererSubsystem::initialize(EngineContext& context)
{
	mTickCount = 0;
	mInitialized = true;
	refreshFrameBridgeStats(context, false);
	return true;
}

void RendererSubsystem::tick(EngineContext& context)
{
	++mTickCount;
	refreshFrameBridgeStats(context, false);
}

void RendererSubsystem::shutdown(EngineContext& context)
{
	mInitialized = false;
	clearRendererBackend();
	mRenderer = nullptr;
	mTickCount = 0;
	refreshFrameBridgeStats(context, false);
}

const char* RendererSubsystem::getDebugName() const
{
	return "RendererSubsystem";
}

bool RendererSubsystem::isInitializedForDiagnostics() const
{
	return mInitialized;
}

int RendererSubsystem::getTickCountForDiagnostics() const
{
	return getTickCount();
}

void RendererSubsystem::setRenderer(GLframework::Renderer* renderer)
{
	mRenderer = renderer;
	mFrameBridgeState->setRendererObservation(hasRenderer(), getObservedRendererPassCount());
}

GLframework::Renderer* RendererSubsystem::getRenderer() const
{
	return mRenderer;
}

bool RendererSubsystem::hasRenderer() const
{
	return mInitialized && mRenderer != nullptr;
}

const GLframework::RendererFrameStats* RendererSubsystem::getLastFrameStats() const
{
	return mRenderer ? &mRenderer->getLastFrameStats() : nullptr;
}

void RendererSubsystem::setRendererBackend(std::unique_ptr<RendererBackend> rendererBackend)
{
	setRendererBackend(std::move(rendererBackend), RendererBackendAttachmentDesc{});
}

void RendererSubsystem::setRendererBackend(
	std::unique_ptr<RendererBackend> rendererBackend,
	RendererBackendAttachmentDesc attachmentDesc
)
{
	const auto change = mBackendSlot->setBackend(std::move(rendererBackend), attachmentDesc);
	mFrameBridgeState->recordRendererBackendAttachmentChange(change.detached, change.attached);
	mFrameBridgeState->refreshRendererBackendStats(
		mBackendSlot->captureSnapshot()
	);
}

void RendererSubsystem::clearRendererBackend()
{
	setRendererBackend(nullptr);
}

RendererBackend* RendererSubsystem::getRendererBackend() const
{
	return mBackendSlot->getBackend();
}

bool RendererSubsystem::hasRendererBackend() const
{
	return mBackendSlot->hasBackend();
}

void RendererSubsystem::renderFrameBridge(
	const EngineContext& context,
	const RendererFrameIntent& frameIntent
)
{
	mFrameBridgeState->recordRenderFrameBridgeCall();
	beginFrameBridge(context, frameIntent);
	const bool backendReady = isRendererBackendReady();
	if (backendReady)
	{
		mFrameBridgeState->recordRendererBackendReadyFrame();
	}
	else
	{
		mFrameBridgeState->recordRendererBackendNotReadyFrame();
	}

	try
	{
		const auto executionResult = mFrameExecutionBridge->executeFrame(
			mBackendSlot->getBackend(),
			backendReady,
			context,
			frameIntent
		);
		if (executionResult.frameExecuted)
		{
			mFrameBridgeState->recordRendererBackendFrameCall();
		}
		mFrameBridgeState->applyFrameResult(executionResult.frameResult);
	}
	catch (...)
	{
		endFrameBridge(context, frameIntent);
		throw;
	}
	endFrameBridge(context, frameIntent);
}

void RendererSubsystem::beginFrameBridge(const EngineContext& context, const RendererFrameIntent& frameIntent)
{
	mFrameBridgeState->recordBeginFrame();
	mFrameBridgeState->applyFrameIntent(frameIntent);
	refreshFrameBridgeStats(context, mInitialized && mRenderer != nullptr && isRendererBackendReady());
}

void RendererSubsystem::endFrameBridge(const EngineContext& context, const RendererFrameIntent& frameIntent)
{
	mFrameBridgeState->recordCompletedFrame();
	mFrameBridgeState->applyFrameIntent(frameIntent);
	refreshFrameBridgeStats(context, mInitialized && mRenderer != nullptr && isRendererBackendReady());
}

const RendererSubsystemFrameBridgeStats& RendererSubsystem::getFrameBridgeStats() const
{
	return mFrameBridgeState->getStats();
}

int RendererSubsystem::getTickCount() const
{
	return mTickCount;
}

bool RendererSubsystem::isRendererBackendReady() const
{
	return mBackendSlot->isBackendReady();
}

int RendererSubsystem::getObservedRendererPassCount() const
{
	return mRenderer ? mRenderer->getLastFrameStats().rendererPassCount : 0;
}

void RendererSubsystem::refreshFrameBridgeStats(const EngineContext& context, bool frameBridgeActive)
{
	mFrameBridgeState->refreshFrameBridgeStats(
		context,
		mInitialized,
		hasRenderer(),
		frameBridgeActive,
		getObservedRendererPassCount(),
		mBackendSlot->captureSnapshot()
	);
}

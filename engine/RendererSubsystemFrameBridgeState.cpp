#include "engine/RendererSubsystemFrameBridgeState.h"

#include "engine/EngineContext.h"
#include "engine/RendererBackendFrameTypes.h"
#include "engine/RendererSubsystemBackendSlot.h"

using namespace GLengine;

namespace
{
	const char* makeRendererBackendState(bool attached, bool ready)
	{
		if (!attached)
		{
			return "detached";
		}

		return ready ? "ready" : "attached-not-ready";
	}
}

const RendererSubsystemFrameBridgeStats& RendererSubsystemFrameBridgeState::getStats() const
{
	return mStats;
}

void RendererSubsystemFrameBridgeState::setRendererObservation(bool hasRenderer, int observedRendererPasses)
{
	mStats.hasRenderer = hasRenderer;
	mStats.observedRendererPasses = observedRendererPasses;
}

void RendererSubsystemFrameBridgeState::recordRendererBackendAttachmentChange(bool detached, bool attached)
{
	if (detached)
	{
		++mStats.rendererBackendDetachCount;
	}
	if (attached)
	{
		++mStats.rendererBackendAttachCount;
	}
}

void RendererSubsystemFrameBridgeState::recordRenderFrameBridgeCall()
{
	++mStats.renderFrameBridgeCallCount;
}

void RendererSubsystemFrameBridgeState::recordRendererBackendFrameCall()
{
	++mStats.rendererBackendFrameCallCount;
}

void RendererSubsystemFrameBridgeState::recordRendererBackendReadyFrame()
{
	++mStats.rendererBackendReadyFrameCount;
}

void RendererSubsystemFrameBridgeState::recordRendererBackendNotReadyFrame()
{
	++mStats.rendererBackendNotReadyFrameCount;
}

void RendererSubsystemFrameBridgeState::recordBeginFrame()
{
	++mStats.beginFrameCount;
}

void RendererSubsystemFrameBridgeState::recordCompletedFrame()
{
	++mStats.completedFrameCount;
}

void RendererSubsystemFrameBridgeState::applyFrameIntent(const RendererFrameIntent& frameIntent)
{
	mStats.framebufferWidth = frameIntent.framebufferWidth;
	mStats.framebufferHeight = frameIntent.framebufferHeight;
	mStats.frameConfigValid = frameIntent.framebufferWidth > 0 && frameIntent.framebufferHeight > 0;
}

void RendererSubsystemFrameBridgeState::applyFrameResult(const RendererFrameResult& frameResult)
{
	mStats.framePlanKey = frameResult.framePlanKey.empty()
		? "none"
		: frameResult.framePlanKey;
	mStats.plannedPassCount = frameResult.plannedPassCount;
	mStats.executedPassCount = frameResult.executedPassCount;
	mStats.skippedPassCount = frameResult.skippedPassCount;
}

void RendererSubsystemFrameBridgeState::refreshRendererBackendStats(
	const RendererSubsystemBackendSlotSnapshot& backendSnapshot
)
{
	const bool attached = backendSnapshot.attached;
	const bool ready = backendSnapshot.ready;
	mStats.rendererBackendAttached = attached;
	mStats.rendererBackendReady = ready;
	mStats.rendererBackendKey = backendSnapshot.backendKey.empty() ? "none" : backendSnapshot.backendKey;
	mStats.rendererBackendState = makeRendererBackendState(attached, ready);
	mStats.rendererBackendOwnerKey = attached ? backendSnapshot.attachmentDesc.ownerKey : "none";
	mStats.rendererBackendOwnership = attached ? backendSnapshot.attachmentDesc.ownership : "detached";
	mStats.rendererBackendRegistryKey = attached ? backendSnapshot.attachmentDesc.registryKey : "none";
	mStats.rendererBackendRegistryCount = attached ? backendSnapshot.attachmentDesc.registryBackendCount : 0;
}

void RendererSubsystemFrameBridgeState::refreshFrameBridgeStats(
	const EngineContext& context,
	bool initialized,
	bool hasRenderer,
	bool frameBridgeActive,
	int observedRendererPasses,
	const RendererSubsystemBackendSlotSnapshot& backendSnapshot
)
{
	mStats.initialized = initialized;
	mStats.hasRenderer = hasRenderer;
	refreshRendererBackendStats(backendSnapshot);
	mStats.frameBridgeActive = frameBridgeActive;
	mStats.observedRendererPasses = observedRendererPasses;
	mStats.lastDeltaSeconds = context.deltaSeconds;
	mStats.engineTimeSeconds = context.timeSeconds;
}

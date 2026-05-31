#include "engine/RendererSubsystemFrameExecutionBridge.h"

using namespace GLengine;

RendererSubsystemFrameExecutionResult RendererSubsystemFrameExecutionBridge::executeFrame(
	RendererBackend* backend,
	bool backendReady,
	const EngineContext& context,
	const RendererFrameIntent& frameIntent
) const
{
	RendererSubsystemFrameExecutionResult result{};
	result.backendReady = backendReady && backend != nullptr;
	if (result.backendReady)
	{
		result.frameExecuted = true;
		result.frameResult = backend->renderFrame(context, frameIntent);
	}
	return result;
}

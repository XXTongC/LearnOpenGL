#pragma once

#include "engine/RendererBackendFrameTypes.h"

namespace GLengine
{
	struct EngineContext;
	class RendererBackend;

	struct RendererSubsystemFrameExecutionResult
	{
		bool backendReady{ false };
		bool frameExecuted{ false };
		RendererFrameResult frameResult{};
	};

	class RendererSubsystemFrameExecutionBridge
	{
	public:
		RendererSubsystemFrameExecutionResult executeFrame(
			RendererBackend* backend,
			bool backendReady,
			const EngineContext& context,
			const RendererFrameIntent& frameIntent
		) const;
	};
}

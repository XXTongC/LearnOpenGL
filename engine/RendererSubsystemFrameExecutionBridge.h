#pragma once

#include "engine/RendererBackend.h"

namespace GLengine
{
	struct EngineContext;

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

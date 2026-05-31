#pragma once

namespace GL_RUNTIME
{
	struct RuntimeFrameClockConfig
	{
		bool useFixedDelta{ false };
		float fixedDeltaSeconds{ 1.0f / 60.0f };
		float maxDeltaSeconds{ 0.25f };
	};
}

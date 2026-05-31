#pragma once

#include <chrono>

#include "RuntimeFrameClockTypes.h"

namespace GL_RUNTIME
{
	class RuntimeFrameClock
	{
	public:
		void reset();
		float tick(const RuntimeFrameClockConfig& config);

	private:
		using Clock = std::chrono::steady_clock;

		Clock::time_point mLastTick{};
		bool mHasLastTick{ false };
	};
}

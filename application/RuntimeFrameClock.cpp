#include "RuntimeFrameClock.h"

namespace
{
	float sanitizeDelta(float deltaSeconds)
	{
		return deltaSeconds > 0.0f ? deltaSeconds : (1.0f / 60.0f);
	}
}

namespace GL_RUNTIME
{
	void RuntimeFrameClock::reset()
	{
		mLastTick = Clock::now();
		mHasLastTick = true;
	}

	float RuntimeFrameClock::tick(const RuntimeFrameClockConfig& config)
	{
		const float fallbackDelta = sanitizeDelta(config.fixedDeltaSeconds);
		if (config.useFixedDelta)
		{
			if (!mHasLastTick)
			{
				reset();
			}
			return fallbackDelta;
		}

		const auto now = Clock::now();
		if (!mHasLastTick)
		{
			mLastTick = now;
			mHasLastTick = true;
			return fallbackDelta;
		}

		const std::chrono::duration<float> elapsed = now - mLastTick;
		mLastTick = now;
		float deltaSeconds = sanitizeDelta(elapsed.count());
		const float maxDeltaSeconds = sanitizeDelta(config.maxDeltaSeconds);
		if (deltaSeconds > maxDeltaSeconds)
		{
			deltaSeconds = maxDeltaSeconds;
		}
		return deltaSeconds;
	}
}

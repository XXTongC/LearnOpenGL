#include "RuntimeFrameClock.h"

#include <chrono>

#include "RuntimeFrameClockTypes.h"

namespace
{
	float sanitizeDelta(float deltaSeconds)
	{
		return deltaSeconds > 0.0f ? deltaSeconds : (1.0f / 60.0f);
	}
}

namespace GL_RUNTIME
{
	struct RuntimeFrameClock::Impl
	{
		using Clock = std::chrono::steady_clock;

		Clock::time_point lastTick{};
		bool hasLastTick{ false };
	};

	RuntimeFrameClock::RuntimeFrameClock()
		: mImpl(std::make_unique<Impl>())
	{
	}

	RuntimeFrameClock::~RuntimeFrameClock() = default;
	RuntimeFrameClock::RuntimeFrameClock(RuntimeFrameClock&&) noexcept = default;
	RuntimeFrameClock& RuntimeFrameClock::operator=(RuntimeFrameClock&&) noexcept = default;

	void RuntimeFrameClock::reset()
	{
		mImpl->lastTick = Impl::Clock::now();
		mImpl->hasLastTick = true;
	}

	float RuntimeFrameClock::tick(const RuntimeFrameClockConfig& config)
	{
		const float fallbackDelta = sanitizeDelta(config.fixedDeltaSeconds);
		if (config.useFixedDelta)
		{
			if (!mImpl->hasLastTick)
			{
				reset();
			}
			return fallbackDelta;
		}

		const auto now = Impl::Clock::now();
		if (!mImpl->hasLastTick)
		{
			mImpl->lastTick = now;
			mImpl->hasLastTick = true;
			return fallbackDelta;
		}

		const std::chrono::duration<float> elapsed = now - mImpl->lastTick;
		mImpl->lastTick = now;
		float deltaSeconds = sanitizeDelta(elapsed.count());
		const float maxDeltaSeconds = sanitizeDelta(config.maxDeltaSeconds);
		if (deltaSeconds > maxDeltaSeconds)
		{
			deltaSeconds = maxDeltaSeconds;
		}
		return deltaSeconds;
	}
}

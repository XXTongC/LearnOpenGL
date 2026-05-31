#pragma once

#include <memory>

namespace GL_RUNTIME
{
	struct RuntimeFrameClockConfig;

	class RuntimeFrameClock
	{
	public:
		RuntimeFrameClock();
		~RuntimeFrameClock();

		RuntimeFrameClock(const RuntimeFrameClock&) = delete;
		RuntimeFrameClock& operator=(const RuntimeFrameClock&) = delete;
		RuntimeFrameClock(RuntimeFrameClock&&) noexcept;
		RuntimeFrameClock& operator=(RuntimeFrameClock&&) noexcept;

		void reset();
		float tick(const RuntimeFrameClockConfig& config);

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}

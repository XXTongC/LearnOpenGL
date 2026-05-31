#pragma once

#include <memory>

namespace GL_RUNTIME
{
	class RuntimeFrameClock;

	class RuntimeFrameLifecycleState
	{
	public:
		RuntimeFrameLifecycleState();
		~RuntimeFrameLifecycleState();

		RuntimeFrameLifecycleState(const RuntimeFrameLifecycleState&) = delete;
		RuntimeFrameLifecycleState& operator=(const RuntimeFrameLifecycleState&) = delete;

		RuntimeFrameClock& frameClock();
		const RuntimeFrameClock& frameClock() const;

		int renderedFrameCount() const;
		void resetRenderedFrameCount();
		void incrementRenderedFrameCount();

		bool& verificationCaptureWritten();
		void resetVerificationCaptureWritten();

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl{};
	};
}

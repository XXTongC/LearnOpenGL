#include "RuntimeFrameLifecycleState.h"

#include "RuntimeFrameClock.h"

namespace GL_RUNTIME
{
	struct RuntimeFrameLifecycleState::Impl
	{
		RuntimeFrameClock frameClock{};
		int renderedFrameCount{ 0 };
		bool verificationCaptureWritten{ false };
	};

	RuntimeFrameLifecycleState::RuntimeFrameLifecycleState()
		: mImpl(std::make_unique<Impl>())
	{
	}

	RuntimeFrameLifecycleState::~RuntimeFrameLifecycleState() = default;

	RuntimeFrameClock& RuntimeFrameLifecycleState::frameClock()
	{
		return mImpl->frameClock;
	}

	const RuntimeFrameClock& RuntimeFrameLifecycleState::frameClock() const
	{
		return mImpl->frameClock;
	}

	int RuntimeFrameLifecycleState::renderedFrameCount() const
	{
		return mImpl->renderedFrameCount;
	}

	void RuntimeFrameLifecycleState::resetRenderedFrameCount()
	{
		mImpl->renderedFrameCount = 0;
	}

	void RuntimeFrameLifecycleState::incrementRenderedFrameCount()
	{
		++mImpl->renderedFrameCount;
	}

	bool& RuntimeFrameLifecycleState::verificationCaptureWritten()
	{
		return mImpl->verificationCaptureWritten;
	}

	void RuntimeFrameLifecycleState::resetVerificationCaptureWritten()
	{
		mImpl->verificationCaptureWritten = false;
	}
}

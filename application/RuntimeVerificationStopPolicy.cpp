#include "RuntimeVerificationStopPolicy.h"

#include "RuntimeVerificationConfig.h"

namespace GL_RUNTIME
{
	bool RuntimeVerificationStopPolicy::shouldStopAfterFrames(
		const RuntimeVerificationConfig& config,
		int renderedFrameCount
	)
	{
		return config.enabled && renderedFrameCount >= config.maxFrames;
	}
}

#pragma once

namespace GL_RUNTIME
{
	struct RuntimeVerificationConfig;

	class RuntimeVerificationStopPolicy
	{
	public:
		static bool shouldStopAfterFrames(
			const RuntimeVerificationConfig& config,
			int renderedFrameCount
		);
	};
}

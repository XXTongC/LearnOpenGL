#pragma once

#include <string>

namespace GL_RUNTIME
{
	class RuntimeVerificationCapture
	{
	public:
		static bool captureDefaultFramebuffer(
			const std::string& path,
			unsigned int width,
			unsigned int height
		);
	};
}

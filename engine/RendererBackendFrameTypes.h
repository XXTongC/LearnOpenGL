#pragma once

#include <string>

namespace GLengine
{
	struct RendererFrameIntent
	{
		unsigned int framebufferWidth{ 0 };
		unsigned int framebufferHeight{ 0 };
	};

	struct RendererFrameResult
	{
		std::string framePlanKey{ "none" };
		int plannedPassCount{ 0 };
		int executedPassCount{ 0 };
		int skippedPassCount{ 0 };
	};
}

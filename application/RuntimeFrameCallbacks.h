#pragma once

#include <functional>

namespace GL_RUNTIME
{
	struct RuntimeFrameCallbacks
	{
		std::function<void()> renderUi{};
	};
}

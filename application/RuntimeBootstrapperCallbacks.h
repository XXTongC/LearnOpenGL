#pragma once

#include <functional>

namespace GL_RUNTIME
{
	struct RuntimeBootstrapperCallbacks
	{
		std::function<bool()> initialize{};
		std::function<bool()> shouldContinue{};
		std::function<void()> runFrame{};
		std::function<void()> cleanup{};
		std::function<void()> destroy{};
	};
}

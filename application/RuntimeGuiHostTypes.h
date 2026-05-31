#pragma once

#include <functional>

struct GLFWwindow;

namespace GL_RUNTIME
{
	struct RuntimeGuiInitContext
	{
		GLFWwindow* window{ nullptr };
		const char* glslVersion{ "#version 460" };
	};

	struct RuntimeGuiFrameContext
	{
		GLFWwindow* window{ nullptr };
		std::function<void()> drawPanels{};
	};
}

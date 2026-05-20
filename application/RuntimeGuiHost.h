#pragma once

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
		void (*drawPanels)(){ nullptr };
	};

	class RuntimeGuiHost
	{
	public:
		static void initialize(const RuntimeGuiInitContext& context);
		static void renderFrame(const RuntimeGuiFrameContext& context);
	};
}

#pragma once

struct GLFWwindow;

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeWindowConfig
	{
		int width{ 1200 };
		int height{ 900 };
	};

	struct RuntimeWindowSnapshot
	{
		int width{ 0 };
		int height{ 0 };
		GLFWwindow* nativeWindow{ nullptr };
	};

	struct RuntimeWindowCallbackContext
	{
		GLframework::AppRuntimeContext* runtime{ nullptr };
		int* width{ nullptr };
		int* height{ nullptr };
	};
}

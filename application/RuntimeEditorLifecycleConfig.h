#pragma once

struct GLFWwindow;

namespace GL_RUNTIME
{
	struct RuntimeEditorLifecycleConfig
	{
		bool enableGui{ true };
		GLFWwindow* window{ nullptr };
		float* editorOrbitAngle{ nullptr };
	};
}

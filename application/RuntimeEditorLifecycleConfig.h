#pragma once

#include "../tools/editor/EditorUiModuleComposition.h"

struct GLFWwindow;

namespace GL_RUNTIME
{
	struct RuntimeEditorLifecycleConfig
	{
		bool enableGui{ true };
		GLFWwindow* window{ nullptr };
		float* editorOrbitAngle{ nullptr };
		GL_EDITOR::EditorUiModuleCompositionPolicy editorUiModulePolicy{};
	};
}

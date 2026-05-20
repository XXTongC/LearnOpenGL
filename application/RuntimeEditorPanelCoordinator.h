#pragma once

#include "AppRuntimeContext.h"
#include "../tools/editor/DebugControllerPanel.h"
#include "../tools/editor/EditorPanels.h"

namespace GL_RUNTIME
{
	class RuntimeEditorPanelCoordinator
	{
	public:
		static GL_EDITOR::DebugControllerContext makeDebugControllerContext(
			GLframework::AppRuntimeContext& context,
			float* orbitAngle
		);

		static GL_EDITOR::EditorPanelContext makeEditorPanelContext(
			GLframework::AppRuntimeContext& context
		);

		static void drawPanels(
			GLframework::AppRuntimeContext& context,
			GL_EDITOR::SelectionContext& selection,
			float* orbitAngle
		);
	};
}

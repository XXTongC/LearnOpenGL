#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_EDITOR
{
	struct DebugControllerContext;
	struct EditorPanelContext;
	class EditTransactionLog;
	struct SelectionContext;
}

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
			GLframework::AppRuntimeContext& context,
			GL_EDITOR::EditTransactionLog& editTransactions
		);

		static void drawPanels(
			GLframework::AppRuntimeContext& context,
			GL_EDITOR::SelectionContext& selection,
			GL_EDITOR::EditTransactionLog& editTransactions,
			float* orbitAngle
		);
	};
}

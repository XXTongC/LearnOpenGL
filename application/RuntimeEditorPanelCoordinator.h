#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_EDITOR
{
	struct DebugControllerContext;
	struct EditorPanelContext;
	struct EditorUiModuleRegistries;
	class EditTransactionLog;
	struct SelectionContext;
}

namespace GL_RUNTIME
{
	class RuntimeEditorLifecycleState;

	class RuntimeEditorPanelCoordinator
	{
	public:
		static GL_EDITOR::DebugControllerContext makeDebugControllerContext(
			GLframework::AppRuntimeContext& context,
			RuntimeEditorLifecycleState& editorState,
			float* orbitAngle
		);

		static GL_EDITOR::EditorPanelContext makeEditorPanelContext(
			GLframework::AppRuntimeContext& context,
			GL_EDITOR::EditTransactionLog& editTransactions,
			const GL_EDITOR::EditorUiModuleRegistries& editorUiModules
		);

		static void drawPanels(
			GLframework::AppRuntimeContext& context,
			RuntimeEditorLifecycleState& editorState,
			float* orbitAngle
		);
	};
}

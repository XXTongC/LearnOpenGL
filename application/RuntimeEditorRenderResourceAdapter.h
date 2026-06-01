#pragma once

namespace GL_EDITOR
{
	struct DebugControllerContext;
	struct EditorPanelContext;
	struct SelectionContext;
}

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState;

	class RuntimeEditorRenderResourceAdapter
	{
	public:
		static void applyDebugControllerResources(
			const RuntimeRenderResourceState& renderResources,
			GL_EDITOR::DebugControllerContext& editorContext
		);

		static void applyEditorPanelResources(
			const RuntimeRenderResourceState& renderResources,
			GL_EDITOR::EditorPanelContext& editorContext
		);

		static void ensureDefaultSelection(
			const RuntimeRenderResourceState& renderResources,
			GL_EDITOR::SelectionContext& selection
		);
	};
}

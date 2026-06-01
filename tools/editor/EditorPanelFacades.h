#pragma once

namespace GL_EDITOR
{
	struct EditorPanelContext;
	struct SelectionContext;

	void drawHierarchyPanel(const EditorPanelContext& context, SelectionContext& selection);
	void drawAssetBrowserPanel(const EditorPanelContext& context, SelectionContext& selection);
	void drawSelectionInspectorPanel(const EditorPanelContext& context, SelectionContext& selection);
}

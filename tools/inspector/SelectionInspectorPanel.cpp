#include "../editor/EditorPanelContext.h"
#include "../editor/EditorPanelFacades.h"
#include "../editor/EditorSelectionState.h"
#include "../editor/EditorUiModuleRegistry.h"
#include "../../third_party/imgui/imgui.h"

void GL_EDITOR::drawSelectionInspectorPanel(const EditorPanelContext& context, SelectionContext& selection)
{
	ImGui::Begin("inspector");

	SelectionInspectorProviderContext providerContext{ context, selection };
	const auto& editorUiModules = context.editorUiModules ? *context.editorUiModules : defaultEditorUiModuleRegistries();
	if (!editorUiModules.selectionInspectors.drawFirst(providerContext))
	{
		ImGui::TextUnformatted("No target selected.");
	}
	ImGui::End();
}

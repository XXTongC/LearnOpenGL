#include "../editor/EditorPanelContext.h"
#include "../editor/EditorPanelFacades.h"
#include "../editor/EditorSelectionState.h"
#include "SelectionInspectorProviders.h"
#include "../../third_party/imgui/imgui.h"

void GL_EDITOR::drawSelectionInspectorPanel(const EditorPanelContext& context, SelectionContext& selection)
{
	ImGui::Begin("inspector");

	SelectionInspectorProviderContext providerContext{ context, selection };
	if (!getDefaultSelectionInspectorProviderRegistry().drawFirst(providerContext))
	{
		ImGui::TextUnformatted("No target selected.");
	}
	ImGui::End();
}

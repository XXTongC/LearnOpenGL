#include "DebugControllerPanel.h"

#include "EditorUiModuleRegistry.h"
#include "../../third_party/imgui/imgui.h"

void GL_EDITOR::drawDebugControllerPanel(const DebugControllerContext& context)
{
	ImGui::Begin("controller");

	GL_EDITOR::defaultEditorUiModuleRegistries().debugControllerSections.drawAll(context);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

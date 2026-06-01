#include "DebugControllerPanel.h"

#include "DebugControllerContext.h"
#include "EditorUiModuleRegistry.h"
#include "../../third_party/imgui/imgui.h"

void GL_EDITOR::drawDebugControllerPanel(const DebugControllerContext& context)
{
	ImGui::Begin("controller");

	const auto& editorUiModules = context.editorUiModules ? *context.editorUiModules : GL_EDITOR::defaultEditorUiModuleRegistries();
	editorUiModules.debugControllerSections.drawAll(context);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

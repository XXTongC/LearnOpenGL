#include "EditorUiModuleDiagnosticsSection.h"

#include "DebugControllerContext.h"
#include "DebugControllerSectionRegistry.h"
#include "DebugSectionRegistration.h"
#include "EditorUiModuleRegistry.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
	constexpr int kEditorUiModuleDiagnosticsOrder = 850;

	void drawEditorUiModuleDiagnostics(const GL_EDITOR::DebugControllerContext& context)
	{
		if (!ImGui::CollapsingHeader("Editor UI Modules"))
		{
			return;
		}

		const auto* modules = context.editorUiModules;
		if (!modules)
		{
			ImGui::TextUnformatted("No injected editor UI module registries; static fallback may be active.");
			return;
		}

		ImGui::Text("Active Modules: %d", static_cast<int>(modules->activeModuleKeys.size()));
		if (modules->activeModuleKeys.empty())
		{
			ImGui::TextUnformatted("No active editor UI modules.");
		}
		else
		{
			for (const auto& key : modules->activeModuleKeys)
			{
				ImGui::BulletText("%s", key.c_str());
			}
		}

		ImGui::Separator();
		ImGui::Text("Debug Controller Sections: %d", static_cast<int>(modules->debugControllerSections.sectionCount()));
		ImGui::Text("Pipeline Profile Sections: %d", static_cast<int>(modules->pipelineProfileControls.sectionCount()));
		ImGui::Text("Scene Profile Sections: %d", static_cast<int>(modules->sceneProfileControls.sectionCount()));
		ImGui::Text("Selection Inspector Providers: %d", static_cast<int>(modules->selectionInspectors.providerCount()));
	}
}

void GL_EDITOR::registerEditorUiModuleDiagnosticsSection(DebugControllerSectionRegistry& registry)
{
	registerRequiredDebugSection(registry, {
		"editor-ui-modules",
		kEditorUiModuleDiagnosticsOrder,
		drawEditorUiModuleDiagnostics
	});
}

#include "SampleEditorUiModule.h"

#include "DebugControllerContext.h"
#include "DebugSectionRegistration.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
	constexpr int kSampleDiagnosticsOrder = 900;

	const char* availabilityLabel(const bool available)
	{
		return available ? "Available" : "Missing";
	}

	void drawSampleEditorModuleDiagnostics(const GL_EDITOR::DebugControllerContext& context)
	{
		if (!ImGui::CollapsingHeader("Sample Editor UI Module"))
		{
			return;
		}

		ImGui::TextUnformatted("Registered through EditorUiModuleList without changing panel code.");
		ImGui::Separator();
		ImGui::Text("Engine: %s", availabilityLabel(context.engine != nullptr));
		ImGui::Text("Engine World: %s", availabilityLabel(context.engineWorld != nullptr));
		ImGui::Text("Asset Subsystem: %s", availabilityLabel(context.assetSubsystem != nullptr));
		ImGui::Text("Renderer Subsystem: %s", availabilityLabel(context.rendererSubsystem != nullptr));
		ImGui::Text("Renderer Handle: %s", availabilityLabel(context.renderer != nullptr));
		ImGui::Text("Point Light Slots: %d", context.pointLights ? static_cast<int>(context.pointLights->size()) : 0);
	}
}

GL_EDITOR::EditorUiModule GL_EDITOR::sampleEditorUiModule()
{
	return { "sample-editor-ui", registerSampleEditorUiModule };
}

void GL_EDITOR::registerSampleEditorUiModule(EditorUiModuleRegistries& registries)
{
	registerRequiredDebugSection(registries.debugControllerSections, {
		"sample-editor-ui-module",
		kSampleDiagnosticsOrder,
		drawSampleEditorModuleDiagnostics
	});
}

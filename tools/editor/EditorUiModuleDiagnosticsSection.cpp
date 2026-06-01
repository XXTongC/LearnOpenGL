#include "EditorUiModuleDiagnosticsSection.h"

#include "DebugControllerContext.h"
#include "DebugControllerSectionRegistry.h"
#include "DebugSectionRegistration.h"
#include "EditorUiModulePolicyDiagnostics.h"
#include "EditorUiModuleRegistry.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
	constexpr int kEditorUiModuleDiagnosticsOrder = 850;

	const char* toEnabledText(bool value)
	{
		return value ? "enabled" : "disabled";
	}

	const char* toYesNoText(bool value)
	{
		return value ? "yes" : "no";
	}

	void drawPolicySnapshot(
		const char* label,
		const GL_EDITOR::EditorUiModulePolicySnapshot& snapshot
	)
	{
		ImGui::Text(
			"%s: Core=%s, Sample=%s",
			label,
			toEnabledText(snapshot.includeCoreEditorUi),
			toEnabledText(snapshot.includeSampleEditorUi)
		);
	}

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

		ImGui::Separator();
		ImGui::TextUnformatted("Policy Diagnostics");
		const auto* diagnostics = context.editorUiModulePolicyDiagnostics;
		if (!diagnostics)
		{
			ImGui::TextUnformatted("No injected editor UI module policy diagnostics.");
			return;
		}

		drawPolicySnapshot("Active Policy", diagnostics->activePolicy);
		if (diagnostics->hasLastAppliedPolicy)
		{
			drawPolicySnapshot("Last Applied Policy", diagnostics->lastAppliedPolicy);
		}
		else
		{
			ImGui::TextUnformatted("Last Applied Policy: none");
		}
		if (diagnostics->hasPendingPolicy)
		{
			drawPolicySnapshot("Pending Policy", diagnostics->pendingPolicy);
		}
		else
		{
			ImGui::TextUnformatted("Pending Policy: none");
		}
		ImGui::Text("Registry Builds: %d", diagnostics->registryBuildCount);
		ImGui::Text("Reapply Requests: %d", diagnostics->reapplyRequestCount);
		ImGui::Text("Reapply Applies: %d", diagnostics->reapplyApplyCount);
		ImGui::Text(
			"Last Apply Rebuilt Registries: %s",
			toYesNoText(diagnostics->lastApplyRebuiltRegistries)
		);
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

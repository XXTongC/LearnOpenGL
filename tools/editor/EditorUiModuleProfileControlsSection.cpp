#include "EditorUiModuleProfileControlsSection.h"

#include "DebugControllerContext.h"
#include "DebugControllerSectionRegistry.h"
#include "DebugSectionRegistration.h"
#include "EditorUiModuleProfile.h"
#include "EditorUiModuleProfileConfig.h"
#include "../../third_party/imgui/imgui.h"
#include "../inspector/PropertyInspector.h"

#include <string>

namespace
{
	constexpr int kEditorUiModuleProfileControlsOrder = 860;

	void drawEditorUiModuleProfileControls(const GL_EDITOR::DebugControllerContext& context)
	{
		if (!ImGui::CollapsingHeader("Editor UI Module Profile"))
		{
			return;
		}

		if (!context.editorUiModuleProfile)
		{
			ImGui::TextUnformatted("Editor UI module profile is not available in this context.");
			return;
		}

		static std::string lastProfileStatus{};
		const std::string configPath = context.editorUiModuleProfilePath
			? *context.editorUiModuleProfilePath
			: GL_EDITOR::EditorUiModuleProfileStorage::defaultPath();

		ImGui::TextWrapped("Profile File: %s", configPath.c_str());
		ImGui::TextWrapped("Save/reload changes the profile. Apply queues a safe registry rebuild after the current UI frame.");

		GL_EDITOR::PropertyBuilder builder{};
		GL_EDITOR::buildEditorUiModuleProfileConfigSchema(builder, *context.editorUiModuleProfile);
		GL_EDITOR::drawProperties(builder);

		if (!context.editorUiModuleProfile->enableCoreEditorUiModule)
		{
			ImGui::TextWrapped("Warning: applying with Core Editor UI disabled removes this controls section until the profile or CLI enables it again.");
		}

		if (ImGui::Button("Save Editor UI Module Profile"))
		{
			lastProfileStatus = GL_EDITOR::EditorUiModuleProfileStorage::saveToFile(
				configPath,
				*context.editorUiModuleProfile
			)
				? "Editor UI module profile saved."
				: "Editor UI module profile save failed.";
		}
		ImGui::SameLine();
		if (ImGui::Button("Reload Editor UI Module Profile"))
		{
			lastProfileStatus = GL_EDITOR::EditorUiModuleProfileStorage::loadFromFile(
				configPath,
				*context.editorUiModuleProfile
			)
				? "Editor UI module profile reloaded. Apply it to rebuild the active module registries."
				: "Editor UI module profile reload failed.";
		}
		ImGui::SameLine();
		if (!context.requestEditorUiModuleProfileApply)
		{
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Apply Profile To Active Modules"))
		{
			const bool willRebuild = context.requestEditorUiModuleProfileApply
				? context.requestEditorUiModuleProfileApply(*context.editorUiModuleProfile)
				: false;
			lastProfileStatus = willRebuild
				? "Editor UI module registry reapply queued after this UI frame."
				: "Editor UI module registries already match the current profile.";
		}
		if (!context.requestEditorUiModuleProfileApply)
		{
			ImGui::EndDisabled();
			ImGui::TextWrapped("Runtime reapply is unavailable in this context.");
		}

		if (!lastProfileStatus.empty())
		{
			ImGui::TextWrapped("%s", lastProfileStatus.c_str());
		}
	}
}

void GL_EDITOR::registerEditorUiModuleProfileControlsSection(DebugControllerSectionRegistry& registry)
{
	registerRequiredDebugSection(registry, {
		"editor-ui-module-profile",
		kEditorUiModuleProfileControlsOrder,
		drawEditorUiModuleProfileControls
	});
}

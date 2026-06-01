#include "DebugPipelineProfileControlSections.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>

#include "DebugControllerContext.h"
#include "DebugProfileControlSectionRegistry.h"
#include "../../application/RuntimeFramePipelineProfile.h"
#include "../../application/RuntimeFramePipelineProfileConfig.h"
#include "../../renderer/PostProcessSettings.h"
#include "../../renderer/PostProcessSettingsConfig.h"
#include "../../renderer/RendererFramePassProfile.h"
#include "../../renderer/RendererFramePassProfileConfig.h"
#include "../../renderer/renderer.h"
#include "../../third_party/imgui/imgui.h"
#include "../inspector/PropertyInspector.h"

namespace
{
	constexpr int kPostProcessOrder = 100;
	constexpr int kRuntimeFramePipelineOrder = 200;
	constexpr int kRendererFramePassOrder = 300;

	void registerDefaultProfileSection(
		GL_EDITOR::DebugProfileControlSectionRegistry& registry,
		GL_EDITOR::DebugProfileControlSection section
	)
	{
		const bool registered = registry.registerSection(std::move(section));
		assert(registered && registry.lastRegistrationFailure().empty());
	}

	void drawPostProcessControls(
		GLframework::PostProcessSettings* settings,
		const std::string* settingsPath
	)
	{
		if (!settings)
		{
			return;
		}

		static std::string lastConfigStatus{};
		const std::string configPath = settingsPath ? *settingsPath : GLframework::PostProcessSettingsStorage::defaultPath();
		if (ImGui::CollapsingHeader("Post Process", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextWrapped("Profile File: %s", configPath.c_str());

			GL_EDITOR::PropertyBuilder builder{};
			GLframework::buildPostProcessSettingsConfigSchema(builder, *settings);
			GL_EDITOR::drawProperties(builder);

			if (ImGui::Button("Save Post Process Profile"))
			{
				lastConfigStatus = GLframework::PostProcessSettingsStorage::saveToFile(configPath, *settings)
					? "Post process profile saved."
					: "Post process profile save failed.";
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload Post Process Profile"))
			{
				lastConfigStatus = GLframework::PostProcessSettingsStorage::loadFromFile(configPath, *settings)
					? "Post process profile reloaded."
					: "Post process profile reload failed.";
			}

			if (!lastConfigStatus.empty())
			{
				ImGui::TextWrapped("%s", lastConfigStatus.c_str());
			}
		}
	}

	void drawFramePipelineControls(
		GL_RUNTIME::RuntimeFramePipelineProfile* profile,
		const std::string* profilePath
	)
	{
		if (!profile)
		{
			return;
		}

		static std::string lastConfigStatus{};
		const std::string configPath = profilePath ? *profilePath : GL_RUNTIME::RuntimeFramePipelineProfileStorage::defaultPath();
		if (ImGui::CollapsingHeader("Runtime Frame Pipeline"))
		{
			ImGui::TextWrapped("Profile File: %s", configPath.c_str());
			ImGui::TextWrapped("These toggles are intended for render-path debugging. Disabling required passes may leave stale frame textures visible.");

			GL_EDITOR::PropertyBuilder builder{};
			GL_RUNTIME::buildRuntimeFramePipelineProfileConfigSchema(builder, *profile);
			GL_EDITOR::drawProperties(builder);

			if (ImGui::Button("Save Frame Pipeline Profile"))
			{
				lastConfigStatus = GL_RUNTIME::RuntimeFramePipelineProfileStorage::saveToFile(configPath, *profile)
					? "Frame pipeline profile saved."
					: "Frame pipeline profile save failed.";
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload Frame Pipeline Profile"))
			{
				lastConfigStatus = GL_RUNTIME::RuntimeFramePipelineProfileStorage::loadFromFile(configPath, *profile)
					? "Frame pipeline profile reloaded."
					: "Frame pipeline profile reload failed.";
			}

			if (!lastConfigStatus.empty())
			{
				ImGui::TextWrapped("%s", lastConfigStatus.c_str());
			}
		}
	}

	void drawRendererFramePassControls(
		const std::shared_ptr<GLframework::Renderer>& renderer,
		const std::string* profilePath
	)
	{
		if (!renderer)
		{
			return;
		}

		static std::string lastConfigStatus{};
		auto& profile = renderer->getFramePassProfile();
		const std::string configPath = profilePath
			? *profilePath
			: GLframework::RendererFramePassProfileStorage::defaultPath();
		if (ImGui::CollapsingHeader("Renderer Frame Pass Plan"))
		{
			ImGui::TextWrapped("Profile File: %s", configPath.c_str());
			ImGui::TextWrapped("This controls the renderer-internal scene pass order. Invalid or empty plans fall back to the built-in default plan.");

			GL_EDITOR::PropertyBuilder builder{};
			GLframework::buildRendererFramePassProfileConfigSchema(builder, profile);
			GL_EDITOR::drawProperties(builder);

			if (ImGui::Button("Save Renderer Pass Profile"))
			{
				lastConfigStatus = GLframework::RendererFramePassProfileStorage::saveToFile(configPath, profile)
					? "Renderer pass profile saved."
					: "Renderer pass profile save failed.";
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload Renderer Pass Profile"))
			{
				lastConfigStatus = GLframework::RendererFramePassProfileStorage::loadFromFile(configPath, profile)
					? "Renderer pass profile reloaded."
					: "Renderer pass profile reload failed.";
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset Renderer Pass Defaults"))
			{
				profile.resetToDefaults();
				lastConfigStatus = "Renderer pass profile reset to built-in defaults.";
			}

			if (!lastConfigStatus.empty())
			{
				ImGui::TextWrapped("%s", lastConfigStatus.c_str());
			}
		}
	}
}

void GL_EDITOR::registerDefaultDebugPipelineProfileControlSections(DebugProfileControlSectionRegistry& registry)
{
	registerDefaultProfileSection(registry, {
		"post-process",
		kPostProcessOrder,
		[](const DebugControllerContext& context)
		{
			drawPostProcessControls(context.postProcessSettings, context.postProcessSettingsPath);
		}
	});

	registerDefaultProfileSection(registry, {
		"runtime-frame-pipeline",
		kRuntimeFramePipelineOrder,
		[](const DebugControllerContext& context)
		{
			drawFramePipelineControls(context.framePipelineProfile, context.framePipelineProfilePath);
		}
	});

	registerDefaultProfileSection(registry, {
		"renderer-frame-pass",
		kRendererFramePassOrder,
		[](const DebugControllerContext& context)
		{
			drawRendererFramePassControls(context.renderer, context.rendererFramePassProfilePath);
		}
	});
}

#include "DebugSceneProfileControlSections.h"

#include <memory>
#include <string>

#include "DebugControllerContext.h"
#include "DebugProfileControlSectionRegistry.h"
#include "DebugSectionRegistration.h"
#include "../../materials/pbrMaterial/PBRMaterialProfile.h"
#include "../../renderer/EnvironmentProfile.h"
#include "../../renderer/EnvironmentProfileConfig.h"
#include "../../renderer/EnvironmentRenderTargets.h"
#include "../../renderer/renderer.h"
#include "../../third_party/imgui/imgui.h"
#include "../inspector/PropertyInspector.h"
#include "../sceneSetup/PBRCameraRigProfile.h"
#include "../sceneSetup/PBRExperimentProfile.h"
#include "../sceneSetup/PBRLightRigProfile.h"
#include "../sceneSetup/PBRPreviewProfile.h"
#include "../sceneSetup/PBRPreviewProfileConfig.h"

namespace
{
	constexpr int kPBRPreviewOrder = 100;
	constexpr int kPBRExperimentOrder = 200;
	constexpr int kEnvironmentIblOrder = 300;

	void drawEnvironmentControls(
		const std::shared_ptr<GLframework::Renderer>& renderer,
		GLframework::EnvironmentProfile* profile,
		const std::string* profilePath
	)
	{
		if (!profile)
		{
			return;
		}

		static std::string lastPrecomputeStatus{};
		static std::string lastConfigStatus{};
		const std::string configPath = profilePath ? *profilePath : GLframework::EnvironmentProfileStorage::defaultPath();

		if (ImGui::CollapsingHeader("Environment / IBL", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextWrapped("Profile File: %s", configPath.c_str());

			GL_EDITOR::PropertyBuilder builder{};
			GLframework::buildEnvironmentProfileConfigSchema(builder, *profile);
			GL_EDITOR::drawProperties(builder);

			const bool ready = renderer && renderer->getEnvironmentRenderTargets().hasPrecomputedEnvironment();
			ImGui::Text("IBL Ready: %s", ready ? "Yes" : "No");

			if (ImGui::Button("Save Environment Profile"))
			{
				lastConfigStatus = GLframework::EnvironmentProfileStorage::saveToFile(configPath, *profile)
					? "Environment profile saved."
					: "Environment profile save failed.";
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload Environment Profile"))
			{
				if (GLframework::EnvironmentProfileStorage::loadFromFile(configPath, *profile))
				{
					lastConfigStatus = "Environment profile reloaded.";
				}
				else
				{
					lastConfigStatus = "Environment profile reload failed.";
				}
			}

			if (!lastConfigStatus.empty())
			{
				ImGui::TextWrapped("%s", lastConfigStatus.c_str());
			}

			const bool canPrecompute = renderer != nullptr && profile->hasEnvironmentSource();
			if (!canPrecompute)
			{
				ImGui::BeginDisabled();
			}

			if (ImGui::Button("Precompute IBL Now"))
			{
				lastPrecomputeStatus = renderer->precomputeEnvironment(*profile)
					? "IBL precompute finished."
					: "IBL precompute failed.";
			}

			if (!canPrecompute)
			{
				ImGui::EndDisabled();
				ImGui::TextWrapped("Set an HDR path or enable procedural environment before precomputing.");
			}

			if (!lastPrecomputeStatus.empty())
			{
				ImGui::TextWrapped("%s", lastPrecomputeStatus.c_str());
			}
		}
	}

	void drawPBRPreviewControls(
		GL_SCENE::PBRPreviewProfile* profile,
		const std::string* profilePath
	)
	{
		if (!profile)
		{
			return;
		}

		static std::string lastConfigStatus{};
		static std::string lastMaterialConfigStatus{};
		const std::string configPath = profilePath ? *profilePath : GL_SCENE::PBRPreviewProfileStorage::defaultPath();
		if (ImGui::CollapsingHeader("PBR Preview Profile"))
		{
			ImGui::TextWrapped("Profile File: %s", configPath.c_str());
			ImGui::TextWrapped("Geometry and grid edits are applied when the preview scene is prepared. Save the profile and restart/reprepare to rebuild preview objects.");

			GL_EDITOR::PropertyBuilder builder{};
			GL_SCENE::buildPBRPreviewProfileConfigSchema(builder, *profile);
			GL_EDITOR::drawProperties(builder);

			if (ImGui::Button("Save PBR Preview Profile"))
			{
				lastConfigStatus = GL_SCENE::PBRPreviewProfileStorage::saveToFile(configPath, *profile)
					? "PBR preview profile saved."
					: "PBR preview profile save failed.";
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload PBR Preview Profile"))
			{
				lastConfigStatus = GL_SCENE::PBRPreviewProfileStorage::loadFromFile(configPath, *profile)
					? "PBR preview profile reloaded."
					: "PBR preview profile reload failed.";
			}

			if (!lastConfigStatus.empty())
			{
				ImGui::TextWrapped("%s", lastConfigStatus.c_str());
			}

			const std::string materialConfigPath = profile->materialProfilePath.empty()
				? GLframework::PBRMaterialProfileStorage::defaultPath()
				: profile->materialProfilePath;
			ImGui::Separator();
			ImGui::TextWrapped("Material Preset File: %s", materialConfigPath.c_str());
			if (ImGui::Button("Save PBR Material Profile"))
			{
				if (profile->materialProfilePath.empty())
				{
					profile->materialProfilePath = materialConfigPath;
				}
				lastMaterialConfigStatus = GLframework::PBRMaterialProfileStorage::saveToFile(materialConfigPath, profile->material)
					? "PBR material profile saved."
					: "PBR material profile save failed.";
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload PBR Material Profile"))
			{
				if (profile->materialProfilePath.empty())
				{
					profile->materialProfilePath = materialConfigPath;
				}
				lastMaterialConfigStatus = GLframework::PBRMaterialProfileStorage::loadFromFile(materialConfigPath, profile->material)
					? "PBR material profile reloaded."
					: "PBR material profile reload failed.";
			}

			if (!lastMaterialConfigStatus.empty())
			{
				ImGui::TextWrapped("%s", lastMaterialConfigStatus.c_str());
			}
		}
	}

	void drawPBRExperimentControls(const GL_EDITOR::DebugControllerContext& context)
	{
		if (
			!context.environmentProfile ||
			!context.postProcessSettings ||
			!context.pbrPreviewProfile ||
			!context.lightRigProfile ||
			!context.cameraRigProfile ||
			!context.ambientLight ||
			!context.directionalLight ||
			!context.spotLight ||
			!context.pointLights
		)
		{
			return;
		}

		static std::string lastConfigStatus{};
		const std::string configPath = context.pbrExperimentProfilePath
			? *context.pbrExperimentProfilePath
			: GL_SCENE::PBRExperimentProfileStorage::defaultPath();
		if (ImGui::CollapsingHeader("PBR Experiment Preset"))
		{
			ImGui::TextWrapped("Preset File: %s", configPath.c_str());
			ImGui::TextWrapped("Saves or reloads the combined environment, postprocess, PBR preview, light rig, and camera rig profile.");

			if (ImGui::Button("Save PBR Experiment Preset"))
			{
				context.lightRigProfile->copyFrom(
					*context.ambientLight,
					*context.directionalLight,
					*context.spotLight,
					*context.pointLights
				);
				context.cameraRigProfile->copyFrom(context.mainCamera);
				lastConfigStatus = GL_SCENE::PBRExperimentProfileStorage::saveToFile(
					configPath,
					*context.environmentProfile,
					*context.postProcessSettings,
					*context.pbrPreviewProfile,
					*context.lightRigProfile,
					*context.cameraRigProfile
				)
					? "PBR experiment preset saved."
					: "PBR experiment preset save failed.";
			}
			ImGui::SameLine();
			if (ImGui::Button("Reload PBR Experiment Preset"))
			{
				lastConfigStatus = GL_SCENE::PBRExperimentProfileStorage::loadFromFile(
					configPath,
					*context.environmentProfile,
					*context.postProcessSettings,
					*context.pbrPreviewProfile,
					*context.lightRigProfile,
					*context.cameraRigProfile
				)
					? "PBR experiment preset reloaded."
					: "PBR experiment preset reload failed.";
				if (lastConfigStatus == "PBR experiment preset reloaded.")
				{
					context.lightRigProfile->applyTo(
						*context.ambientLight,
						*context.directionalLight,
						*context.spotLight,
						*context.pointLights
					);
					context.cameraRigProfile->applyTo(context.mainCamera);
				}
			}

			if (!lastConfigStatus.empty())
			{
				ImGui::TextWrapped("%s", lastConfigStatus.c_str());
			}
		}
	}
}

void GL_EDITOR::registerDefaultDebugSceneProfileControlSections(DebugProfileControlSectionRegistry& registry)
{
	registerRequiredDebugSection(registry, {
		"pbr-preview",
		kPBRPreviewOrder,
		[](const DebugControllerContext& context)
		{
			drawPBRPreviewControls(context.pbrPreviewProfile, context.pbrPreviewProfilePath);
		}
	});

	registerRequiredDebugSection(registry, {
		"pbr-experiment",
		kPBRExperimentOrder,
		[](const DebugControllerContext& context)
		{
			drawPBRExperimentControls(context);
		}
	});

	registerRequiredDebugSection(registry, {
		"environment-ibl",
		kEnvironmentIblOrder,
		[](const DebugControllerContext& context)
		{
			drawEnvironmentControls(context.renderer, context.environmentProfile, context.environmentProfilePath);
		}
	});
}

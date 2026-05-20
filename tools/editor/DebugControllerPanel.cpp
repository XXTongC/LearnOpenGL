#include "DebugControllerPanel.h"

#include <string>

#include "../../renderer/EnvironmentProfile.h"
#include "../../renderer/renderer.h"
#include "../../light/directionalLight.h"
#include "../../light/pointLight.h"
#include "../../light/shadow/shadow.h"
#include "../../mesh/mesh.h"
#include "../../renderer/PostProcessSettings.h"
#include "../../third_party/imgui/imgui.h"
#include "../inspector/PropertyInspector.h"
#include "../sceneSetup/PBRExperimentProfile.h"
#include "../sceneSetup/PBRPreviewProfile.h"

namespace
{
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
			profile->visitEditableProperties(builder);
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
			settings->visitEditableProperties(builder);
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
			profile->visitEditableProperties(builder);
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
		if (!context.environmentProfile || !context.postProcessSettings || !context.pbrPreviewProfile)
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
			ImGui::TextWrapped("Saves or reloads the combined environment, postprocess, and PBR preview profile.");

			if (ImGui::Button("Save PBR Experiment Preset"))
			{
				lastConfigStatus = GL_SCENE::PBRExperimentProfileStorage::saveToFile(
					configPath,
					*context.environmentProfile,
					*context.postProcessSettings,
					*context.pbrPreviewProfile
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
					*context.pbrPreviewProfile
				)
					? "PBR experiment preset reloaded."
					: "PBR experiment preset reload failed.";
			}

			if (!lastConfigStatus.empty())
			{
				ImGui::TextWrapped("%s", lastConfigStatus.c_str());
			}
		}
	}
}

void GL_EDITOR::drawDebugControllerPanel(const DebugControllerContext& context)
{
	ImGui::Begin("controller");

	if (context.directionalLight)
	{
		auto pos = context.directionalLight->getPosition();
		if (ImGui::SliderFloat("light.x", &pos.x, 0.0f, 50.0f, "%.2f"))
		{
			context.directionalLight->setPosition(pos);
		}

		if (context.directionalLight->getShadow())
		{
			ImGui::SliderFloat("tightness", &context.directionalLight->getShadow()->mDiskTightness, 0.0f, 1.0f, "%.3f");
			ImGui::SliderFloat("pcfRadius", &context.directionalLight->getShadow()->mPcfRadius, 0.0f, 10.0f, "%.3f");
		}
	}

	if (context.textObject)
	{
		float rotate = context.textObject->getAngleX();
		if (ImGui::SliderFloat("Text Rotate:", &rotate, -360.0f, 360.0f))
		{
			context.textObject->setAngleX(rotate);
		}
	}

	if (context.orbitAngle && context.pointLights && !context.pointLights->empty())
	{
		if (ImGui::SliderAngle("angle", context.orbitAngle))
		{
			const float radius = 3.0f;
			const float x = radius * glm::sin(*context.orbitAngle);
			const float z = radius * glm::cos(*context.orbitAngle);
			(*context.pointLights)[0]->setPosition({ x, 3.0f, z });
		}
	}

	drawPostProcessControls(context.postProcessSettings, context.postProcessSettingsPath);
	drawPBRPreviewControls(context.pbrPreviewProfile, context.pbrPreviewProfilePath);
	drawPBRExperimentControls(context);
	drawEnvironmentControls(context.renderer, context.environmentProfile, context.environmentProfilePath);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

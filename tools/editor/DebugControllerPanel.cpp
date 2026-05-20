#include "DebugControllerPanel.h"

#include <string>

#include "../../application/RuntimeFramePipelineProfile.h"
#include "../../renderer/EnvironmentProfile.h"
#include "../../renderer/renderer.h"
#include "../../light/directionalLight.h"
#include "../../light/pointLight.h"
#include "../../light/shadow/shadow.h"
#include "../../mesh/mesh.h"
#include "../../renderer/PostProcessSettings.h"
#include "../../third_party/imgui/imgui.h"
#include "../inspector/PropertyInspector.h"
#include "../sceneSetup/PBRCameraRigProfile.h"
#include "../sceneSetup/PBRExperimentProfile.h"
#include "../sceneSetup/PBRLightRigProfile.h"
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
			profile->visitEditableProperties(builder);
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
			profile.visitEditableProperties(builder);
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

	void drawRendererFrameStats(const std::shared_ptr<GLframework::Renderer>& renderer)
	{
		if (!renderer)
		{
			return;
		}

		if (ImGui::CollapsingHeader("Renderer Frame Stats", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const GLframework::RendererFrameStats& stats = renderer->getLastFrameStats();
			const bool pbrPathActive = stats.pbrDepthPrepassDrawCalls > 0 || stats.pbrSceneDrawCalls > 0;

			ImGui::Text("PBR Path Active: %s", pbrPathActive ? "Yes" : "No");
			ImGui::Text("Renderer Passes Executed: %d", stats.rendererPassCount);
			ImGui::Text("Shadow Casters: %d", stats.shadowCasterCount);
			ImGui::Text("Directional Shadow Layers: %d", stats.directionalShadowLayerCount);
			ImGui::Text("Directional Shadow Draw Calls: %d", stats.directionalShadowDrawCalls);
			ImGui::Text("Point Shadow Lights: %d", stats.pointShadowLightCount);
			ImGui::Text("Point Shadow Faces: %d", stats.pointShadowFaceCount);
			ImGui::Text("Point Shadow Draw Calls: %d", stats.pointShadowDrawCalls);
			ImGui::Text("Legacy Scene Draw Calls: %d", stats.legacySceneDrawCalls);
			ImGui::Text("PBR Depth Prepass Draw Calls: %d", stats.pbrDepthPrepassDrawCalls);
			ImGui::Text("PBR GBuffer Ready: %s", stats.pbrGBufferReady ? "Yes" : "No");
			ImGui::Text("PBR GBuffer Size: %d x %d", stats.pbrGBufferWidth, stats.pbrGBufferHeight);
			ImGui::Text("PBR GBuffer Draw Calls: %d", stats.pbrGBufferDrawCalls);
			ImGui::Text("PBR GBuffer Debug Draw Calls: %d", stats.pbrGBufferDebugDrawCalls);
			ImGui::Text("PBR Scene Draw Calls: %d", stats.pbrSceneDrawCalls);
			ImGui::Text("IBL Debug Draw Calls: %d", stats.iblDebugDrawCalls);
			ImGui::TextWrapped("Use these values to verify whether the current scene is actually using the PBR render path.");
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

void GL_EDITOR::drawDebugControllerPanel(const DebugControllerContext& context)
{
	ImGui::Begin("controller");

	if (context.directionalLight && *context.directionalLight)
	{
		auto pos = (*context.directionalLight)->getPosition();
		if (ImGui::SliderFloat("light.x", &pos.x, 0.0f, 50.0f, "%.2f"))
		{
			(*context.directionalLight)->setPosition(pos);
		}

		if ((*context.directionalLight)->getShadow())
		{
			ImGui::SliderFloat("tightness", &(*context.directionalLight)->getShadow()->mDiskTightness, 0.0f, 1.0f, "%.3f");
			ImGui::SliderFloat("pcfRadius", &(*context.directionalLight)->getShadow()->mPcfRadius, 0.0f, 10.0f, "%.3f");
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
	drawFramePipelineControls(context.framePipelineProfile, context.framePipelineProfilePath);
	drawRendererFramePassControls(context.renderer, context.rendererFramePassProfilePath);
	drawRendererFrameStats(context.renderer);
	drawPBRPreviewControls(context.pbrPreviewProfile, context.pbrPreviewProfilePath);
	drawPBRExperimentControls(context);
	drawEnvironmentControls(context.renderer, context.environmentProfile, context.environmentProfilePath);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

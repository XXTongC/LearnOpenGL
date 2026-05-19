#include "DebugControllerPanel.h"

#include <array>
#include <string>

#include "../../renderer/EnvironmentProfile.h"
#include "../../renderer/renderer.h"
#include "../../light/directionalLight.h"
#include "../../light/pointLight.h"
#include "../../light/shadow/shadow.h"
#include "../../mesh/mesh.h"
#include "../../renderer/PostProcessSettings.h"
#include "../../third_party/imgui/imgui.h"

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

		static std::array<char, 512> hdrPathBuffer{};
		static std::string lastPath{};
		static std::string lastPrecomputeStatus{};
		static std::string lastConfigStatus{};
		const std::string configPath = profilePath ? *profilePath : GLframework::EnvironmentProfileStorage::defaultPath();
		if (lastPath != profile->hdrEquirectangularPath)
		{
			hdrPathBuffer.fill('\0');
			profile->hdrEquirectangularPath.copy(hdrPathBuffer.data(), hdrPathBuffer.size() - 1);
			lastPath = profile->hdrEquirectangularPath;
		}

		if (ImGui::CollapsingHeader("Environment / IBL", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::TextWrapped("Profile File: %s", configPath.c_str());

			if (ImGui::InputText("HDR Path", hdrPathBuffer.data(), hdrPathBuffer.size()))
			{
				profile->hdrEquirectangularPath = hdrPathBuffer.data();
				lastPath = profile->hdrEquirectangularPath;
			}

			int unit = static_cast<int>(profile->hdrTextureUnit);
			if (ImGui::SliderInt("HDR Texture Unit", &unit, 0, 31))
			{
				profile->hdrTextureUnit = static_cast<unsigned int>(unit);
			}

			ImGui::Checkbox("Precompute On Prepare", &profile->precomputeOnPrepare);
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
					lastPath.clear();
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

			const bool canPrecompute = renderer != nullptr && profile->hasHdrSource();
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
				ImGui::TextWrapped("Set an HDR equirectangular texture path before precomputing.");
			}

			if (!lastPrecomputeStatus.empty())
			{
				ImGui::TextWrapped("%s", lastPrecomputeStatus.c_str());
			}
		}
	}

	void drawPostProcessControls(GLframework::PostProcessSettings* settings)
	{
		if (!settings)
		{
			return;
		}

		if (ImGui::CollapsingHeader("Post Process", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Exposure", &settings->exposure, 0.0f, 4.0f);

			int toneMappingMode = static_cast<int>(settings->toneMappingMode);
			if (ImGui::SliderInt("Tone Mapping Mode", &toneMappingMode, 0, 1))
			{
				settings->toneMappingMode = toneMappingMode == 1
					? GLframework::ToneMappingMode::Reinhard
					: GLframework::ToneMappingMode::Exposure;
			}

			ImGui::Checkbox("Bloom Enabled", &settings->bloomEnabled);
			ImGui::SliderFloat("Bloom Threshold", &settings->bloomThreshold, 0.0f, 20.0f);
			ImGui::SliderFloat("Bloom Intensity", &settings->bloomIntensity, 0.0f, 2.0f);
			ImGui::SliderInt("Bloom Iterations", &settings->bloomIterations, 0, 20);
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

	drawPostProcessControls(context.postProcessSettings);
	drawEnvironmentControls(context.renderer, context.environmentProfile, context.environmentProfilePath);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

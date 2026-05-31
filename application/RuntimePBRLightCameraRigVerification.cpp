#include "RuntimePBRLightCameraRigVerification.h"

#include <algorithm>
#include <cstddef>

#include "AppRuntimeContext.h"
#include "RuntimeVerificationConfig.h"
#include "../tools/sceneSetup/PBRCameraRigProfile.h"
#include "../tools/sceneSetup/PBRLightRigProfile.h"

namespace
{
	void applyPressurePointLightRig(GLframework::AppRuntimeContext& context)
	{
		auto& lightRigProfile = context.profiles.pbrLightRigProfile();
		struct PressureLightPreset
		{
			glm::vec3 position{ 0.0f };
			glm::vec3 color{ 1.0f };
			float intensity{ 1.0f };
		};

		const PressureLightPreset presets[] = {
			{ { -2.35f, 1.05f, 2.15f }, { 1.0f, 0.32f, 0.18f }, 2.8f },
			{ { -1.45f, -0.75f, 2.05f }, { 1.0f, 0.72f, 0.25f }, 2.4f },
			{ { -0.35f, 1.20f, 2.35f }, { 0.45f, 0.95f, 1.0f }, 2.6f },
			{ { 0.75f, -0.95f, 2.10f }, { 0.28f, 0.55f, 1.0f }, 2.5f },
			{ { 1.85f, 0.85f, 2.20f }, { 1.0f, 0.25f, 0.65f }, 2.7f },
			{ { 2.55f, -0.35f, 1.55f }, { 0.45f, 1.0f, 0.38f }, 2.2f },
			{ { -2.15f, -1.25f, 1.35f }, { 0.75f, 0.45f, 1.0f }, 2.1f },
			{ { 0.15f, 0.05f, 0.95f }, { 1.0f, 1.0f, 0.45f }, 2.3f }
		};

		const int count = std::min(
			static_cast<int>(sizeof(presets) / sizeof(presets[0])),
			GL_SCENE::PBRLightRigProfile::maxPointLights
		);
		lightRigProfile.pointLightCount = count;
		for (int index = 0; index < count; ++index)
		{
			auto& pointLight = lightRigProfile.pointLights[static_cast<std::size_t>(index)];
			pointLight.position = presets[index].position;
			pointLight.color = presets[index].color;
			pointLight.intensity = presets[index].intensity;
			pointLight.specularIntensity = 1.0f;
			pointLight.attenuationK2 = 96.0f;
			pointLight.attenuationK1 = 0.0f;
			pointLight.attenuationK0 = 1.0f;
		}
	}
}

namespace GL_RUNTIME
{
	void RuntimePBRLightCameraRigVerification::applyLightCameraRig(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& verification
	)
	{
		const auto& probes = verification.pbr.probes;
		const auto& engineWorld = verification.engineWorld;
		auto& lightRigProfile = context.profiles.pbrLightRigProfile();

		lightRigProfile.ambientColor = { 0.1f, 0.1f, 0.1f };
		lightRigProfile.ambientIntensity = 1.0f;
		lightRigProfile.directional.color = { 0.0f, 0.0f, 0.0f };
		lightRigProfile.spot.color = { 0.0f, 0.0f, 0.0f };
		lightRigProfile.pointLightCount = 2;
		lightRigProfile.pointLights[0].position = { 3.0f, 3.0f, -1.0f };
		lightRigProfile.pointLights[0].color = { 0.8f, 0.8f, 0.9f };
		lightRigProfile.pointLights[0].intensity = 1.0f;
		lightRigProfile.pointLights[0].specularIntensity = 0.9f;
		lightRigProfile.pointLights[1].position = { -3.0f, 3.0f, -1.0f };
		lightRigProfile.pointLights[1].color = { 1.0f, 1.0f, 1.0f };
		lightRigProfile.pointLights[1].intensity = 1.0f;
		lightRigProfile.pointLights[1].specularIntensity = 0.9f;
		if (engineWorld.enableMinimalScene)
		{
			lightRigProfile.pointLightCount = 2;
			lightRigProfile.pointLights[0].position = { -1.55f, 1.25f, 2.75f };
			lightRigProfile.pointLights[0].color = { 1.0f, 0.74f, 0.38f };
			lightRigProfile.pointLights[0].intensity = 2.8f;
			lightRigProfile.pointLights[0].specularIntensity = 1.0f;
			lightRigProfile.pointLights[0].attenuationK2 = 24.0f;
			lightRigProfile.pointLights[0].attenuationK1 = 0.0f;
			lightRigProfile.pointLights[0].attenuationK0 = 1.0f;
			lightRigProfile.pointLights[1].position = { 1.45f, -0.65f, 2.25f };
			lightRigProfile.pointLights[1].color = { 0.32f, 0.72f, 1.0f };
			lightRigProfile.pointLights[1].intensity = 2.4f;
			lightRigProfile.pointLights[1].specularIntensity = 1.0f;
			lightRigProfile.pointLights[1].attenuationK2 = 24.0f;
			lightRigProfile.pointLights[1].attenuationK1 = 0.0f;
			lightRigProfile.pointLights[1].attenuationK0 = 1.0f;
		}
		if (probes.enablePbrTiledLightProbe)
		{
			lightRigProfile.pointLightCount = 2;
			lightRigProfile.pointLights[0].position = { -1.45f, 0.65f, 2.2f };
			lightRigProfile.pointLights[0].color = { 1.0f, 0.45f, 0.25f };
			lightRigProfile.pointLights[0].intensity = 2.5f;
			lightRigProfile.pointLights[0].specularIntensity = 1.0f;
			lightRigProfile.pointLights[0].attenuationK2 = 96.0f;
			lightRigProfile.pointLights[0].attenuationK1 = 0.0f;
			lightRigProfile.pointLights[0].attenuationK0 = 1.0f;
			lightRigProfile.pointLights[1].position = { 1.45f, -0.35f, 2.2f };
			lightRigProfile.pointLights[1].color = { 0.25f, 0.65f, 1.0f };
			lightRigProfile.pointLights[1].intensity = 2.5f;
			lightRigProfile.pointLights[1].specularIntensity = 1.0f;
			lightRigProfile.pointLights[1].attenuationK2 = 96.0f;
			lightRigProfile.pointLights[1].attenuationK1 = 0.0f;
			lightRigProfile.pointLights[1].attenuationK0 = 1.0f;
		}
		if (probes.enablePbrLightPressureProbe)
		{
			applyPressurePointLightRig(context);
		}

		auto& cameraRigProfile = context.profiles.pbrCameraRigProfile();
		cameraRigProfile.position = { 0.0f, 0.0f, 5.0f };
		cameraRigProfile.up = { 0.0f, 1.0f, 0.0f };
		cameraRigProfile.right = { 1.0f, 0.0f, 0.0f };
		cameraRigProfile.fovy = 60.0f;
		if (probes.enablePbrShowcaseSpheres)
		{
			cameraRigProfile.position = { 0.0f, 0.2f, 6.1f };
			cameraRigProfile.fovy = 52.0f;
		}
		if (engineWorld.enableMinimalScene)
		{
			cameraRigProfile.position = { 0.0f, 0.08f, 4.85f };
			cameraRigProfile.fovy = 48.0f;
		}
		cameraRigProfile.nearPlane = 0.1f;
		cameraRigProfile.farPlane = 1000.0f;
		cameraRigProfile.applyTo(context.cameraLights.camera);
	}
}

#include "PBRLightRigProfile.h"

#include <algorithm>
#include <string>

#include "../../light/ambientLight.h"
#include "../../light/directionalLight.h"
#include "../../light/light.h"
#include "../../light/pointLight.h"
#include "../../light/shadow/pointLightShadow/pointLightShadow.h"
#include "../../light/spotLight.h"

namespace
{
	void applyBasicLightProfile(const GL_SCENE::PBRBasicLightProfile& profile, GLframework::Light& light)
	{
		light.setPosition(profile.position);
		light.setAngleX(profile.rotation.x);
		light.setAngleY(profile.rotation.y);
		light.setAngleZ(profile.rotation.z);
		light.setColor(profile.color);
		light.setIntensity(profile.intensity);
		light.setSpecularIntensity(profile.specularIntensity);
	}

	void copyBasicLightProfile(const GLframework::Light& light, GL_SCENE::PBRBasicLightProfile& profile)
	{
		profile.position = light.getPosition();
		profile.rotation = { light.getAngleX(), light.getAngleY(), light.getAngleZ() };
		profile.color = light.getColor();
		profile.intensity = light.getIntensity();
		profile.specularIntensity = light.getSpecularIntensity();
	}

	int clampPointLightCount(int value)
	{
		return std::clamp(value, 0, GL_SCENE::PBRLightRigProfile::maxPointLights);
	}
}

GL_SCENE::PBRLightRigProfile::PBRLightRigProfile()
{
	directional.position = { 0.0f, 11.0f, 0.0f };
	directional.rotation = { -45.0f, 45.0f, 0.0f };
	directional.color = { 0.0f, 0.0f, 0.0f };
	directional.intensity = 1.0f;
	directional.specularIntensity = 0.5f;

	spot.position = { 1.5f, 0.0f, 0.0f };
	spot.rotation = { 0.0f, 0.0f, 0.0f };
	spot.color = { 0.0f, 0.0f, 0.0f };
	spot.intensity = 1.0f;
	spot.specularIntensity = 1.0f;
	spot.innerAngle = 30.0f;
	spot.outerAngle = 60.0f;

	pointLights[0].position = { 3.0f, 3.0f, -1.0f };
	pointLights[0].color = { 0.8f, 0.8f, 0.9f };
	pointLights[0].intensity = 1.0f;
	pointLights[0].specularIntensity = 0.9f;

	pointLights[1].position = { -3.0f, 3.0f, -1.0f };
	pointLights[1].color = { 1.0f, 1.0f, 1.0f };
	pointLights[1].intensity = 1.0f;
	pointLights[1].specularIntensity = 0.9f;
}

void GL_SCENE::PBRLightRigProfile::applyTo(
	std::shared_ptr<GLframework::AmbientLight>& ambientLight,
	std::shared_ptr<GLframework::DirectionalLight>& directionalLight,
	std::shared_ptr<GLframework::SpotLight>& spotLight,
	std::vector<std::shared_ptr<GLframework::PointLight>>& runtimePointLights
) const
{
	if (!ambientLight)
	{
		ambientLight = std::make_shared<GLframework::AmbientLight>();
	}
	ambientLight->setName("Ambient Light");
	ambientLight->setColor(ambientColor);
	ambientLight->setIntensity(ambientIntensity);

	if (!directionalLight)
	{
		directionalLight = std::make_shared<GLframework::DirectionalLight>();
	}
	directionalLight->setName("Directional Light");
	applyBasicLightProfile(directional, *directionalLight);

	if (!spotLight)
	{
		spotLight = std::make_shared<GLframework::SpotLight>();
	}
	spotLight->setName("Spot Light");
	applyBasicLightProfile(spot, *spotLight);
	spotLight->setInnerAngle(spot.innerAngle);
	spotLight->setOutAngle(spot.outerAngle);

	const int count = clampPointLightCount(pointLightCount);
	GLframework::PointLightShadow::setMAX_POINT_LIGHT(count);
	runtimePointLights.clear();
	for (int index = 0; index < count; ++index)
	{
		auto pointLight = std::make_shared<GLframework::PointLight>();
		pointLight->setName("Point Light " + std::to_string(index));
		applyBasicLightProfile(pointLights[static_cast<std::size_t>(index)], *pointLight);
		pointLight->setK(
			pointLights[static_cast<std::size_t>(index)].attenuationK2,
			pointLights[static_cast<std::size_t>(index)].attenuationK1,
			pointLights[static_cast<std::size_t>(index)].attenuationK0
		);
		runtimePointLights.push_back(std::move(pointLight));
	}
}

void GL_SCENE::PBRLightRigProfile::copyFrom(
	const std::shared_ptr<GLframework::AmbientLight>& ambientLight,
	const std::shared_ptr<GLframework::DirectionalLight>& directionalLight,
	const std::shared_ptr<GLframework::SpotLight>& spotLight,
	const std::vector<std::shared_ptr<GLframework::PointLight>>& runtimePointLights
)
{
	if (ambientLight)
	{
		ambientColor = ambientLight->getColor();
		ambientIntensity = ambientLight->getIntensity();
	}

	if (directionalLight)
	{
		copyBasicLightProfile(*directionalLight, directional);
	}

	if (spotLight)
	{
		copyBasicLightProfile(*spotLight, spot);
		spot.innerAngle = spotLight->getInnerAngle();
		spot.outerAngle = spotLight->getOutAngle();
	}

	pointLightCount = clampPointLightCount(static_cast<int>(runtimePointLights.size()));
	for (int index = 0; index < pointLightCount; ++index)
	{
		const auto& runtimeLight = runtimePointLights[static_cast<std::size_t>(index)];
		if (!runtimeLight)
		{
			continue;
		}

		auto& profile = pointLights[static_cast<std::size_t>(index)];
		copyBasicLightProfile(*runtimeLight, profile);
		profile.attenuationK2 = runtimeLight->getK2();
		profile.attenuationK1 = runtimeLight->getK1();
		profile.attenuationK0 = runtimeLight->getK0();
	}
}

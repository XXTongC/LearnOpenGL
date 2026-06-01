#include "PBRLightRigProfileConfig.h"

#include <array>
#include <string>

#include "PBRLightRigProfile.h"
#include "../inspector/PropertySchema.h"

namespace
{
	std::array<std::string, 3> xyzKeys(const std::string& prefix)
	{
		return { prefix + "X", prefix + "Y", prefix + "Z" };
	}

	std::array<std::string, 3> rgbKeys(const std::string& prefix)
	{
		return { prefix + "R", prefix + "G", prefix + "B" };
	}

	void addBasicLightProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const std::string& keyPrefix,
		const std::string& labelPrefix,
		GL_SCENE::PBRBasicLightProfile& profile
	)
	{
		builder.addConfigVec3(xyzKeys(keyPrefix + "Position"), labelPrefix + " Position", &profile.position);
		builder.addConfigVec3(xyzKeys(keyPrefix + "Rotation"), labelPrefix + " Rotation", &profile.rotation);
		builder.addConfigColor3(rgbKeys(keyPrefix + "Color"), labelPrefix + " Color", &profile.color);
		builder.addConfigFloat(keyPrefix + "Intensity", labelPrefix + " Intensity", &profile.intensity, 0.0f, 20.0f);
		builder.addConfigFloat(keyPrefix + "Specular", labelPrefix + " Specular", &profile.specularIntensity, 0.0f, 20.0f);
	}
}

void GL_SCENE::buildPBRLightRigProfileConfigSchema(
	GL_EDITOR::PropertyBuilder& builder,
	PBRLightRigProfile& profile
)
{
	builder.addSection("Ambient Light");
	builder.addConfigColor3(rgbKeys("ambientColor"), "Ambient Color", &profile.ambientColor);
	builder.addConfigFloat("ambientIntensity", "Ambient Intensity", &profile.ambientIntensity, 0.0f, 10.0f);

	builder.addSection("Directional Light");
	addBasicLightProperties(builder, "directional", "Directional", profile.directional);

	builder.addSection("Spot Light");
	addBasicLightProperties(builder, "spot", "Spot", profile.spot);
	builder.addConfigFloat("spotInnerAngle", "Spot Inner Angle", &profile.spot.innerAngle, 0.0f, 90.0f);
	builder.addConfigFloat("spotOuterAngle", "Spot Outer Angle", &profile.spot.outerAngle, 0.0f, 90.0f);

	builder.addSection("Point Lights");
	builder.addConfigInt("pointLightCount", "Point Light Count", &profile.pointLightCount, 0, PBRLightRigProfile::maxPointLights);
	for (int index = 0; index < PBRLightRigProfile::maxPointLights; ++index)
	{
		auto& pointProfile = profile.pointLights[static_cast<std::size_t>(index)];
		const std::string keyPrefix = "point" + std::to_string(index);
		const std::string labelPrefix = "Point " + std::to_string(index);
		addBasicLightProperties(builder, keyPrefix, labelPrefix, pointProfile);
		builder.addConfigFloat(keyPrefix + "AttenuationK2", labelPrefix + " Attenuation K2", &pointProfile.attenuationK2, 0.0f, 10.0f);
		builder.addConfigFloat(keyPrefix + "AttenuationK1", labelPrefix + " Attenuation K1", &pointProfile.attenuationK1, 0.0f, 10.0f);
		builder.addConfigFloat(keyPrefix + "AttenuationK0", labelPrefix + " Attenuation K0", &pointProfile.attenuationK0, 0.0f, 10.0f);
	}
}

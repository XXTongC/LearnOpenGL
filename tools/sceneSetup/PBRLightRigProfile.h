#pragma once

#include <array>
#include <memory>
#include <vector>

#include "../../light/ambientLight.h"
#include "../../light/directionalLight.h"
#include "../../light/pointLight.h"
#include "../../light/spotLight.h"

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GL_SCENE
{
	struct PBRBasicLightProfile
	{
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 color{ 1.0f, 1.0f, 1.0f };
		float intensity{ 1.0f };
		float specularIntensity{ 1.0f };
	};

	struct PBRPointLightProfile : PBRBasicLightProfile
	{
		float attenuationK2{ 0.0f };
		float attenuationK1{ 0.0f };
		float attenuationK0{ 1.0f };
	};

	struct PBRSpotLightProfile : PBRBasicLightProfile
	{
		float innerAngle{ 30.0f };
		float outerAngle{ 60.0f };
	};

	struct PBRLightRigProfile
	{
		static constexpr int maxPointLights = 2;

		PBRLightRigProfile();

		glm::vec3 ambientColor{ 0.1f, 0.1f, 0.1f };
		float ambientIntensity{ 1.0f };
		PBRBasicLightProfile directional{};
		PBRSpotLightProfile spot{};
		int pointLightCount{ 2 };
		std::array<PBRPointLightProfile, maxPointLights> pointLights{};

		void applyTo(
			std::shared_ptr<GLframework::AmbientLight>& ambientLight,
			std::shared_ptr<GLframework::DirectionalLight>& directionalLight,
			std::shared_ptr<GLframework::SpotLight>& spotLight,
			std::vector<std::shared_ptr<GLframework::PointLight>>& runtimePointLights
		) const;
		void copyFrom(
			const std::shared_ptr<GLframework::AmbientLight>& ambientLight,
			const std::shared_ptr<GLframework::DirectionalLight>& directionalLight,
			const std::shared_ptr<GLframework::SpotLight>& spotLight,
			const std::vector<std::shared_ptr<GLframework::PointLight>>& runtimePointLights
		);
		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder);
	};
}

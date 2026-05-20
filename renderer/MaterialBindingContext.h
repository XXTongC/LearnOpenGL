#pragma once

#include <memory>
#include <vector>

class Camera;

namespace GLframework
{
	class AmbientLight;
	class DirectionalLight;
	class EnvironmentRenderTargets;
	class PointLight;
	class SpotLight;

	struct MaterialBindingContext
	{
		Camera* camera{ nullptr };
		std::shared_ptr<DirectionalLight> dirLight{ nullptr };
		std::shared_ptr<SpotLight> spotLight{ nullptr };
		const std::vector<std::shared_ptr<PointLight>>* pointLights{ nullptr };
		std::shared_ptr<AmbientLight> ambient{ nullptr };
		const EnvironmentRenderTargets* environmentTargets{ nullptr };

		const std::vector<std::shared_ptr<PointLight>>& getPointLights() const
		{
			static const std::vector<std::shared_ptr<PointLight>> emptyPointLights{};
			return pointLights ? *pointLights : emptyPointLights;
		}
	};
}

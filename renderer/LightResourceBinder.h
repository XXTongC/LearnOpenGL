#pragma once

#include <memory>
#include <vector>

namespace GLframework
{
	class AmbientLight;
	class DirectionalLight;
	class PointLight;
	class Shader;
	class SpotLight;

	class LightResourceBinder
	{
	public:
		static void bindForwardLights(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::shared_ptr<SpotLight>& spotLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			const std::shared_ptr<AmbientLight>& ambient
		);
	};
}

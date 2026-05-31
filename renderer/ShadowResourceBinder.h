#pragma once

#include <memory>
#include <vector>

class Camera;

namespace GLframework
{
	class DirectionalLight;
	class PointLight;
	class Shader;

	class ShadowResourceBinder
	{
	public:
		static void bindCSMShadowResources(
			const std::shared_ptr<Shader>& shader,
			Camera* camera,
			const std::shared_ptr<DirectionalLight>& dirLight,
			int textureUnit = 2
		);

		static void bindPointShadowResources(
			const std::shared_ptr<Shader>& shader,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			int textureUnit = 2
		);

		static void bindDirectionalFallbackShadow(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<DirectionalLight>& dirLight
		);
	};
}

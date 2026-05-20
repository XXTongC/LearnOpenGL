#pragma once

#include <memory>
#include <vector>

#include "framework/shader.h"
#include "light/ambientLight.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "light/spotLight.h"

namespace GLframework
{
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

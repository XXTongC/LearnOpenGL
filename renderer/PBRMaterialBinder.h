#pragma once

#include <memory>
#include <vector>

#include "camera/camera.h"
#include "framework/shader.h"
#include "light/ambientLight.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "light/spotLight.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "mesh/mesh.h"

namespace GLframework
{
	class EnvironmentRenderTargets;

	class PBRMaterialBinder
	{
	public:
		static bool bind(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<PBRMaterial>& material,
			const std::shared_ptr<Mesh>& mesh,
			Camera* camera,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::shared_ptr<SpotLight>& spotLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			const std::shared_ptr<AmbientLight>& ambient,
			const EnvironmentRenderTargets* environmentTargets
		);
	};
}

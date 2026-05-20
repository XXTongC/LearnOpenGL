#pragma once

#include <memory>
#include <vector>

#include "camera/camera.h"
#include "light/ambientLight.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "light/spotLight.h"
#include "mesh/mesh.h"
#include "renderer/ShaderLibrary.h"

namespace GLframework
{
	class EnvironmentRenderTargets;

	class PBRSceneRenderPass
	{
	public:
		int render(
			const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
			const std::vector<std::shared_ptr<Mesh>>& pbrTransparentObjects,
			Camera* camera,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::shared_ptr<SpotLight>& spotLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			const std::shared_ptr<AmbientLight>& ambient,
			const ShaderLibrary& shaderLibrary,
			const EnvironmentRenderTargets* environmentTargets
		) const;

	private:
		bool renderObject(
			const std::shared_ptr<Mesh>& mesh,
			Camera* camera,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::shared_ptr<SpotLight>& spotLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			const std::shared_ptr<AmbientLight>& ambient,
			const ShaderLibrary& shaderLibrary,
			const EnvironmentRenderTargets* environmentTargets
		) const;

	};
}

#pragma once

#include <memory>
#include <vector>

#include "camera/camera.h"
#include "light/ambientLight.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "light/spotLight.h"
#include "materials/material.h"
#include "mesh/mesh.h"
#include "renderer/ShaderLibrary.h"

namespace GLframework
{
	class SceneRenderPass
	{
	public:
		void render(
			const std::vector<std::shared_ptr<Mesh>>& opacityObjects,
			const std::vector<std::shared_ptr<Mesh>>& transparentObjects,
			Camera* camera,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::shared_ptr<SpotLight>& spotLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			const std::shared_ptr<AmbientLight>& ambient,
			const std::shared_ptr<Material>& globalMaterial,
			const ShaderLibrary& shaderLibrary
		) const;

	private:
		void renderObject(
			const std::shared_ptr<Mesh>& mesh,
			Camera* camera,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::shared_ptr<SpotLight>& spotLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			const std::shared_ptr<AmbientLight>& ambient,
			const std::shared_ptr<Material>& globalMaterial,
			const ShaderLibrary& shaderLibrary
		) const;

		void drawMesh(const std::shared_ptr<Mesh>& mesh) const;
	};
}

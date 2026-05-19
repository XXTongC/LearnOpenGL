#pragma once

#include <memory>
#include <vector>

#include "camera/camera.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "mesh/mesh.h"
#include "renderer/ShaderLibrary.h"

namespace GLframework
{
	class ShadowRenderer
	{
	public:
		void render(
			Camera* camera,
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			ShaderLibrary& shaderLibrary
		);

	private:
		void renderDirectionalShadowMap(
			Camera* camera,
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const std::shared_ptr<DirectionalLight>& dirLight,
			ShaderLibrary& shaderLibrary
		);

		void renderPointShadowMap(
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			ShaderLibrary& shaderLibrary
		);

		bool isPostProcessPass(const std::vector<std::shared_ptr<Mesh>>& meshes) const;
		void drawShadowMesh(const std::shared_ptr<Mesh>& mesh) const;
	};
}

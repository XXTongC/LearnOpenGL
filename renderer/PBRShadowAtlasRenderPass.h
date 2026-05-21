#pragma once

#include <memory>
#include <vector>

#include "camera/camera.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "mesh/mesh.h"
#include "renderer/PBRShadowAtlasRenderTargets.h"
#include "renderer/ShaderLibrary.h"

namespace GLframework
{
	class PBRShadowAtlasRenderPass
	{
	public:
		PBRShadowAtlasStats render(
			Camera* camera,
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			PBRShadowAtlasRenderTargets& targets,
			ShaderLibrary& shaderLibrary
		) const;

	private:
		int renderDirectionalAtlas(
			Camera* camera,
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const PBRShadowAtlasRenderTargets& targets,
			PBRShadowAtlasStats& stats,
			ShaderLibrary& shaderLibrary
		) const;
		int renderPointAtlas(
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			const PBRShadowAtlasRenderTargets& targets,
			PBRShadowAtlasStats& stats,
			ShaderLibrary& shaderLibrary,
			int& renderedFaceCount
		) const;
	};
}

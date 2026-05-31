#pragma once

#include <memory>
#include <vector>

#include "DirectionalShadowRenderPass.h"
#include "PointShadowRenderPass.h"
#include "renderer/ShadowRenderStats.h"

class Camera;

namespace GLframework
{
	class DirectionalLight;
	class Mesh;
	class PointLight;
	class ShaderLibrary;

	class ShadowRenderer
	{
	public:
		ShadowRenderStats render(
			Camera* camera,
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			ShaderLibrary& shaderLibrary
		);

	private:
		DirectionalShadowRenderPass mDirectionalPass{};
		PointShadowRenderPass mPointPass{};
	};
}

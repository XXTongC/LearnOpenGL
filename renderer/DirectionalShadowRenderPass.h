#pragma once

#include <memory>
#include <vector>

#include "renderer/ShadowRenderStats.h"

class Camera;

namespace GLframework
{
	class DirectionalLight;
	class Mesh;
	class ShaderLibrary;

	class DirectionalShadowRenderPass
	{
	public:
		ShadowRenderStats render(
			Camera* camera,
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const std::shared_ptr<DirectionalLight>& dirLight,
			ShaderLibrary& shaderLibrary
		) const;
	};
}

#pragma once

#include <memory>
#include <vector>

#include "renderer/ShadowRenderStats.h"

namespace GLframework
{
	class Mesh;
	class PointLight;
	class ShaderLibrary;

	class PointShadowRenderPass
	{
	public:
		ShadowRenderStats render(
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const std::vector<std::shared_ptr<PointLight>>& pointLights,
			ShaderLibrary& shaderLibrary
		) const;
	};
}

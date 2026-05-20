#pragma once

#include <memory>
#include <vector>

#include "light/pointLight.h"
#include "mesh/mesh.h"
#include "renderer/ShaderLibrary.h"
#include "renderer/ShadowRenderStats.h"

namespace GLframework
{
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

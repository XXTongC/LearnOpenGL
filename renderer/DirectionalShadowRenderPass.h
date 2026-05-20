#pragma once

#include <memory>
#include <vector>

#include "camera/camera.h"
#include "light/directionalLight.h"
#include "mesh/mesh.h"
#include "renderer/ShaderLibrary.h"
#include "renderer/ShadowRenderStats.h"

namespace GLframework
{
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

#pragma once

#include <memory>
#include <vector>

#include "mesh/mesh.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/ShaderLibrary.h"

namespace GLframework
{
	class PBRSceneRenderPass
	{
	public:
		int render(
			const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
			const std::vector<std::shared_ptr<Mesh>>& pbrTransparentObjects,
			const ShaderLibrary& shaderLibrary,
			const MaterialBindingContext& bindingContext
		) const;

	private:
		bool renderObject(
			const std::shared_ptr<Mesh>& mesh,
			const ShaderLibrary& shaderLibrary,
			const MaterialBindingContext& bindingContext
		) const;

	};
}

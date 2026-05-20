#pragma once

#include <memory>
#include <vector>

#include "materials/material.h"
#include "mesh/mesh.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/ShaderLibrary.h"

namespace GLframework
{
	class SceneRenderPass
	{
	public:
		int render(
			const std::vector<std::shared_ptr<Mesh>>& opacityObjects,
			const std::vector<std::shared_ptr<Mesh>>& transparentObjects,
			const std::shared_ptr<Material>& globalMaterial,
			const ShaderLibrary& shaderLibrary,
			const MaterialBindingContext& bindingContext
		) const;

	private:
		bool renderObject(
			const std::shared_ptr<Mesh>& mesh,
			const std::shared_ptr<Material>& globalMaterial,
			const ShaderLibrary& shaderLibrary,
			const MaterialBindingContext& bindingContext
		) const;

	};
}

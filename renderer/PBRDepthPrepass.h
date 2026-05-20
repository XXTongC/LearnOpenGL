#pragma once

#include <memory>
#include <vector>

#include "mesh/mesh.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/ShaderLibrary.h"

namespace GLframework
{
	class PBRDepthPrepass
	{
	public:
		int render(
			const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
			const MaterialBindingContext& bindingContext,
			const ShaderLibrary& shaderLibrary
		) const;

	private:
		bool renderObject(
			const std::shared_ptr<Mesh>& mesh,
			const std::shared_ptr<Shader>& shader
		) const;

	};
}

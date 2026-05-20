#pragma once

#include <memory>

#include "framework/shader.h"
#include "materials/material.h"
#include "mesh/mesh.h"
#include "renderer/MaterialBindingContext.h"

namespace GLframework
{
	class MaterialBinder
	{
	public:
		static bool bind(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<Material>& material,
			const std::shared_ptr<Mesh>& mesh,
			const MaterialBindingContext& context
		);
	};
}

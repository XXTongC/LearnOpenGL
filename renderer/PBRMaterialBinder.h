#pragma once

#include <memory>

#include "framework/shader.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "mesh/mesh.h"
#include "renderer/MaterialBindingContext.h"

namespace GLframework
{
	class PBRMaterialBinder
	{
	public:
		static bool bind(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<PBRMaterial>& material,
			const std::shared_ptr<Mesh>& mesh,
			const MaterialBindingContext& context
		);
	};
}

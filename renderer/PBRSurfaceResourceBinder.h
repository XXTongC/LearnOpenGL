#pragma once

#include <memory>

#include "framework/shader.h"
#include "materials/pbrMaterial/PBRMaterial.h"

namespace GLframework
{
	class PBRSurfaceResourceBinder
	{
	public:
		static bool bind(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<PBRMaterial>& material
		);
	};
}

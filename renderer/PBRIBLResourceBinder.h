#pragma once

#include <memory>

#include "framework/shader.h"
#include "materials/pbrMaterial/PBRMaterial.h"

namespace GLframework
{
	class EnvironmentRenderTargets;

	class PBRIBLResourceBinder
	{
	public:
		static bool canUseIBL(
			const std::shared_ptr<PBRMaterial>& material,
			const EnvironmentRenderTargets* environmentTargets
		);

		static bool bind(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<PBRMaterial>& material,
			const EnvironmentRenderTargets* environmentTargets
		);
	};
}

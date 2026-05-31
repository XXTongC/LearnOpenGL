#pragma once

#include <memory>

namespace GLframework
{
	class EnvironmentRenderTargets;
	class PBRMaterial;
	class Shader;

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

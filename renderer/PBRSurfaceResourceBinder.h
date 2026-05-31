#pragma once

#include <memory>

namespace GLframework
{
	class PBRMaterial;
	class Shader;

	class PBRSurfaceResourceBinder
	{
	public:
		static bool bind(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<PBRMaterial>& material
		);
	};
}

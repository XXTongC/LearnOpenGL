#pragma once

#include <memory>

namespace GLframework
{
	struct MaterialBindingContext;
	class Mesh;
	class PBRMaterial;
	class Shader;

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

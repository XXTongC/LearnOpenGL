#pragma once

#include <memory>

namespace GLframework
{
	struct MaterialBindingContext;
	class Material;
	class Mesh;
	class Shader;

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

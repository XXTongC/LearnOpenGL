#pragma once

#include <memory>
#include <vector>

namespace GLframework
{
	class Material;
	struct MaterialBindingContext;
	class Mesh;
	class ShaderLibrary;

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

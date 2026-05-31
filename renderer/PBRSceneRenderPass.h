#pragma once

#include <memory>
#include <vector>

namespace GLframework
{
	struct MaterialBindingContext;
	class Mesh;
	class ShaderLibrary;

	class PBRSceneRenderPass
	{
	public:
		int render(
			const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
			const std::vector<std::shared_ptr<Mesh>>& pbrTransparentObjects,
			const ShaderLibrary& shaderLibrary,
			const MaterialBindingContext& bindingContext
		) const;

	private:
		bool renderObject(
			const std::shared_ptr<Mesh>& mesh,
			const ShaderLibrary& shaderLibrary,
			const MaterialBindingContext& bindingContext
		) const;

	};
}

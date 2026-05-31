#pragma once

#include <memory>
#include <vector>

namespace GLframework
{
	struct MaterialBindingContext;
	class Mesh;
	class Shader;
	class ShaderLibrary;

	class PBRDepthPrepass
	{
	public:
		int render(
			const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
			const MaterialBindingContext& bindingContext,
			const ShaderLibrary& shaderLibrary
		) const;

	private:
		bool renderObject(
			const std::shared_ptr<Mesh>& mesh,
			const std::shared_ptr<Shader>& shader
		) const;

	};
}

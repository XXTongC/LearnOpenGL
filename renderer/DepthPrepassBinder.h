#pragma once

#include <memory>

namespace GLframework
{
	struct MaterialBindingContext;
	class Mesh;
	class Shader;

	class DepthPrepassBinder
	{
	public:
		static bool bindFrame(const std::shared_ptr<Shader>& shader, const MaterialBindingContext& context);
		static bool bindObject(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh);
	};
}

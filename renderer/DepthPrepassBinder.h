#pragma once

#include <memory>

#include "framework/shader.h"
#include "mesh/mesh.h"
#include "renderer/MaterialBindingContext.h"

namespace GLframework
{
	class DepthPrepassBinder
	{
	public:
		static bool bindFrame(const std::shared_ptr<Shader>& shader, const MaterialBindingContext& context);
		static bool bindObject(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh);
	};
}

#pragma once

#include <memory>

#include "framework/shader.h"
#include "renderer/MaterialBindingContext.h"

namespace GLframework
{
	class PBRShadowResourceBinder
	{
	public:
		static bool bind(
			const std::shared_ptr<Shader>& shader,
			const MaterialBindingContext& context
		);
		static int getCsmLayerCount(const MaterialBindingContext& context);
	};
}

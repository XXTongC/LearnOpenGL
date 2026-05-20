#pragma once

#include <memory>

#include "framework/shader.h"
#include "renderer/MaterialBindingContext.h"

namespace GLframework
{
	enum class PBRShadowResourceSource
	{
		None,
		LegacyCSM,
		PBRShadowAtlas
	};

	struct PBRShadowResourceBindResult
	{
		bool bound{ false };
		int csmLayerCount{ 0 };
		PBRShadowResourceSource source{ PBRShadowResourceSource::None };
		bool pointShadowAtlasBound{ false };
		int pointShadowAtlasLightCount{ 0 };
	};

	class PBRShadowResourceBinder
	{
	public:
		static bool bind(
			const std::shared_ptr<Shader>& shader,
			const MaterialBindingContext& context
		);
		static PBRShadowResourceBindResult bindDetailed(
			const std::shared_ptr<Shader>& shader,
			const MaterialBindingContext& context
		);
		static int getCsmLayerCount(const MaterialBindingContext& context);
	};
}

#pragma once

#include <vector>

namespace GLframework
{
	struct RendererFrameContext;

	enum class RendererFramePassKey
	{
		BeginFrame,
		ShadowMaps,
		GlobalMaterialScene,
		PBRDepthPrepass,
		LegacyOpaqueScene,
		PBROpaqueScene,
		LegacyTransparentScene,
		PBRTransparentScene
	};

	struct RendererFramePassDefinition
	{
		RendererFramePassKey key;
		const char* debugName;
	};

	class RendererFramePassRegistry
	{
	public:
		static const std::vector<RendererFramePassDefinition>& defaultPasses();
		static const std::vector<RendererFramePassDefinition>& globalMaterialOverridePasses();
		static void executePass(const RendererFramePassDefinition& pass, RendererFrameContext& context);
	};
}

#pragma once

#include <string>
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
		PBRGBuffer,
		PBRGBufferDebug,
		LegacyOpaqueScene,
		PBROpaqueScene,
		LegacyTransparentScene,
		PBRTransparentScene,
		IBLDebug
	};

	struct RendererFramePassDefinition
	{
		RendererFramePassKey key;
		const char* keyName;
		const char* debugName;
	};

	class RendererFramePassRegistry
	{
	public:
		static const std::vector<RendererFramePassDefinition>& defaultPasses();
		static const std::vector<RendererFramePassDefinition>& globalMaterialOverridePasses();
		static const char* defaultPassOrder();
		static const char* globalMaterialOverridePassOrder();
		static const RendererFramePassDefinition* findPassByKey(const std::string& key);
		static std::vector<const RendererFramePassDefinition*> buildPassPlan(const std::string& passOrder);
		static void executePass(const RendererFramePassDefinition& pass, RendererFrameContext& context);
	};
}

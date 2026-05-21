#pragma once

#include <string>

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GLframework
{
	struct RendererFramePassProfile
	{
		RendererFramePassProfile();

		std::string defaultPassOrder{};
		std::string globalMaterialOverridePassOrder{};
		int pbrGBufferDebugMode{ 0 };
		float pbrGBufferDebugIntensity{ 1.0f };
		float pbrDeferredLightingIntensity{ 1.0f };
		float pbrDeferredIblDiffuseStrength{ 1.0f };
		float pbrDeferredIblSpecularStrength{ 1.0f };
		bool pbrDeferredTiledLightsEnabled{ true };
		int pbrDeferredTileSize{ 16 };
		float pbrDeferredTiledLightCutoff{ 0.01f };
		bool pbrDeferredClusteredLightsEnabled{ false };
		bool pbrDeferredClusteredLayoutStatsEnabled{ false };
		int pbrDeferredClusteredDepthSlices{ 24 };
		int pbrDeferredClusteredMaxLightsPerCluster{ 64 };
		bool pbrDeferredClusteredStatsReadbackEnabled{ false };
		int pbrDeferredTiledLightDebugMaxLights{ 2 };
		float pbrDeferredTiledLightDebugIntensity{ 1.0f };
		int iblDebugMode{ 0 };
		float iblDebugMipLevel{ 0.0f };
		float iblDebugIntensity{ 1.0f };

		void resetToDefaults();
		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder);
	};

	class RendererFramePassProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, RendererFramePassProfile& profile);
		static bool saveToFile(const std::string& path, const RendererFramePassProfile& profile);
	};
}

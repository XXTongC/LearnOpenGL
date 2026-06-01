#include "RendererFramePassProfile.h"

#include "RendererFramePassProfileConfig.h"
#include "RendererFramePassRegistry.h"
#include "tools/config/ProfileConfigIO.h"

using namespace GLframework;

RendererFramePassProfile::RendererFramePassProfile()
{
	resetToDefaults();
}

void RendererFramePassProfile::resetToDefaults()
{
	defaultPassOrder = RendererFramePassRegistry::defaultPassOrder();
	globalMaterialOverridePassOrder = RendererFramePassRegistry::globalMaterialOverridePassOrder();
	rendererGpuTimingEnabled = false;
	pbrGBufferDebugMode = 0;
	pbrGBufferDebugIntensity = 1.0f;
	pbrDeferredLightingIntensity = 1.0f;
	pbrDeferredIblDiffuseStrength = 1.0f;
	pbrDeferredIblSpecularStrength = 1.0f;
	pbrDeferredTiledLightsEnabled = true;
	pbrDeferredTileSize = 16;
	pbrDeferredTiledLightCutoff = 0.01f;
	pbrDeferredClusteredLightsEnabled = false;
	pbrDeferredClusteredLayoutStatsEnabled = false;
	pbrDeferredClusteredDepthSlices = 24;
	pbrDeferredClusteredMaxLightsPerCluster = 64;
	pbrDeferredClusteredStatsReadbackEnabled = false;
	pbrDeferredTiledLightDebugMaxLights = 2;
	pbrDeferredTiledLightDebugIntensity = 1.0f;
	pbrDeferredClusteredLightDebugDepthSlice = -1;
	pbrDeferredClusteredLightDebugMaxLights = 2;
	pbrDeferredClusteredLightDebugIntensity = 1.0f;
	iblDebugMode = 0;
	iblDebugMipLevel = 0.0f;
	iblDebugIntensity = 1.0f;
}

std::string RendererFramePassProfileStorage::defaultPath()
{
	return "config/renderer_frame_pass.local.ini";
}

bool RendererFramePassProfileStorage::loadFromFile(const std::string& path, RendererFramePassProfile& profile)
{
	RendererFramePassProfile loadedProfile = profile;
	GL_EDITOR::PropertyBuilder builder{};
	buildRendererFramePassProfileConfigSchema(builder, loadedProfile);
	const bool loaded = GL_CONFIG::loadPropertyConfig(path, builder);
	if (!loaded)
	{
		return false;
	}

	profile = loadedProfile;
	return true;
}

bool RendererFramePassProfileStorage::saveToFile(const std::string& path, const RendererFramePassProfile& profile)
{
	RendererFramePassProfile snapshot = profile;
	GL_EDITOR::PropertyBuilder builder{};
	buildRendererFramePassProfileConfigSchema(builder, snapshot);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local renderer frame pass plan for PBR render-path experiments",
		builder
	);
}

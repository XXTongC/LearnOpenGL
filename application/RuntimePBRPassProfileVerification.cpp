#include "RuntimePBRPassProfileVerification.h"

#include <string>

#include "AppRuntimeContext.h"
#include "RuntimeVerificationConfig.h"
#include "../renderer/RendererFramePassProfile.h"
#include "../renderer/renderer.h"

namespace GL_RUNTIME
{
	void RuntimePBRPassProfileVerification::applyRendererPassProfile(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& verification
	)
	{
		if (!context.renderResources.renderer)
		{
			return;
		}

		const auto& config = verification.pbr;
		const auto& passes = config.passes;
		const auto& probes = config.probes;
		const auto& deferred = config.deferred;
		const auto& rendererVerification = verification.renderer;
		auto& rendererPassProfile = context.renderResources.renderer->getFramePassProfile();
		rendererPassProfile.resetToDefaults();
		const std::string shadowPrefix = passes.disablePbrShadowAtlasPass
			? "BeginFrame,ShadowMaps,"
			: "BeginFrame,ShadowMaps,PBRShadowAtlas,";
		if (passes.disablePbrShadowAtlasPass)
		{
			rendererPassProfile.defaultPassOrder =
				"BeginFrame,ShadowMaps,PBRDepthPrepass,LegacyOpaqueScene,PBROpaqueScene,LegacyTransparentScene,PBRTransparentScene";
		}
		if (passes.enablePbrDeferredLightingPass)
		{
			rendererPassProfile.defaultPassOrder =
				shadowPrefix + "PBRDepthPrepass,PBRGBuffer,PBRDeferredLighting";
			if (probes.enablePbrTransparentFallbackPass)
			{
				rendererPassProfile.defaultPassOrder += ",LegacyTransparentScene,PBRTransparentScene";
			}
			rendererPassProfile.pbrDeferredLightingIntensity = 1.0f;
			rendererPassProfile.pbrDeferredIblDiffuseStrength = 1.0f;
			rendererPassProfile.pbrDeferredIblSpecularStrength = 1.0f;
		}
		else if (passes.enablePbrDeferredTiledLightDebugPass)
		{
			rendererPassProfile.defaultPassOrder =
				shadowPrefix + "PBRDepthPrepass,PBRGBuffer,PBRDeferredTiledLightDebug";
			rendererPassProfile.pbrDeferredTiledLightDebugMaxLights = 2;
			rendererPassProfile.pbrDeferredTiledLightDebugIntensity = 1.0f;
		}
		else if (passes.enablePbrDeferredClusteredLightDebugPass)
		{
			rendererPassProfile.defaultPassOrder =
				shadowPrefix + "PBRDepthPrepass,PBRGBuffer,PBRDeferredClusteredLightDebug";
			rendererPassProfile.pbrDeferredClusteredLightDebugDepthSlice = -1;
			rendererPassProfile.pbrDeferredClusteredLightDebugMaxLights = 2;
			rendererPassProfile.pbrDeferredClusteredLightDebugIntensity = 1.0f;
		}
		else if (passes.enablePbrGBufferPass || passes.enablePbrGBufferDebugPass)
		{
			rendererPassProfile.defaultPassOrder =
				shadowPrefix + "PBRDepthPrepass,PBRGBuffer,LegacyOpaqueScene,PBROpaqueScene,LegacyTransparentScene,PBRTransparentScene";
		}

		if (deferred.pbrDeferredTileSizeOverride > 0)
		{
			rendererPassProfile.pbrDeferredTileSize = deferred.pbrDeferredTileSizeOverride;
		}
		if (deferred.pbrDeferredTiledLightCutoffOverride > 0.0f)
		{
			rendererPassProfile.pbrDeferredTiledLightCutoff = deferred.pbrDeferredTiledLightCutoffOverride;
		}
		if (deferred.disablePbrDeferredTiledLights)
		{
			rendererPassProfile.pbrDeferredTiledLightsEnabled = false;
		}
		if (probes.enablePbrClusteredLayoutProbe)
		{
			rendererPassProfile.pbrDeferredClusteredLayoutStatsEnabled = true;
		}
		if (probes.enablePbrClusteredGridProbe)
		{
			rendererPassProfile.pbrDeferredClusteredLightsEnabled = true;
		}
		if (probes.enablePbrClusteredStatsReadback)
		{
			rendererPassProfile.pbrDeferredClusteredStatsReadbackEnabled = true;
		}
		if (rendererVerification.enableGpuTimingProbe)
		{
			rendererPassProfile.rendererGpuTimingEnabled = true;
		}

		if (passes.enablePbrGBufferDebugPass)
		{
			rendererPassProfile.defaultPassOrder += ",PBRGBufferDebug";
			rendererPassProfile.pbrGBufferDebugMode = 0;
			rendererPassProfile.pbrGBufferDebugIntensity = 1.0f;
		}

		if (passes.enableIblDebugPass)
		{
			rendererPassProfile.defaultPassOrder += ",IBLDebug";
			rendererPassProfile.iblDebugMode = 0;
			rendererPassProfile.iblDebugMipLevel = 0.0f;
			rendererPassProfile.iblDebugIntensity = 1.0f;
		}
	}
}

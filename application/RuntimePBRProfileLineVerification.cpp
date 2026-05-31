#include "RuntimePBRProfileLineVerification.h"

#include <iostream>
#include <string>

#include "RuntimeVerificationConfig.h"
#include "../tools/Logger/LogManager.h"

namespace
{
	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}
}

namespace GL_RUNTIME
{
	void RuntimePBRProfileLineVerification::reportAppliedProfile(
		const RuntimeVerificationConfig& verification
	)
	{
		const auto& config = verification.pbr;
		const auto& passes = config.passes;
		const auto& probes = config.probes;
		const auto& deferred = config.deferred;
		const auto& engineWorld = verification.engineWorld;
		const auto& rendererVerification = verification.renderer;

		std::string profileLine = "PBR verification profile applied: procedural IBL";
		if (!engineWorld.enableMinimalScene)
		{
			profileLine += " + 5x5 material grid";
		}
		if (passes.enablePbrGBufferPass)
		{
			profileLine += " + PBR G-buffer pass";
		}
		if (passes.enablePbrDeferredLightingPass)
		{
			profileLine += " + PBR deferred lighting pass";
		}
		if (passes.enablePbrDeferredTiledLightDebugPass)
		{
			profileLine += " + PBR tiled light heatmap pass";
		}
		if (passes.enablePbrDeferredClusteredLightDebugPass)
		{
			profileLine += " + PBR clustered light heatmap pass";
		}
		if (probes.enablePbrTransparentFallbackPass)
		{
			profileLine += " + transparent forward fallback";
		}
		if (probes.enablePbrEmissiveProbe)
		{
			profileLine += " + emissive G-buffer probe";
		}
		if (probes.enablePbrMaterialIblProbe)
		{
			profileLine += " + material IBL params probe";
		}
		if (probes.enablePbrAlphaMaskProbe)
		{
			profileLine += " + alpha mask probe";
		}
		if (probes.enablePbrImportedAssetProbe)
		{
			profileLine += " + imported PBR asset probe";
		}
		if (probes.enablePbrTextureSetProbe)
		{
			profileLine += " + textured PBR material probe";
		}
		if (probes.enablePbrShowcaseSpheres)
		{
			profileLine += " + PBR showcase sphere scene";
		}
		if (engineWorld.enableSceneProbe)
		{
			profileLine += " + engine world scene probe";
		}
		if (engineWorld.enableMinimalScene)
		{
			profileLine += " + engine world minimal scene";
		}
		if (engineWorld.enableEditorCreate)
		{
			profileLine += " + editor-created actor probe";
		}
		if (engineWorld.enableScenePackageRoundTrip)
		{
			profileLine += " + scene package roundtrip";
		}
		if (probes.enablePbrTiledLightProbe)
		{
			profileLine += " + sparse tiled light probe";
		}
		if (probes.enablePbrClusteredLayoutProbe)
		{
			profileLine += " + clustered layout stats probe";
		}
		if (probes.enablePbrClusteredGridProbe)
		{
			profileLine += " + clustered grid probe";
		}
		if (probes.enablePbrLightPressureProbe)
		{
			profileLine += " + point light pressure rig";
		}
		if (rendererVerification.enableGpuTimingProbe)
		{
			profileLine += " + renderer GPU timing";
		}
		if (deferred.disablePbrDeferredTiledLights)
		{
			profileLine += " + tiled lights disabled";
		}
		if (deferred.pbrDeferredTileSizeOverride > 0)
		{
			profileLine += " + tiled light tile size " + std::to_string(deferred.pbrDeferredTileSizeOverride);
		}
		if (deferred.pbrDeferredTiledLightCutoffOverride > 0.0f)
		{
			profileLine += " + tiled light cutoff " + std::to_string(deferred.pbrDeferredTiledLightCutoffOverride);
		}
		if (passes.enablePbrGBufferDebugPass)
		{
			profileLine += " + PBR G-buffer debug pass";
		}
		if (passes.enableIblDebugPass)
		{
			profileLine += " + IBL debug pass";
		}
		if (passes.disablePbrShadowAtlasPass)
		{
			profileLine += " + PBR shadow atlas disabled";
		}
		reportLine(profileLine);
	}
}

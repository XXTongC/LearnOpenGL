#include "RuntimeApplicationShell.h"
#include "RuntimeBootstrapper.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "tools/Logger/LogManager.h"

#include <string>

int GLframework::PointLightShadow::MAX_POINT_LIGHTS = 2;

namespace
{
	bool hasArgument(int argc, char** argv, const std::string& expected)
	{
		for (int index = 1; index < argc; ++index)
		{
			if (expected == argv[index])
			{
				return true;
			}
		}

		return false;
	}

	GL_RUNTIME::RuntimeApplicationShellConfig makeShellConfig(int argc, char** argv)
	{
		GL_RUNTIME::RuntimeApplicationShellConfig config{};
		const bool verifyPbr = hasArgument(argc, argv, "--verify-pbr");
		const bool verifyPbrIblDebug = hasArgument(argc, argv, "--verify-pbr-ibl-debug");
		const bool verifyPbrGBuffer = hasArgument(argc, argv, "--verify-pbr-gbuffer");
		const bool verifyPbrGBufferDebug = hasArgument(argc, argv, "--verify-pbr-gbuffer-debug");
		const bool verifyPbrDeferred = hasArgument(argc, argv, "--verify-pbr-deferred");
		const bool verifyPbrDeferredNoAtlas = hasArgument(argc, argv, "--verify-pbr-deferred-no-atlas");
		const bool verifyPbrDeferredTransparent = hasArgument(argc, argv, "--verify-pbr-deferred-transparent");
		const bool verifyPbrDeferredEmissive = hasArgument(argc, argv, "--verify-pbr-deferred-emissive");
		const bool verifyPbrDeferredMaterialIbl = hasArgument(argc, argv, "--verify-pbr-deferred-material-ibl");
		const bool verifyPbrDeferredAlphaMask = hasArgument(argc, argv, "--verify-pbr-deferred-alpha-mask");
		const bool verifyPbrDeferredTiledLights = hasArgument(argc, argv, "--verify-pbr-deferred-tiled-lights");
		const bool verifyPbrDeferredTiledLights32 = hasArgument(argc, argv, "--verify-pbr-deferred-tiled-lights-32");
		const bool verifyPbrDeferredTiledHeatmap = hasArgument(argc, argv, "--verify-pbr-deferred-tiled-heatmap");
		const bool verifyPbrImport = hasArgument(argc, argv, "--verify-pbr-import");
		const bool verifyPbrNoAtlas = hasArgument(argc, argv, "--verify-pbr-no-atlas");
		if (verifyPbr || verifyPbrIblDebug || verifyPbrGBuffer || verifyPbrGBufferDebug || verifyPbrDeferred || verifyPbrDeferredNoAtlas || verifyPbrDeferredTransparent || verifyPbrDeferredEmissive || verifyPbrDeferredMaterialIbl || verifyPbrDeferredAlphaMask || verifyPbrDeferredTiledLights || verifyPbrDeferredTiledLights32 || verifyPbrDeferredTiledHeatmap || verifyPbrImport || verifyPbrNoAtlas)
		{
			config.window = { 1280, 720 };
			config.enableGui = false;
			config.pbrVerification.enabled = true;
			config.pbrVerification.maxFrames = 3;
			config.pbrVerification.captureFrame = 2;
			config.pbrVerification.capturePath = "out/pbr_verification.ppm";
			config.pbrVerification.disablePbrShadowAtlasPass = verifyPbrNoAtlas || verifyPbrDeferredNoAtlas;
			config.pbrVerification.enablePbrTransparentFallbackPass = verifyPbrDeferredTransparent;
			config.pbrVerification.enablePbrEmissiveProbe = verifyPbrDeferredEmissive;
			config.pbrVerification.enablePbrMaterialIblProbe = verifyPbrDeferredMaterialIbl;
			config.pbrVerification.enablePbrAlphaMaskProbe = verifyPbrDeferredAlphaMask;
			config.pbrVerification.enablePbrTiledLightProbe = verifyPbrDeferredTiledLights || verifyPbrDeferredTiledLights32;
			config.pbrVerification.enablePbrDeferredTiledLightDebugPass = verifyPbrDeferredTiledHeatmap;
			config.pbrVerification.enablePbrImportedAssetProbe = verifyPbrImport;
			if (verifyPbrDeferredTiledLights32)
			{
				config.pbrVerification.pbrDeferredTileSizeOverride = 32;
			}
			if (verifyPbrIblDebug)
			{
				config.pbrVerification.enableIblDebugPass = true;
				config.pbrVerification.capturePath = "out/pbr_ibl_debug_verification.ppm";
			}
			if (verifyPbrGBuffer)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.capturePath = "out/pbr_gbuffer_verification.ppm";
			}
			if (verifyPbrGBufferDebug)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrGBufferDebugPass = true;
				config.pbrVerification.capturePath = "out/pbr_gbuffer_debug_verification.ppm";
			}
			if (verifyPbrDeferred)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrDeferredLightingPass = true;
				config.pbrVerification.capturePath = "out/pbr_deferred_verification.ppm";
			}
			if (verifyPbrDeferredNoAtlas)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrDeferredLightingPass = true;
				config.pbrVerification.capturePath = "out/pbr_deferred_no_atlas_verification.ppm";
			}
			if (verifyPbrDeferredTransparent)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrDeferredLightingPass = true;
				config.pbrVerification.capturePath = "out/pbr_deferred_transparent_verification.ppm";
			}
			if (verifyPbrDeferredEmissive)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrDeferredLightingPass = true;
				config.pbrVerification.capturePath = "out/pbr_deferred_emissive_verification.ppm";
			}
			if (verifyPbrDeferredMaterialIbl)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrDeferredLightingPass = true;
				config.pbrVerification.capturePath = "out/pbr_deferred_material_ibl_verification.ppm";
			}
			if (verifyPbrDeferredAlphaMask)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrDeferredLightingPass = true;
				config.pbrVerification.capturePath = "out/pbr_deferred_alpha_mask_verification.ppm";
			}
			if (verifyPbrDeferredTiledLights)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrDeferredLightingPass = true;
				config.pbrVerification.capturePath = "out/pbr_deferred_tiled_lights_verification.ppm";
			}
			if (verifyPbrDeferredTiledLights32)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrDeferredLightingPass = true;
				config.pbrVerification.capturePath = "out/pbr_deferred_tiled_lights_32_verification.ppm";
			}
			if (verifyPbrDeferredTiledHeatmap)
			{
				config.pbrVerification.enablePbrGBufferPass = true;
				config.pbrVerification.enablePbrTiledLightProbe = true;
				config.pbrVerification.capturePath = "out/pbr_deferred_tiled_heatmap_verification.ppm";
			}
			if (verifyPbrImport)
			{
				config.pbrVerification.capturePath = "out/pbr_import_verification.ppm";
			}
		}

		return config;
	}
}

int main(int argc, char** argv)
{
	LogManager::getInstance().setMinLevel(LogManager::Level::info);
	GL_RUNTIME::RuntimeApplicationShell shell{ makeShellConfig(argc, argv) };
	return GL_RUNTIME::RuntimeBootstrapper::run(shell.makeCallbacks());
}

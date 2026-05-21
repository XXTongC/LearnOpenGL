#include "RuntimePBRVerificationArgs.h"

#include <string>

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

	struct PbrVerificationArgs
	{
		bool forward{ false };
		bool iblDebug{ false };
		bool gbuffer{ false };
		bool gbufferDebug{ false };
		bool deferred{ false };
		bool deferredNoAtlas{ false };
		bool deferredTransparent{ false };
		bool deferredEmissive{ false };
		bool deferredMaterialIbl{ false };
		bool deferredAlphaMask{ false };
		bool deferredUntiledLights{ false };
		bool deferredTiledLights{ false };
		bool deferredTiledLights32{ false };
		bool deferredTiledLightsCutoff005{ false };
		bool deferredTiledHeatmap{ false };
		bool importAsset{ false };
		bool textureSet{ false };
		bool deferredTextureSet{ false };
		bool forwardNoAtlas{ false };

		bool any() const
		{
			return forward
				|| iblDebug
				|| gbuffer
				|| gbufferDebug
				|| deferred
				|| deferredNoAtlas
				|| deferredTransparent
				|| deferredEmissive
				|| deferredMaterialIbl
				|| deferredAlphaMask
				|| deferredUntiledLights
				|| deferredTiledLights
				|| deferredTiledLights32
				|| deferredTiledLightsCutoff005
				|| deferredTiledHeatmap
				|| importAsset
				|| textureSet
				|| deferredTextureSet
				|| forwardNoAtlas;
		}
	};

	PbrVerificationArgs parsePbrVerificationArgs(int argc, char** argv)
	{
		return PbrVerificationArgs{
			hasArgument(argc, argv, "--verify-pbr"),
			hasArgument(argc, argv, "--verify-pbr-ibl-debug"),
			hasArgument(argc, argv, "--verify-pbr-gbuffer"),
			hasArgument(argc, argv, "--verify-pbr-gbuffer-debug"),
			hasArgument(argc, argv, "--verify-pbr-deferred"),
			hasArgument(argc, argv, "--verify-pbr-deferred-no-atlas"),
			hasArgument(argc, argv, "--verify-pbr-deferred-transparent"),
			hasArgument(argc, argv, "--verify-pbr-deferred-emissive"),
			hasArgument(argc, argv, "--verify-pbr-deferred-material-ibl"),
			hasArgument(argc, argv, "--verify-pbr-deferred-alpha-mask"),
			hasArgument(argc, argv, "--verify-pbr-deferred-untiled-lights"),
			hasArgument(argc, argv, "--verify-pbr-deferred-tiled-lights"),
			hasArgument(argc, argv, "--verify-pbr-deferred-tiled-lights-32"),
			hasArgument(argc, argv, "--verify-pbr-deferred-tiled-lights-cutoff-005"),
			hasArgument(argc, argv, "--verify-pbr-deferred-tiled-heatmap"),
			hasArgument(argc, argv, "--verify-pbr-import"),
			hasArgument(argc, argv, "--verify-pbr-texture-set"),
			hasArgument(argc, argv, "--verify-pbr-deferred-texture-set"),
			hasArgument(argc, argv, "--verify-pbr-no-atlas")
		};
	}

	void enableDeferredVerification(GL_RUNTIME::RuntimeApplicationShellConfig& config)
	{
		config.pbrVerification.enablePbrGBufferPass = true;
		config.pbrVerification.enablePbrDeferredLightingPass = true;
	}

	void applyPbrVerificationDefaults(
		GL_RUNTIME::RuntimeApplicationShellConfig& config,
		const PbrVerificationArgs& args
	)
	{
		config.window = { 1280, 720 };
		config.enableGui = false;
		config.pbrVerification.enabled = true;
		config.pbrVerification.maxFrames = 3;
		config.pbrVerification.captureFrame = 2;
		config.pbrVerification.capturePath = "out/pbr_verification.ppm";
		config.pbrVerification.disablePbrShadowAtlasPass = args.forwardNoAtlas || args.deferredNoAtlas;
		config.pbrVerification.enablePbrTransparentFallbackPass = args.deferredTransparent;
		config.pbrVerification.enablePbrEmissiveProbe = args.deferredEmissive;
		config.pbrVerification.enablePbrMaterialIblProbe = args.deferredMaterialIbl;
		config.pbrVerification.enablePbrAlphaMaskProbe = args.deferredAlphaMask;
		config.pbrVerification.enablePbrTiledLightProbe =
			args.deferredUntiledLights
			|| args.deferredTiledLights
			|| args.deferredTiledLights32
			|| args.deferredTiledLightsCutoff005;
		config.pbrVerification.enablePbrDeferredTiledLightDebugPass = args.deferredTiledHeatmap;
		config.pbrVerification.enablePbrImportedAssetProbe = args.importAsset;
		config.pbrVerification.enablePbrTextureSetProbe = args.textureSet || args.deferredTextureSet;
		config.pbrVerification.disablePbrDeferredTiledLights = args.deferredUntiledLights;
	}

	void applyPbrVerificationOverrides(
		GL_RUNTIME::RuntimeApplicationShellConfig& config,
		const PbrVerificationArgs& args
	)
	{
		if (args.deferredTiledLights32)
		{
			config.pbrVerification.pbrDeferredTileSizeOverride = 32;
		}
		if (args.deferredTiledLightsCutoff005)
		{
			config.pbrVerification.pbrDeferredTiledLightCutoffOverride = 0.05f;
		}
	}

	void applyPbrVerificationMode(
		GL_RUNTIME::RuntimeApplicationShellConfig& config,
		const PbrVerificationArgs& args
	)
	{
		if (args.iblDebug)
		{
			config.pbrVerification.enableIblDebugPass = true;
			config.pbrVerification.capturePath = "out/pbr_ibl_debug_verification.ppm";
		}
		if (args.gbuffer)
		{
			config.pbrVerification.enablePbrGBufferPass = true;
			config.pbrVerification.capturePath = "out/pbr_gbuffer_verification.ppm";
		}
		if (args.gbufferDebug)
		{
			config.pbrVerification.enablePbrGBufferPass = true;
			config.pbrVerification.enablePbrGBufferDebugPass = true;
			config.pbrVerification.capturePath = "out/pbr_gbuffer_debug_verification.ppm";
		}
		if (args.deferred)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_verification.ppm";
		}
		if (args.deferredNoAtlas)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_no_atlas_verification.ppm";
		}
		if (args.deferredTransparent)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_transparent_verification.ppm";
		}
		if (args.deferredEmissive)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_emissive_verification.ppm";
		}
		if (args.deferredMaterialIbl)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_material_ibl_verification.ppm";
		}
		if (args.deferredAlphaMask)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_alpha_mask_verification.ppm";
		}
		if (args.deferredUntiledLights)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_untiled_lights_verification.ppm";
		}
		if (args.deferredTiledLights)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_tiled_lights_verification.ppm";
		}
		if (args.deferredTiledLights32)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_tiled_lights_32_verification.ppm";
		}
		if (args.deferredTiledLightsCutoff005)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_tiled_lights_cutoff_005_verification.ppm";
		}
		if (args.deferredTiledHeatmap)
		{
			config.pbrVerification.enablePbrGBufferPass = true;
			config.pbrVerification.enablePbrTiledLightProbe = true;
			config.pbrVerification.capturePath = "out/pbr_deferred_tiled_heatmap_verification.ppm";
		}
		if (args.importAsset)
		{
			config.pbrVerification.capturePath = "out/pbr_import_verification.ppm";
		}
		if (args.textureSet)
		{
			config.pbrVerification.capturePath = "out/pbr_texture_set_verification.ppm";
		}
		if (args.deferredTextureSet)
		{
			enableDeferredVerification(config);
			config.pbrVerification.capturePath = "out/pbr_deferred_texture_set_verification.ppm";
		}
	}
}

GL_RUNTIME::RuntimeApplicationShellConfig GL_RUNTIME::makeShellConfigFromArguments(int argc, char** argv)
{
	RuntimeApplicationShellConfig config{};
	const auto pbrArgs = parsePbrVerificationArgs(argc, argv);
	if (!pbrArgs.any())
	{
		return config;
	}

	applyPbrVerificationDefaults(config, pbrArgs);
	applyPbrVerificationOverrides(config, pbrArgs);
	applyPbrVerificationMode(config, pbrArgs);
	return config;
}

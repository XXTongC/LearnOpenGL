#include "RuntimePBRVerificationArgs.h"

#include <string>

namespace
{
	enum PbrVerificationModeOption : unsigned int
	{
		OptionNone = 0u,
		OptionIblDebugPass = 1u << 0,
		OptionPbrGBufferPass = 1u << 1,
		OptionPbrDeferredLightingPass = 1u << 2,
		OptionPbrDeferredTiledLightDebugPass = 1u << 3,
		OptionPbrGBufferDebugPass = 1u << 4,
		OptionDisablePbrShadowAtlasPass = 1u << 5,
		OptionPbrTransparentFallbackPass = 1u << 6,
		OptionPbrEmissiveProbe = 1u << 7,
		OptionPbrMaterialIblProbe = 1u << 8,
		OptionPbrAlphaMaskProbe = 1u << 9,
		OptionPbrImportedAssetProbe = 1u << 10,
		OptionPbrTextureSetProbe = 1u << 11,
		OptionPbrTiledLightProbe = 1u << 12,
		OptionDisablePbrDeferredTiledLights = 1u << 13
	};

	struct PbrVerificationModeDescriptor
	{
		const char* argument{ nullptr };
		const char* capturePath{ nullptr };
		unsigned int options{ OptionNone };
		int tileSizeOverride{ 0 };
		float tiledLightCutoffOverride{ 0.0f };
	};

	constexpr unsigned int kDeferredPbrOptions =
		OptionPbrGBufferPass
		| OptionPbrDeferredLightingPass;

	constexpr PbrVerificationModeDescriptor kPbrVerificationModes[] = {
		{ "--verify-pbr", nullptr, OptionNone },
		{ "--verify-pbr-no-atlas", nullptr, OptionDisablePbrShadowAtlasPass },
		{ "--verify-pbr-ibl-debug", "out/pbr_ibl_debug_verification.ppm", OptionIblDebugPass },
		{ "--verify-pbr-gbuffer", "out/pbr_gbuffer_verification.ppm", OptionPbrGBufferPass },
		{ "--verify-pbr-gbuffer-debug", "out/pbr_gbuffer_debug_verification.ppm", OptionPbrGBufferPass | OptionPbrGBufferDebugPass },
		{ "--verify-pbr-deferred", "out/pbr_deferred_verification.ppm", kDeferredPbrOptions },
		{ "--verify-pbr-deferred-no-atlas", "out/pbr_deferred_no_atlas_verification.ppm", kDeferredPbrOptions | OptionDisablePbrShadowAtlasPass },
		{ "--verify-pbr-deferred-transparent", "out/pbr_deferred_transparent_verification.ppm", kDeferredPbrOptions | OptionPbrTransparentFallbackPass },
		{ "--verify-pbr-deferred-emissive", "out/pbr_deferred_emissive_verification.ppm", kDeferredPbrOptions | OptionPbrEmissiveProbe },
		{ "--verify-pbr-deferred-material-ibl", "out/pbr_deferred_material_ibl_verification.ppm", kDeferredPbrOptions | OptionPbrMaterialIblProbe },
		{ "--verify-pbr-deferred-alpha-mask", "out/pbr_deferred_alpha_mask_verification.ppm", kDeferredPbrOptions | OptionPbrAlphaMaskProbe },
		{ "--verify-pbr-deferred-untiled-lights", "out/pbr_deferred_untiled_lights_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe | OptionDisablePbrDeferredTiledLights },
		{ "--verify-pbr-deferred-tiled-lights", "out/pbr_deferred_tiled_lights_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe },
		{ "--verify-pbr-deferred-tiled-lights-32", "out/pbr_deferred_tiled_lights_32_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe, 32 },
		{ "--verify-pbr-deferred-tiled-lights-cutoff-005", "out/pbr_deferred_tiled_lights_cutoff_005_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe, 0, 0.05f },
		{ "--verify-pbr-deferred-tiled-heatmap", "out/pbr_deferred_tiled_heatmap_verification.ppm", OptionPbrGBufferPass | OptionPbrDeferredTiledLightDebugPass | OptionPbrTiledLightProbe },
		{ "--verify-pbr-import", "out/pbr_import_verification.ppm", OptionPbrImportedAssetProbe },
		{ "--verify-pbr-texture-set", "out/pbr_texture_set_verification.ppm", OptionPbrTextureSetProbe },
		{ "--verify-pbr-deferred-texture-set", "out/pbr_deferred_texture_set_verification.ppm", kDeferredPbrOptions | OptionPbrTextureSetProbe }
	};

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

	bool hasOption(unsigned int options, PbrVerificationModeOption option)
	{
		return (options & option) != 0u;
	}

	void applyPbrVerificationDefaults(GL_RUNTIME::RuntimeApplicationShellConfig& config)
	{
		config.window = { 1280, 720 };
		config.enableGui = false;
		config.pbrVerification.enabled = true;
		config.pbrVerification.maxFrames = 3;
		config.pbrVerification.captureFrame = 2;
		config.pbrVerification.capturePath = "out/pbr_verification.ppm";
	}

	void applyPbrVerificationMode(
		GL_RUNTIME::RuntimeApplicationShellConfig& config,
		const PbrVerificationModeDescriptor& mode
	)
	{
		auto& verification = config.pbrVerification;
		if (mode.capturePath != nullptr)
		{
			verification.capturePath = mode.capturePath;
		}

		verification.enableIblDebugPass |= hasOption(mode.options, OptionIblDebugPass);
		verification.enablePbrGBufferPass |= hasOption(mode.options, OptionPbrGBufferPass);
		verification.enablePbrDeferredLightingPass |= hasOption(mode.options, OptionPbrDeferredLightingPass);
		verification.enablePbrDeferredTiledLightDebugPass |= hasOption(mode.options, OptionPbrDeferredTiledLightDebugPass);
		verification.enablePbrGBufferDebugPass |= hasOption(mode.options, OptionPbrGBufferDebugPass);
		verification.disablePbrShadowAtlasPass |= hasOption(mode.options, OptionDisablePbrShadowAtlasPass);
		verification.enablePbrTransparentFallbackPass |= hasOption(mode.options, OptionPbrTransparentFallbackPass);
		verification.enablePbrEmissiveProbe |= hasOption(mode.options, OptionPbrEmissiveProbe);
		verification.enablePbrMaterialIblProbe |= hasOption(mode.options, OptionPbrMaterialIblProbe);
		verification.enablePbrAlphaMaskProbe |= hasOption(mode.options, OptionPbrAlphaMaskProbe);
		verification.enablePbrImportedAssetProbe |= hasOption(mode.options, OptionPbrImportedAssetProbe);
		verification.enablePbrTextureSetProbe |= hasOption(mode.options, OptionPbrTextureSetProbe);
		verification.enablePbrTiledLightProbe |= hasOption(mode.options, OptionPbrTiledLightProbe);
		verification.disablePbrDeferredTiledLights |= hasOption(mode.options, OptionDisablePbrDeferredTiledLights);

		if (mode.tileSizeOverride > 0)
		{
			verification.pbrDeferredTileSizeOverride = mode.tileSizeOverride;
		}
		if (mode.tiledLightCutoffOverride > 0.0f)
		{
			verification.pbrDeferredTiledLightCutoffOverride = mode.tiledLightCutoffOverride;
		}
	}

	bool applyRequestedPbrVerificationModes(
		GL_RUNTIME::RuntimeApplicationShellConfig& config,
		int argc,
		char** argv
	)
	{
		bool foundPbrVerificationMode = false;
		for (const auto& mode : kPbrVerificationModes)
		{
			if (!hasArgument(argc, argv, mode.argument))
			{
				continue;
			}

			if (!foundPbrVerificationMode)
			{
				applyPbrVerificationDefaults(config);
				foundPbrVerificationMode = true;
			}

			applyPbrVerificationMode(config, mode);
		}

		return foundPbrVerificationMode;
	}
}

GL_RUNTIME::RuntimeApplicationShellConfig GL_RUNTIME::makeShellConfigFromArguments(int argc, char** argv)
{
	RuntimeApplicationShellConfig config{};
	applyRequestedPbrVerificationModes(config, argc, argv);
	return config;
}

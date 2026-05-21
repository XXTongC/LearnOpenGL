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
		OptionPbrDeferredClusteredLightDebugPass = 1u << 4,
		OptionPbrGBufferDebugPass = 1u << 5,
		OptionDisablePbrShadowAtlasPass = 1u << 6,
		OptionPbrTransparentFallbackPass = 1u << 7,
		OptionPbrEmissiveProbe = 1u << 8,
		OptionPbrMaterialIblProbe = 1u << 9,
		OptionPbrAlphaMaskProbe = 1u << 10,
		OptionPbrImportedAssetProbe = 1u << 11,
		OptionPbrTextureSetProbe = 1u << 12,
		OptionPbrTiledLightProbe = 1u << 13,
		OptionPbrClusteredLayoutProbe = 1u << 14,
		OptionPbrClusteredGridProbe = 1u << 15,
		OptionDisablePbrDeferredTiledLights = 1u << 16,
		OptionPbrClusteredStatsReadback = 1u << 17,
		OptionPbrLightPressureProbe = 1u << 18,
		OptionRendererGpuTimingProbe = 1u << 19
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
		{ "--verify-pbr-deferred-tiled-lights-pressure", "out/pbr_deferred_tiled_lights_pressure_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe | OptionPbrLightPressureProbe },
		{ "--verify-pbr-deferred-tiled-lights-32", "out/pbr_deferred_tiled_lights_32_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe, 32 },
		{ "--verify-pbr-deferred-tiled-lights-cutoff-005", "out/pbr_deferred_tiled_lights_cutoff_005_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe, 0, 0.05f },
		{ "--verify-pbr-deferred-tiled-heatmap", "out/pbr_deferred_tiled_heatmap_verification.ppm", OptionPbrGBufferPass | OptionPbrDeferredTiledLightDebugPass | OptionPbrTiledLightProbe },
		{ "--verify-pbr-deferred-clustered-heatmap", "out/pbr_deferred_clustered_heatmap_verification.ppm", OptionPbrGBufferPass | OptionPbrDeferredClusteredLightDebugPass | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights },
		{ "--verify-pbr-deferred-clustered-layout", "out/pbr_deferred_clustered_layout_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredLayoutProbe | OptionDisablePbrDeferredTiledLights },
		{ "--verify-pbr-deferred-clustered-grid", "out/pbr_deferred_clustered_grid_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights | OptionPbrClusteredStatsReadback },
		{ "--verify-pbr-deferred-clustered-grid-timing", "out/pbr_deferred_clustered_grid_timing_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights | OptionPbrClusteredStatsReadback | OptionRendererGpuTimingProbe },
		{ "--verify-pbr-deferred-clustered-grid-pressure", "out/pbr_deferred_clustered_grid_pressure_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights | OptionPbrClusteredStatsReadback | OptionPbrLightPressureProbe },
		{ "--verify-pbr-deferred-clustered-grid-no-readback", "out/pbr_deferred_clustered_grid_no_readback_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights },
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
		verification.enablePbrDeferredClusteredLightDebugPass |= hasOption(mode.options, OptionPbrDeferredClusteredLightDebugPass);
		verification.enablePbrGBufferDebugPass |= hasOption(mode.options, OptionPbrGBufferDebugPass);
		verification.disablePbrShadowAtlasPass |= hasOption(mode.options, OptionDisablePbrShadowAtlasPass);
		verification.enablePbrTransparentFallbackPass |= hasOption(mode.options, OptionPbrTransparentFallbackPass);
		verification.enablePbrEmissiveProbe |= hasOption(mode.options, OptionPbrEmissiveProbe);
		verification.enablePbrMaterialIblProbe |= hasOption(mode.options, OptionPbrMaterialIblProbe);
		verification.enablePbrAlphaMaskProbe |= hasOption(mode.options, OptionPbrAlphaMaskProbe);
		verification.enablePbrImportedAssetProbe |= hasOption(mode.options, OptionPbrImportedAssetProbe);
		verification.enablePbrTextureSetProbe |= hasOption(mode.options, OptionPbrTextureSetProbe);
		verification.enablePbrTiledLightProbe |= hasOption(mode.options, OptionPbrTiledLightProbe);
		verification.enablePbrClusteredLayoutProbe |= hasOption(mode.options, OptionPbrClusteredLayoutProbe);
		verification.enablePbrClusteredGridProbe |= hasOption(mode.options, OptionPbrClusteredGridProbe);
		verification.enablePbrClusteredStatsReadback |= hasOption(mode.options, OptionPbrClusteredStatsReadback);
		verification.enablePbrLightPressureProbe |= hasOption(mode.options, OptionPbrLightPressureProbe);
		verification.enableRendererGpuTimingProbe |= hasOption(mode.options, OptionRendererGpuTimingProbe);
		verification.disablePbrDeferredTiledLights |= hasOption(mode.options, OptionDisablePbrDeferredTiledLights);
		if (hasOption(mode.options, OptionRendererGpuTimingProbe))
		{
			verification.maxFrames = 5;
			verification.captureFrame = 5;
		}

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

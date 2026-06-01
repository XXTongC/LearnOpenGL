#include "RuntimeVerificationArgs.h"

#include <limits>
#include <string>

#include "RuntimeApplicationConfig.h"
#include "RuntimeRendererBackendKeys.h"

namespace
{
	enum RuntimeVerificationModeOption : unsigned int
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
		OptionRendererGpuTimingProbe = 1u << 19,
		OptionPbrShowcaseSpheres = 1u << 20,
		OptionEngineWorldSceneProbe = 1u << 21,
		OptionEngineWorldMinimalScene = 1u << 22,
		OptionEngineWorldEditorCreate = 1u << 23,
		OptionEngineWorldScenePackageRoundTrip = 1u << 24,
		OptionRendererBackendNoOp = 1u << 25
	};

	struct RuntimeVerificationModeDescriptor
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

	constexpr RuntimeVerificationModeDescriptor kRuntimeVerificationModes[] = {
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
		{ "--verify-pbr-deferred-tiled-lights-pressure-timing", "out/pbr_deferred_tiled_lights_pressure_timing_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe | OptionPbrLightPressureProbe | OptionRendererGpuTimingProbe },
		{ "--verify-pbr-deferred-tiled-lights-32", "out/pbr_deferred_tiled_lights_32_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe, 32 },
		{ "--verify-pbr-deferred-tiled-lights-cutoff-005", "out/pbr_deferred_tiled_lights_cutoff_005_verification.ppm", kDeferredPbrOptions | OptionPbrTiledLightProbe, 0, 0.05f },
		{ "--verify-pbr-deferred-tiled-heatmap", "out/pbr_deferred_tiled_heatmap_verification.ppm", OptionPbrGBufferPass | OptionPbrDeferredTiledLightDebugPass | OptionPbrTiledLightProbe },
		{ "--verify-pbr-deferred-clustered-heatmap", "out/pbr_deferred_clustered_heatmap_verification.ppm", OptionPbrGBufferPass | OptionPbrDeferredClusteredLightDebugPass | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights },
		{ "--verify-pbr-deferred-clustered-layout", "out/pbr_deferred_clustered_layout_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredLayoutProbe | OptionDisablePbrDeferredTiledLights },
		{ "--verify-pbr-deferred-clustered-grid", "out/pbr_deferred_clustered_grid_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights | OptionPbrClusteredStatsReadback },
		{ "--verify-pbr-deferred-clustered-grid-timing", "out/pbr_deferred_clustered_grid_timing_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights | OptionPbrClusteredStatsReadback | OptionRendererGpuTimingProbe },
		{ "--verify-pbr-deferred-clustered-grid-pressure", "out/pbr_deferred_clustered_grid_pressure_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights | OptionPbrClusteredStatsReadback | OptionPbrLightPressureProbe },
		{ "--verify-pbr-deferred-clustered-grid-pressure-timing", "out/pbr_deferred_clustered_grid_pressure_timing_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights | OptionPbrClusteredStatsReadback | OptionPbrLightPressureProbe | OptionRendererGpuTimingProbe },
		{ "--verify-pbr-deferred-clustered-grid-no-readback", "out/pbr_deferred_clustered_grid_no_readback_verification.ppm", kDeferredPbrOptions | OptionPbrClusteredGridProbe | OptionDisablePbrDeferredTiledLights },
		{ "--verify-pbr-import", "out/pbr_import_verification.ppm", OptionPbrImportedAssetProbe },
		{ "--verify-pbr-texture-set", "out/pbr_texture_set_verification.ppm", OptionPbrTextureSetProbe },
		{ "--verify-pbr-deferred-texture-set", "out/pbr_deferred_texture_set_verification.ppm", kDeferredPbrOptions | OptionPbrTextureSetProbe },
		{ "--verify-pbr-showcase-spheres", "out/pbr_showcase_spheres_verification.ppm", kDeferredPbrOptions | OptionPbrShowcaseSpheres | OptionPbrTiledLightProbe | OptionPbrLightPressureProbe },
		{ "--verify-engine-world-scene-probe", "out/engine_world_scene_probe_verification.ppm", kDeferredPbrOptions | OptionEngineWorldSceneProbe },
		{ "--verify-engine-world-minimal-scene", "out/engine_world_minimal_scene_verification.ppm", kDeferredPbrOptions | OptionEngineWorldMinimalScene },
		{ "--verify-engine-world-editor-create", "out/engine_world_editor_create_verification.ppm", kDeferredPbrOptions | OptionEngineWorldMinimalScene | OptionEngineWorldEditorCreate },
		{ "--verify-engine-world-scene-package", "out/engine_world_scene_package_verification.ppm", kDeferredPbrOptions | OptionEngineWorldMinimalScene | OptionEngineWorldScenePackageRoundTrip },
		{ "--verify-renderer-backend-registry-noop", "out/renderer_backend_registry_noop_verification.ppm", OptionRendererBackendNoOp }
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

	bool hasOption(unsigned int options, RuntimeVerificationModeOption option)
	{
		return (options & option) != 0u;
	}

	void applyRuntimeVerificationDefaults(GL_RUNTIME::RuntimeApplicationShellConfig& config)
	{
		config.window = { 1280, 720 };
		config.enableGui = false;
		config.verification.enabled = true;
		config.verification.maxFrames = 3;
		config.verification.captureFrame = 2;
		config.verification.capturePath = "out/pbr_verification.ppm";
	}

	void applyRuntimeVerificationMode(
		GL_RUNTIME::RuntimeApplicationShellConfig& config,
		const RuntimeVerificationModeDescriptor& mode
	)
	{
		auto& verification = config.verification;
		auto& pbr = verification.pbr;
		auto& pbrPasses = pbr.passes;
		auto& pbrProbes = pbr.probes;
		auto& pbrDeferred = pbr.deferred;
		auto& engineWorld = verification.engineWorld;
		auto& renderer = verification.renderer;
		if (mode.capturePath != nullptr)
		{
			verification.capturePath = mode.capturePath;
		}

		pbrPasses.enableIblDebugPass |= hasOption(mode.options, OptionIblDebugPass);
		pbrPasses.enablePbrGBufferPass |= hasOption(mode.options, OptionPbrGBufferPass);
		pbrPasses.enablePbrDeferredLightingPass |= hasOption(mode.options, OptionPbrDeferredLightingPass);
		pbrPasses.enablePbrDeferredTiledLightDebugPass |= hasOption(mode.options, OptionPbrDeferredTiledLightDebugPass);
		pbrPasses.enablePbrDeferredClusteredLightDebugPass |= hasOption(mode.options, OptionPbrDeferredClusteredLightDebugPass);
		pbrPasses.enablePbrGBufferDebugPass |= hasOption(mode.options, OptionPbrGBufferDebugPass);
		pbrPasses.disablePbrShadowAtlasPass |= hasOption(mode.options, OptionDisablePbrShadowAtlasPass);
		pbrProbes.enablePbrTransparentFallbackPass |= hasOption(mode.options, OptionPbrTransparentFallbackPass);
		pbrProbes.enablePbrEmissiveProbe |= hasOption(mode.options, OptionPbrEmissiveProbe);
		pbrProbes.enablePbrMaterialIblProbe |= hasOption(mode.options, OptionPbrMaterialIblProbe);
		pbrProbes.enablePbrAlphaMaskProbe |= hasOption(mode.options, OptionPbrAlphaMaskProbe);
		pbrProbes.enablePbrImportedAssetProbe |= hasOption(mode.options, OptionPbrImportedAssetProbe);
		pbrProbes.enablePbrTextureSetProbe |= hasOption(mode.options, OptionPbrTextureSetProbe);
		pbrProbes.enablePbrShowcaseSpheres |= hasOption(mode.options, OptionPbrShowcaseSpheres);
		engineWorld.enableSceneProbe |= hasOption(mode.options, OptionEngineWorldSceneProbe);
		engineWorld.enableMinimalScene |= hasOption(mode.options, OptionEngineWorldMinimalScene);
		engineWorld.enableEditorCreate |= hasOption(mode.options, OptionEngineWorldEditorCreate);
		engineWorld.enableScenePackageRoundTrip |= hasOption(mode.options, OptionEngineWorldScenePackageRoundTrip);
		pbrProbes.enablePbrTiledLightProbe |= hasOption(mode.options, OptionPbrTiledLightProbe);
		pbrProbes.enablePbrClusteredLayoutProbe |= hasOption(mode.options, OptionPbrClusteredLayoutProbe);
		pbrProbes.enablePbrClusteredGridProbe |= hasOption(mode.options, OptionPbrClusteredGridProbe);
		pbrProbes.enablePbrClusteredStatsReadback |= hasOption(mode.options, OptionPbrClusteredStatsReadback);
		pbrProbes.enablePbrLightPressureProbe |= hasOption(mode.options, OptionPbrLightPressureProbe);
		renderer.enableGpuTimingProbe |= hasOption(mode.options, OptionRendererGpuTimingProbe);
		pbrDeferred.disablePbrDeferredTiledLights |= hasOption(mode.options, OptionDisablePbrDeferredTiledLights);
		if (hasOption(mode.options, OptionRendererGpuTimingProbe))
		{
			verification.maxFrames = 5;
			verification.captureFrame = 5;
		}

		if (mode.tileSizeOverride > 0)
		{
			pbrDeferred.pbrDeferredTileSizeOverride = mode.tileSizeOverride;
		}
		if (mode.tiledLightCutoffOverride > 0.0f)
		{
			pbrDeferred.pbrDeferredTiledLightCutoffOverride = mode.tiledLightCutoffOverride;
		}
		if (hasOption(mode.options, OptionRendererBackendNoOp))
		{
			config.rendererBackendKey = GL_RUNTIME::RuntimeRendererBackendKeys::testNoOpBackendKey();
		}
	}

	bool applyRequestedRuntimeVerificationModes(
		GL_RUNTIME::RuntimeApplicationShellConfig& config,
		int argc,
		char** argv
	)
	{
		bool foundRuntimeVerificationMode = false;
		for (const auto& mode : kRuntimeVerificationModes)
		{
			if (!hasArgument(argc, argv, mode.argument))
			{
				continue;
			}

			if (!foundRuntimeVerificationMode)
			{
				applyRuntimeVerificationDefaults(config);
				foundRuntimeVerificationMode = true;
			}

			applyRuntimeVerificationMode(config, mode);
		}

		return foundRuntimeVerificationMode;
	}

	void applyEditorUiModuleArguments(GL_RUNTIME::RuntimeApplicationShellConfig& config, int argc, char** argv)
	{
		for (int index = 1; index < argc; ++index)
		{
			const std::string argument = argv[index];
			if (argument == "--enable-core-editor-ui-module")
			{
				config.enableCoreEditorUiModule = true;
			}
			else if (argument == "--disable-core-editor-ui-module")
			{
				config.enableCoreEditorUiModule = false;
			}
			else if (argument == "--enable-sample-editor-ui-module")
			{
				config.enableSampleEditorUiModule = true;
			}
			else if (argument == "--disable-sample-editor-ui-module")
			{
				config.enableSampleEditorUiModule = false;
			}
		}
	}

	void applyInteractivePbrShowcaseMode(GL_RUNTIME::RuntimeApplicationShellConfig& config)
	{
		applyRuntimeVerificationDefaults(config);
		config.enableGui = false;
		config.verification.maxFrames = std::numeric_limits<int>::max();
		config.verification.captureFrame = std::numeric_limits<int>::max();

		RuntimeVerificationModeDescriptor showcaseMode{
			"--pbr-showcase-spheres",
			nullptr,
			kDeferredPbrOptions | OptionPbrShowcaseSpheres | OptionPbrTiledLightProbe | OptionPbrLightPressureProbe
		};
		applyRuntimeVerificationMode(config, showcaseMode);
	}
}

GL_RUNTIME::RuntimeApplicationShellConfig GL_RUNTIME::makeShellConfigFromArguments(int argc, char** argv)
{
	RuntimeApplicationShellConfig config{};
	if (!applyRequestedRuntimeVerificationModes(config, argc, argv)
		&& hasArgument(argc, argv, "--pbr-showcase-spheres"))
	{
		applyInteractivePbrShowcaseMode(config);
	}
	applyEditorUiModuleArguments(config, argc, argv);
	return config;
}

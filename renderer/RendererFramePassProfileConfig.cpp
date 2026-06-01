#include "RendererFramePassProfileConfig.h"

#include "RendererFramePassProfile.h"
#include "tools/inspector/PropertySchema.h"

using namespace GLframework;

void GLframework::buildRendererFramePassProfileConfigSchema(
	GL_EDITOR::PropertyBuilder& builder,
	RendererFramePassProfile& profile
)
{
	builder.addSection("Renderer Pass Plan");
	builder.addConfigString("defaultPassOrder", "Default Pass Order", &profile.defaultPassOrder);
	builder.addConfigString("globalMaterialOverridePassOrder", "Global Material Override Pass Order", &profile.globalMaterialOverridePassOrder);
	builder.addConfigBool("rendererGpuTimingEnabled", "Renderer GPU Timing", &profile.rendererGpuTimingEnabled);
	builder.addText("Renderer GPU Timing Note", "Uses GL_TIME_ELAPSED with deferred readback from older frames. Keep disabled for normal rendering; enable for profiling and verification only.");
	builder.addText(
		"Available Pass Keys",
		"BeginFrame, ShadowMaps, PBRShadowAtlas, GlobalMaterialScene, PBRDepthPrepass, PBRGBuffer, PBRDeferredLighting, PBRDeferredTiledLightDebug, PBRDeferredClusteredLightDebug, PBRGBufferDebug, LegacyOpaqueScene, PBROpaqueScene, LegacyTransparentScene, PBRTransparentScene, IBLDebug"
	);
	builder.addSection("PBR Deferred Lighting Pass");
	builder.addConfigFloat("pbrDeferredLightingIntensity", "PBR Deferred Lighting Intensity", &profile.pbrDeferredLightingIntensity, 0.0f, 8.0f);
	builder.addConfigFloat("pbrDeferredIblDiffuseStrength", "PBR Deferred IBL Diffuse Strength", &profile.pbrDeferredIblDiffuseStrength, 0.0f, 8.0f);
	builder.addConfigFloat("pbrDeferredIblSpecularStrength", "PBR Deferred IBL Specular Strength", &profile.pbrDeferredIblSpecularStrength, 0.0f, 8.0f);
	builder.addConfigBool("pbrDeferredTiledLightsEnabled", "PBR Deferred Tiled Lights", &profile.pbrDeferredTiledLightsEnabled);
	builder.addConfigInt("pbrDeferredTileSize", "PBR Deferred Tile Size", &profile.pbrDeferredTileSize, 8, 64);
	builder.addConfigFloat("pbrDeferredTiledLightCutoff", "PBR Deferred Tiled Light Cutoff", &profile.pbrDeferredTiledLightCutoff, 0.001f, 1.0f);
	builder.addConfigBool("pbrDeferredClusteredLightsEnabled", "PBR Deferred Clustered Lights", &profile.pbrDeferredClusteredLightsEnabled);
	builder.addConfigBool("pbrDeferredClusteredLayoutStatsEnabled", "PBR Clustered Layout Stats", &profile.pbrDeferredClusteredLayoutStatsEnabled);
	builder.addConfigInt("pbrDeferredClusteredDepthSlices", "PBR Deferred Clustered Depth Slices", &profile.pbrDeferredClusteredDepthSlices, 1, 128);
	builder.addConfigInt("pbrDeferredClusteredMaxLightsPerCluster", "PBR Deferred Clustered Max Lights", &profile.pbrDeferredClusteredMaxLightsPerCluster, 1, 256);
	builder.addConfigBool("pbrDeferredClusteredStatsReadbackEnabled", "PBR Clustered Stats Readback", &profile.pbrDeferredClusteredStatsReadbackEnabled);
	builder.addConfigInt("pbrDeferredTiledLightDebugMaxLights", "PBR Tiled Light Debug Max Lights", &profile.pbrDeferredTiledLightDebugMaxLights, 1, 16);
	builder.addConfigFloat("pbrDeferredTiledLightDebugIntensity", "PBR Tiled Light Debug Intensity", &profile.pbrDeferredTiledLightDebugIntensity, 0.0f, 8.0f);
	builder.addConfigInt("pbrDeferredClusteredLightDebugDepthSlice", "PBR Clustered Debug Depth Slice", &profile.pbrDeferredClusteredLightDebugDepthSlice, -1, 128);
	builder.addConfigInt("pbrDeferredClusteredLightDebugMaxLights", "PBR Clustered Debug Max Lights", &profile.pbrDeferredClusteredLightDebugMaxLights, 1, 16);
	builder.addConfigFloat("pbrDeferredClusteredLightDebugIntensity", "PBR Clustered Debug Intensity", &profile.pbrDeferredClusteredLightDebugIntensity, 0.0f, 8.0f);
	builder.addSection("PBR GBuffer Debug Pass");
	builder.addConfigInt("pbrGBufferDebugMode", "PBR GBuffer Debug Mode", &profile.pbrGBufferDebugMode, 0, 8);
	builder.addConfigFloat("pbrGBufferDebugIntensity", "PBR GBuffer Debug Intensity", &profile.pbrGBufferDebugIntensity, 0.0f, 8.0f);
	builder.addText("PBR GBuffer Debug Modes", "0 = Albedo, 1 = Normal, 2 = Roughness, 3 = Metallic, 4 = AO, 5 = Depth, 6 = World Position, 7 = Emissive, 8 = IBL Params");
	builder.addSection("IBL Debug Pass");
	builder.addConfigInt("iblDebugMode", "IBL Debug Mode", &profile.iblDebugMode, 0, 3);
	builder.addConfigFloat("iblDebugMipLevel", "IBL Debug Mip Level", &profile.iblDebugMipLevel, 0.0f, 8.0f);
	builder.addConfigFloat("iblDebugIntensity", "IBL Debug Intensity", &profile.iblDebugIntensity, 0.0f, 8.0f);
	builder.addText("IBL Debug Modes", "0 = Environment, 1 = Irradiance, 2 = Prefilter, 3 = BRDF LUT");
}

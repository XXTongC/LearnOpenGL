#include "RendererFramePassProfile.h"

#include "RendererFramePassRegistry.h"
#include "tools/config/ProfileConfigIO.h"
#include "tools/inspector/PropertySchema.h"

using namespace GLframework;

RendererFramePassProfile::RendererFramePassProfile()
{
	resetToDefaults();
}

void RendererFramePassProfile::resetToDefaults()
{
	defaultPassOrder = RendererFramePassRegistry::defaultPassOrder();
	globalMaterialOverridePassOrder = RendererFramePassRegistry::globalMaterialOverridePassOrder();
	pbrGBufferDebugMode = 0;
	pbrGBufferDebugIntensity = 1.0f;
	pbrDeferredLightingIntensity = 1.0f;
	pbrDeferredIblDiffuseStrength = 1.0f;
	pbrDeferredIblSpecularStrength = 1.0f;
	iblDebugMode = 0;
	iblDebugMipLevel = 0.0f;
	iblDebugIntensity = 1.0f;
}

void RendererFramePassProfile::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	builder.addSection("Renderer Pass Plan");
	builder.addConfigString("defaultPassOrder", "Default Pass Order", &defaultPassOrder);
	builder.addConfigString("globalMaterialOverridePassOrder", "Global Material Override Pass Order", &globalMaterialOverridePassOrder);
	builder.addText(
		"Available Pass Keys",
		"BeginFrame, ShadowMaps, PBRShadowAtlas, GlobalMaterialScene, PBRDepthPrepass, PBRGBuffer, PBRDeferredLighting, PBRGBufferDebug, LegacyOpaqueScene, PBROpaqueScene, LegacyTransparentScene, PBRTransparentScene, IBLDebug"
	);
	builder.addSection("PBR Deferred Lighting Pass");
	builder.addConfigFloat("pbrDeferredLightingIntensity", "PBR Deferred Lighting Intensity", &pbrDeferredLightingIntensity, 0.0f, 8.0f);
	builder.addConfigFloat("pbrDeferredIblDiffuseStrength", "PBR Deferred IBL Diffuse Strength", &pbrDeferredIblDiffuseStrength, 0.0f, 8.0f);
	builder.addConfigFloat("pbrDeferredIblSpecularStrength", "PBR Deferred IBL Specular Strength", &pbrDeferredIblSpecularStrength, 0.0f, 8.0f);
	builder.addSection("PBR GBuffer Debug Pass");
	builder.addConfigInt("pbrGBufferDebugMode", "PBR GBuffer Debug Mode", &pbrGBufferDebugMode, 0, 6);
	builder.addConfigFloat("pbrGBufferDebugIntensity", "PBR GBuffer Debug Intensity", &pbrGBufferDebugIntensity, 0.0f, 8.0f);
	builder.addText("PBR GBuffer Debug Modes", "0 = Albedo, 1 = Normal, 2 = Roughness, 3 = Metallic, 4 = AO, 5 = Depth, 6 = World Position");
	builder.addSection("IBL Debug Pass");
	builder.addConfigInt("iblDebugMode", "IBL Debug Mode", &iblDebugMode, 0, 3);
	builder.addConfigFloat("iblDebugMipLevel", "IBL Debug Mip Level", &iblDebugMipLevel, 0.0f, 8.0f);
	builder.addConfigFloat("iblDebugIntensity", "IBL Debug Intensity", &iblDebugIntensity, 0.0f, 8.0f);
	builder.addText("IBL Debug Modes", "0 = Environment, 1 = Irradiance, 2 = Prefilter, 3 = BRDF LUT");
}

std::string RendererFramePassProfileStorage::defaultPath()
{
	return "config/renderer_frame_pass.local.ini";
}

bool RendererFramePassProfileStorage::loadFromFile(const std::string& path, RendererFramePassProfile& profile)
{
	RendererFramePassProfile loadedProfile = profile;
	GL_EDITOR::PropertyBuilder builder{};
	loadedProfile.visitEditableProperties(builder);
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
	snapshot.visitEditableProperties(builder);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local renderer frame pass plan for PBR render-path experiments",
		builder
	);
}

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
		"BeginFrame, ShadowMaps, GlobalMaterialScene, PBRDepthPrepass, PBRGBuffer, LegacyOpaqueScene, PBROpaqueScene, LegacyTransparentScene, PBRTransparentScene, IBLDebug"
	);
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

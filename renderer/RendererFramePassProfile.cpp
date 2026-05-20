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
}

void RendererFramePassProfile::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	builder.addSection("Renderer Pass Plan");
	builder.addConfigString("defaultPassOrder", "Default Pass Order", &defaultPassOrder);
	builder.addConfigString("globalMaterialOverridePassOrder", "Global Material Override Pass Order", &globalMaterialOverridePassOrder);
	builder.addText(
		"Available Pass Keys",
		"BeginFrame, ShadowMaps, GlobalMaterialScene, PBRDepthPrepass, LegacyOpaqueScene, PBROpaqueScene, LegacyTransparentScene, PBRTransparentScene"
	);
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

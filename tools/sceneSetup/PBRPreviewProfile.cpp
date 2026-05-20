#include "PBRPreviewProfile.h"

#include "../config/ProfileConfigIO.h"
#include "../inspector/PropertySchema.h"

namespace
{
	int toEditableTextureUnit(unsigned int unit)
	{
		return static_cast<int>(unit);
	}
}

std::string GL_SCENE::PBRPreviewProfileStorage::defaultPath()
{
	return "config/pbr_preview.local.ini";
}

void GL_SCENE::PBRPreviewProfile::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	builder.addSection("PBR Preview");
	builder.addConfigBool("enabled", "Enabled", &enabled);
	builder.addConfigVec3({ "positionX", "positionY", "positionZ" }, "Position", &position);

	builder.addSection("Geometry");
	builder.addConfigFloat("radius", "Radius", &radius, 0.05f, 5.0f);
	builder.addConfigInt("segments", "Segments", &segments, 8, 128);
	builder.addConfigInt("rings", "Rings", &rings, 4, 128);

	builder.addSection("Material Grid");
	builder.addConfigBool("useMaterialGrid", "Use Material Grid", &useMaterialGrid);
	builder.addConfigInt("gridColumns", "Grid Columns", &gridColumns, 1, 10);
	builder.addConfigInt("gridRows", "Grid Rows", &gridRows, 1, 10);
	builder.addConfigFloat("gridSpacing", "Grid Spacing", &gridSpacing, 0.1f, 5.0f);
	builder.addConfigFloat("gridRadius", "Grid Radius", &gridRadius, 0.05f, 2.0f);
	builder.addConfigFloat("gridMetallicMin", "Grid Metallic Min", &gridMetallicMin, 0.0f, 1.0f);
	builder.addConfigFloat("gridMetallicMax", "Grid Metallic Max", &gridMetallicMax, 0.0f, 1.0f);
	builder.addConfigFloat("gridRoughnessMin", "Grid Roughness Min", &gridRoughnessMin, 0.04f, 1.0f);
	builder.addConfigFloat("gridRoughnessMax", "Grid Roughness Max", &gridRoughnessMax, 0.04f, 1.0f);

	builder.addSection("PBR Surface");
	builder.addConfigColor3({ "albedoR", "albedoG", "albedoB" }, "Albedo", &albedo);
	builder.addConfigFloat("metallic", "Metallic", &metallic, 0.0f, 1.0f);
	builder.addConfigFloat("roughness", "Roughness", &roughness, 0.04f, 1.0f);
	builder.addConfigFloat("ao", "AO", &ao, 0.0f, 1.0f);

	builder.addSection("IBL");
	builder.addConfigBool("useIBL", "Use IBL", &useIBL);
	builder.addConfigFloat("iblDiffuseStrength", "IBL Diffuse Strength", &iblDiffuseStrength, 0.0f, 5.0f);
	builder.addConfigFloat("iblSpecularStrength", "IBL Specular Strength", &iblSpecularStrength, 0.0f, 5.0f);

	builder.addSection("Textures");
	builder.addConfigString("normalMapPath", "Normal Map Path", &normalMapPath);
	builder.addConfigInt(
		"normalMapUnit",
		"Normal Map Unit",
		[this]() { return toEditableTextureUnit(normalMapUnit); },
		[this](int value) { normalMapUnit = value < 0 ? 0u : static_cast<unsigned int>(value); },
		0,
		31
	);
}

bool GL_SCENE::PBRPreviewProfileStorage::loadFromFile(const std::string& path, PBRPreviewProfile& profile)
{
	PBRPreviewProfile loadedProfile = profile;
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

bool GL_SCENE::PBRPreviewProfileStorage::saveToFile(const std::string& path, const PBRPreviewProfile& profile)
{
	PBRPreviewProfile snapshot = profile;
	GL_EDITOR::PropertyBuilder builder{};
	snapshot.visitEditableProperties(builder);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local PBR preview profile for material and IBL experiments",
		builder
	);
}

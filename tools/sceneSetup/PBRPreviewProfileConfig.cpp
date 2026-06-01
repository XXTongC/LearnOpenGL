#include "PBRPreviewProfileConfig.h"

#include "PBRPreviewProfile.h"
#include "../../materials/pbrMaterial/PBRMaterialProfileConfig.h"
#include "../inspector/PropertySchema.h"

namespace
{
	int toEditableTextureUnit(unsigned int unit)
	{
		return static_cast<int>(unit);
	}
}

void GL_SCENE::buildPBRPreviewProfileConfigSchema(
	GL_EDITOR::PropertyBuilder& builder,
	PBRPreviewProfile& profile
)
{
	builder.addSection("PBR Preview");
	builder.addConfigBool("enabled", "Enabled", &profile.enabled);
	builder.addConfigVec3({ "positionX", "positionY", "positionZ" }, "Position", &profile.position);

	builder.addSection("Geometry");
	builder.addConfigFloat("radius", "Radius", &profile.radius, 0.05f, 5.0f);
	builder.addConfigInt("segments", "Segments", &profile.segments, 8, 128);
	builder.addConfigInt("rings", "Rings", &profile.rings, 4, 128);

	builder.addSection("Material Grid");
	builder.addConfigBool("useMaterialGrid", "Use Material Grid", &profile.useMaterialGrid);
	builder.addConfigInt("gridColumns", "Grid Columns", &profile.gridColumns, 1, 10);
	builder.addConfigInt("gridRows", "Grid Rows", &profile.gridRows, 1, 10);
	builder.addConfigFloat("gridSpacing", "Grid Spacing", &profile.gridSpacing, 0.1f, 5.0f);
	builder.addConfigFloat("gridRadius", "Grid Radius", &profile.gridRadius, 0.05f, 2.0f);
	builder.addConfigFloat("gridMetallicMin", "Grid Metallic Min", &profile.gridMetallicMin, 0.0f, 1.0f);
	builder.addConfigFloat("gridMetallicMax", "Grid Metallic Max", &profile.gridMetallicMax, 0.0f, 1.0f);
	builder.addConfigFloat("gridRoughnessMin", "Grid Roughness Min", &profile.gridRoughnessMin, 0.04f, 1.0f);
	builder.addConfigFloat("gridRoughnessMax", "Grid Roughness Max", &profile.gridRoughnessMax, 0.04f, 1.0f);

	builder.addSection("Material Preset");
	builder.addConfigString("materialProfilePath", "Material Profile Path", &profile.materialProfilePath);

	GLframework::buildPBRMaterialProfileConfigSchema(builder, profile.material);

	builder.addSection("Textures");
	builder.addConfigString("normalMapPath", "Normal Map Path", &profile.normalMapPath);
	builder.addConfigInt(
		"normalMapUnit",
		"Normal Map Unit",
		[&profile]() { return toEditableTextureUnit(profile.normalMapUnit); },
		[&profile](int value) { profile.normalMapUnit = value < 0 ? 0u : static_cast<unsigned int>(value); },
		0,
		31
	);
}

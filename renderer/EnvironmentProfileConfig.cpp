#include "EnvironmentProfileConfig.h"

#include "EnvironmentProfile.h"
#include "tools/inspector/PropertySchema.h"

using namespace GLframework;

namespace
{
	int toEditableInt(unsigned int value)
	{
		return static_cast<int>(value);
	}
}

void GLframework::buildEnvironmentProfileConfigSchema(
	GL_EDITOR::PropertyBuilder& builder,
	EnvironmentProfile& profile
)
{
	builder.addSection("Environment Source");
	builder.addConfigString("hdrEquirectangularPath", "HDR Path", &profile.hdrEquirectangularPath);
	builder.addConfigInt(
		"hdrTextureUnit",
		"HDR Texture Unit",
		[&profile]() { return toEditableInt(profile.hdrTextureUnit); },
		[&profile](int value) { profile.hdrTextureUnit = value < 0 ? 0u : static_cast<unsigned int>(value); },
		0,
		31
	);
	builder.addConfigBool("useProceduralEnvironment", "Use Procedural Environment", &profile.useProceduralEnvironment);
	builder.addConfigBool("precomputeOnPrepare", "Precompute On Prepare", &profile.precomputeOnPrepare);

	builder.addSection("Procedural Environment");
	builder.addText("Mode", "Generated at precompute time when Use Procedural Environment is enabled.");
	builder.addConfigInt(
		"proceduralWidth",
		"Procedural Width",
		[&profile]() { return toEditableInt(profile.proceduralWidth); },
		[&profile](int value) { profile.proceduralWidth = value < 0 ? 0u : static_cast<unsigned int>(value); },
		64,
		2048
	);
	builder.addConfigInt(
		"proceduralHeight",
		"Procedural Height",
		[&profile]() { return toEditableInt(profile.proceduralHeight); },
		[&profile](int value) { profile.proceduralHeight = value < 0 ? 0u : static_cast<unsigned int>(value); },
		32,
		1024
	);
	builder.addConfigFloat("proceduralSkyIntensity", "Procedural Sky Intensity", &profile.proceduralSkyIntensity, 0.0f, 10.0f);
	builder.addConfigFloat("proceduralGroundIntensity", "Procedural Ground Intensity", &profile.proceduralGroundIntensity, 0.0f, 2.0f);
	builder.addConfigFloat("proceduralSunIntensity", "Procedural Sun Intensity", &profile.proceduralSunIntensity, 0.0f, 20.0f);
}

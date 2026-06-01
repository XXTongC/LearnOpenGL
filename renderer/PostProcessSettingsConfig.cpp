#include "PostProcessSettingsConfig.h"

#include "PostProcessSettings.h"
#include "tools/inspector/PropertySchema.h"

using namespace GLframework;

namespace
{
	ToneMappingMode toneMappingModeFromInt(int value)
	{
		return value == 1 ? ToneMappingMode::Reinhard : ToneMappingMode::Exposure;
	}

	int toneMappingModeToInt(ToneMappingMode mode)
	{
		return mode == ToneMappingMode::Reinhard ? 1 : 0;
	}
}

void GLframework::buildPostProcessSettingsConfigSchema(
	GL_EDITOR::PropertyBuilder& builder,
	PostProcessSettings& settings
)
{
	builder.addSection("HDR Tone Mapping");
	builder.addConfigFloat("exposure", "Exposure", &settings.exposure, 0.0f, 4.0f);
	builder.addConfigInt(
		"toneMappingMode",
		"Tone Mapping Mode",
		[&settings]() { return toneMappingModeToInt(settings.toneMappingMode); },
		[&settings](int value) { settings.toneMappingMode = toneMappingModeFromInt(value); },
		0,
		1
	);
	builder.addText("Tone Mapping Modes", "0 = Exposure, 1 = Reinhard");

	builder.addSection("Bloom");
	builder.addConfigBool("bloomEnabled", "Bloom Enabled", &settings.bloomEnabled);
	builder.addConfigFloat("bloomThreshold", "Bloom Threshold", &settings.bloomThreshold, 0.0f, 20.0f);
	builder.addConfigFloat("bloomIntensity", "Bloom Intensity", &settings.bloomIntensity, 0.0f, 2.0f);
	builder.addConfigInt("bloomIterations", "Bloom Iterations", &settings.bloomIterations, 0, 20);
}

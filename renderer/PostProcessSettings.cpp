#include "PostProcessSettings.h"

#include <string>

#include "tools/config/ProfileConfigIO.h"
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

std::string PostProcessSettingsStorage::defaultPath()
{
	return "config/postprocess_settings.local.ini";
}

void PostProcessSettings::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	builder.addSection("HDR Tone Mapping");
	builder.addConfigFloat("exposure", "Exposure", &exposure, 0.0f, 4.0f);
	builder.addConfigInt(
		"toneMappingMode",
		"Tone Mapping Mode",
		[this]() { return toneMappingModeToInt(toneMappingMode); },
		[this](int value) { toneMappingMode = toneMappingModeFromInt(value); },
		0,
		1
	);
	builder.addText("Tone Mapping Modes", "0 = Exposure, 1 = Reinhard");

	builder.addSection("Bloom");
	builder.addConfigBool("bloomEnabled", "Bloom Enabled", &bloomEnabled);
	builder.addConfigFloat("bloomThreshold", "Bloom Threshold", &bloomThreshold, 0.0f, 20.0f);
	builder.addConfigFloat("bloomIntensity", "Bloom Intensity", &bloomIntensity, 0.0f, 2.0f);
	builder.addConfigInt("bloomIterations", "Bloom Iterations", &bloomIterations, 0, 20);
}

bool PostProcessSettingsStorage::loadFromFile(const std::string& path, PostProcessSettings& settings)
{
	PostProcessSettings loadedSettings = settings;
	GL_EDITOR::PropertyBuilder builder{};
	loadedSettings.visitEditableProperties(builder);
	const bool loaded = GL_CONFIG::loadPropertyConfig(path, builder);
	if (!loaded)
	{
		return false;
	}

	settings = loadedSettings;
	return true;
}

bool PostProcessSettingsStorage::saveToFile(const std::string& path, const PostProcessSettings& settings)
{
	PostProcessSettings snapshot = settings;
	GL_EDITOR::PropertyBuilder builder{};
	snapshot.visitEditableProperties(builder);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local postprocess settings for HDR / Bloom experiments",
		builder
	);
}

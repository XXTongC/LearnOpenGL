#include "PostProcessSettings.h"

#include <string>

#include "PostProcessSettingsConfig.h"
#include "tools/config/ProfileConfigIO.h"

using namespace GLframework;

std::string PostProcessSettingsStorage::defaultPath()
{
	return "config/postprocess_settings.local.ini";
}

bool PostProcessSettingsStorage::loadFromFile(const std::string& path, PostProcessSettings& settings)
{
	PostProcessSettings loadedSettings = settings;
	GL_EDITOR::PropertyBuilder builder{};
	buildPostProcessSettingsConfigSchema(builder, loadedSettings);
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
	buildPostProcessSettingsConfigSchema(builder, snapshot);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local postprocess settings for HDR / Bloom experiments",
		builder
	);
}

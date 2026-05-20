#include "PBRExperimentProfile.h"

#include "../config/ProfileConfigIO.h"
#include "../config/ProfileConfigParser.h"

namespace
{
	bool applyPrefixedPropertyConfigValue(
		const std::string& key,
		const std::string& value,
		const char* prefix,
		const GL_EDITOR::PropertyBuilder& builder
	)
	{
		if (!GL_CONFIG::startsWith(key, prefix))
		{
			return false;
		}

		return GL_CONFIG::applyPropertyConfigValue(
			key.substr(std::char_traits<char>::length(prefix)),
			value,
			builder
		);
	}
}

std::string GL_SCENE::PBRExperimentProfileStorage::defaultPath()
{
	return "config/pbr_experiment.local.ini";
}

bool GL_SCENE::PBRExperimentProfileStorage::loadFromFile(
	const std::string& path,
	GLframework::EnvironmentProfile& environmentProfile,
	GLframework::PostProcessSettings& postProcessSettings,
	PBRPreviewProfile& pbrPreviewProfile
)
{
	bool enabled{ true };
	auto loadedEnvironmentProfile = environmentProfile;
	auto loadedPostProcessSettings = postProcessSettings;
	auto loadedPBRPreviewProfile = pbrPreviewProfile;

	GL_EDITOR::PropertyBuilder environmentBuilder{};
	loadedEnvironmentProfile.visitEditableProperties(environmentBuilder);
	GL_EDITOR::PropertyBuilder postProcessBuilder{};
	loadedPostProcessSettings.visitEditableProperties(postProcessBuilder);
	GL_EDITOR::PropertyBuilder pbrPreviewBuilder{};
	loadedPBRPreviewProfile.visitEditableProperties(pbrPreviewBuilder);

	const bool loaded = GL_CONFIG::readKeyValueFile(path, [
		&enabled,
		&environmentBuilder,
		&postProcessBuilder,
		&pbrPreviewBuilder
	](
		const std::string& key,
		const std::string& value
	)
	{
		if (key == "enabled")
		{
			GL_CONFIG::parseBool(value, enabled);
			return;
		}

		constexpr auto environmentPrefix = "environment.";
		constexpr auto postProcessPrefix = "postprocess.";
		constexpr auto pbrPreviewPrefix = "pbrPreview.";

		if (GL_CONFIG::startsWith(key, environmentPrefix))
		{
			applyPrefixedPropertyConfigValue(key, value, environmentPrefix, environmentBuilder);
			return;
		}

		if (GL_CONFIG::startsWith(key, postProcessPrefix))
		{
			applyPrefixedPropertyConfigValue(key, value, postProcessPrefix, postProcessBuilder);
			return;
		}

		if (GL_CONFIG::startsWith(key, pbrPreviewPrefix))
		{
			applyPrefixedPropertyConfigValue(key, value, pbrPreviewPrefix, pbrPreviewBuilder);
		}
	});
	if (!loaded)
	{
		return false;
	}

	if (!enabled)
	{
		return true;
	}

	environmentProfile = loadedEnvironmentProfile;
	postProcessSettings = loadedPostProcessSettings;
	pbrPreviewProfile = loadedPBRPreviewProfile;
	return true;
}

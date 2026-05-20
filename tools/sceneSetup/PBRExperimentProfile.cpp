#include "PBRExperimentProfile.h"

#include <filesystem>
#include <fstream>

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
	PBRPreviewProfile& pbrPreviewProfile,
	PBRLightRigProfile& lightRigProfile
)
{
	bool enabled{ true };
	auto loadedEnvironmentProfile = environmentProfile;
	auto loadedPostProcessSettings = postProcessSettings;
	auto loadedPBRPreviewProfile = pbrPreviewProfile;
	auto loadedLightRigProfile = lightRigProfile;

	GL_EDITOR::PropertyBuilder environmentBuilder{};
	loadedEnvironmentProfile.visitEditableProperties(environmentBuilder);
	GL_EDITOR::PropertyBuilder postProcessBuilder{};
	loadedPostProcessSettings.visitEditableProperties(postProcessBuilder);
	GL_EDITOR::PropertyBuilder pbrPreviewBuilder{};
	loadedPBRPreviewProfile.visitEditableProperties(pbrPreviewBuilder);
	GL_EDITOR::PropertyBuilder lightRigBuilder{};
	loadedLightRigProfile.visitEditableProperties(lightRigBuilder);

	const bool loaded = GL_CONFIG::readKeyValueFile(path, [
		&enabled,
		&environmentBuilder,
		&postProcessBuilder,
		&pbrPreviewBuilder,
		&lightRigBuilder
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
		constexpr auto lightRigPrefix = "lightRig.";

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
			return;
		}

		if (GL_CONFIG::startsWith(key, lightRigPrefix))
		{
			applyPrefixedPropertyConfigValue(key, value, lightRigPrefix, lightRigBuilder);
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
	PBRPreviewProfileStorage::applyMaterialProfileReference(loadedPBRPreviewProfile);
	pbrPreviewProfile = loadedPBRPreviewProfile;
	lightRigProfile = loadedLightRigProfile;
	return true;
}

bool GL_SCENE::PBRExperimentProfileStorage::saveToFile(
	const std::string& path,
	const GLframework::EnvironmentProfile& environmentProfile,
	const GLframework::PostProcessSettings& postProcessSettings,
	const PBRPreviewProfile& pbrPreviewProfile,
	const PBRLightRigProfile& lightRigProfile
)
{
	const std::filesystem::path filePath{ path };
	const auto parentPath = filePath.parent_path();
	if (!parentPath.empty())
	{
		std::error_code error{};
		std::filesystem::create_directories(parentPath, error);
		if (error)
		{
			return false;
		}
	}

	std::ofstream output(path, std::ios::trunc);
	if (!output)
	{
		return false;
	}

	auto environmentSnapshot = environmentProfile;
	auto postProcessSnapshot = postProcessSettings;
	auto pbrPreviewSnapshot = pbrPreviewProfile;
	auto lightRigSnapshot = lightRigProfile;

	GL_EDITOR::PropertyBuilder environmentBuilder{};
	environmentSnapshot.visitEditableProperties(environmentBuilder);
	GL_EDITOR::PropertyBuilder postProcessBuilder{};
	postProcessSnapshot.visitEditableProperties(postProcessBuilder);
	GL_EDITOR::PropertyBuilder pbrPreviewBuilder{};
	pbrPreviewSnapshot.visitEditableProperties(pbrPreviewBuilder);
	GL_EDITOR::PropertyBuilder lightRigBuilder{};
	lightRigSnapshot.visitEditableProperties(lightRigBuilder);

	output
		<< "# Local PBR experiment preset for environment, postprocess, PBR preview, and light rig\n"
		<< "enabled=1\n\n";
	GL_CONFIG::writePropertyConfig(output, "environment.", environmentBuilder);
	output << '\n';
	GL_CONFIG::writePropertyConfig(output, "postprocess.", postProcessBuilder);
	output << '\n';
	GL_CONFIG::writePropertyConfig(output, "pbrPreview.", pbrPreviewBuilder);
	output << '\n';
	GL_CONFIG::writePropertyConfig(output, "lightRig.", lightRigBuilder);

	return true;
}

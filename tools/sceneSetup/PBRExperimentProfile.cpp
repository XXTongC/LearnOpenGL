#include "PBRExperimentProfile.h"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace
{
	std::string trim(std::string value)
	{
		auto isSpace = [](unsigned char ch)
		{
			return std::isspace(ch) != 0;
		};

		value.erase(value.begin(), std::find_if(value.begin(), value.end(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}));
		value.erase(std::find_if(value.rbegin(), value.rend(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}).base(), value.end());
		return value;
	}

	bool startsWith(const std::string& value, const std::string& prefix)
	{
		return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
	}

	bool parseFloat(const std::string& value, float& output)
	{
		try
		{
			size_t parsedCharacters{ 0 };
			const auto parsed = std::stof(value, &parsedCharacters);
			if (parsedCharacters != value.size())
			{
				return false;
			}

			output = parsed;
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool parseInt(const std::string& value, int& output)
	{
		try
		{
			size_t parsedCharacters{ 0 };
			const auto parsed = std::stoi(value, &parsedCharacters);
			if (parsedCharacters != value.size())
			{
				return false;
			}

			output = parsed;
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool parseUnsigned(const std::string& value, unsigned int& output)
	{
		try
		{
			size_t parsedCharacters{ 0 };
			const auto parsed = std::stoul(value, &parsedCharacters);
			if (parsedCharacters != value.size())
			{
				return false;
			}

			output = static_cast<unsigned int>(parsed);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool parseBool(std::string value, bool& output)
	{
		std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
		{
			return static_cast<char>(std::tolower(ch));
		});

		if (value == "1" || value == "true" || value == "yes" || value == "on")
		{
			output = true;
			return true;
		}

		if (value == "0" || value == "false" || value == "no" || value == "off")
		{
			output = false;
			return true;
		}

		return false;
	}

	bool applyEnvironmentKey(const std::string& key, const std::string& value, GLframework::EnvironmentProfile& profile)
	{
		if (key == "hdrEquirectangularPath")
		{
			profile.hdrEquirectangularPath = value;
			return true;
		}

		if (key == "hdrTextureUnit")
		{
			return parseUnsigned(value, profile.hdrTextureUnit);
		}

		if (key == "precomputeOnPrepare")
		{
			return parseBool(value, profile.precomputeOnPrepare);
		}

		if (key == "useProceduralEnvironment")
		{
			return parseBool(value, profile.useProceduralEnvironment);
		}

		if (key == "proceduralWidth")
		{
			return parseUnsigned(value, profile.proceduralWidth);
		}

		if (key == "proceduralHeight")
		{
			return parseUnsigned(value, profile.proceduralHeight);
		}

		if (key == "proceduralSkyIntensity")
		{
			return parseFloat(value, profile.proceduralSkyIntensity);
		}

		if (key == "proceduralGroundIntensity")
		{
			return parseFloat(value, profile.proceduralGroundIntensity);
		}

		if (key == "proceduralSunIntensity")
		{
			return parseFloat(value, profile.proceduralSunIntensity);
		}

		return false;
	}

	bool applyPostProcessKey(const std::string& key, const std::string& value, GLframework::PostProcessSettings& settings)
	{
		if (key == "exposure")
		{
			return parseFloat(value, settings.exposure);
		}

		if (key == "toneMappingMode")
		{
			int parsedMode{ static_cast<int>(settings.toneMappingMode) };
			if (!parseInt(value, parsedMode))
			{
				return false;
			}

			settings.toneMappingMode = parsedMode == 1
				? GLframework::ToneMappingMode::Reinhard
				: GLframework::ToneMappingMode::Exposure;
			return true;
		}

		if (key == "bloomEnabled")
		{
			return parseBool(value, settings.bloomEnabled);
		}

		if (key == "bloomThreshold")
		{
			return parseFloat(value, settings.bloomThreshold);
		}

		if (key == "bloomIntensity")
		{
			return parseFloat(value, settings.bloomIntensity);
		}

		if (key == "bloomIterations")
		{
			return parseInt(value, settings.bloomIterations);
		}

		return false;
	}

	bool applyPBRPreviewKey(const std::string& key, const std::string& value, GL_SCENE::PBRPreviewProfile& profile)
	{
		if (key == "enabled")
		{
			return parseBool(value, profile.enabled);
		}

		if (key == "positionX")
		{
			return parseFloat(value, profile.position.x);
		}

		if (key == "positionY")
		{
			return parseFloat(value, profile.position.y);
		}

		if (key == "positionZ")
		{
			return parseFloat(value, profile.position.z);
		}

		if (key == "radius")
		{
			return parseFloat(value, profile.radius);
		}

		if (key == "segments")
		{
			return parseInt(value, profile.segments);
		}

		if (key == "rings")
		{
			return parseInt(value, profile.rings);
		}

		if (key == "useMaterialGrid")
		{
			return parseBool(value, profile.useMaterialGrid);
		}

		if (key == "gridColumns")
		{
			return parseInt(value, profile.gridColumns);
		}

		if (key == "gridRows")
		{
			return parseInt(value, profile.gridRows);
		}

		if (key == "gridSpacing")
		{
			return parseFloat(value, profile.gridSpacing);
		}

		if (key == "gridRadius")
		{
			return parseFloat(value, profile.gridRadius);
		}

		if (key == "gridMetallicMin")
		{
			return parseFloat(value, profile.gridMetallicMin);
		}

		if (key == "gridMetallicMax")
		{
			return parseFloat(value, profile.gridMetallicMax);
		}

		if (key == "gridRoughnessMin")
		{
			return parseFloat(value, profile.gridRoughnessMin);
		}

		if (key == "gridRoughnessMax")
		{
			return parseFloat(value, profile.gridRoughnessMax);
		}

		if (key == "albedoR")
		{
			return parseFloat(value, profile.albedo.r);
		}

		if (key == "albedoG")
		{
			return parseFloat(value, profile.albedo.g);
		}

		if (key == "albedoB")
		{
			return parseFloat(value, profile.albedo.b);
		}

		if (key == "metallic")
		{
			return parseFloat(value, profile.metallic);
		}

		if (key == "roughness")
		{
			return parseFloat(value, profile.roughness);
		}

		if (key == "ao")
		{
			return parseFloat(value, profile.ao);
		}

		if (key == "useIBL")
		{
			return parseBool(value, profile.useIBL);
		}

		if (key == "iblDiffuseStrength")
		{
			return parseFloat(value, profile.iblDiffuseStrength);
		}

		if (key == "iblSpecularStrength")
		{
			return parseFloat(value, profile.iblSpecularStrength);
		}

		if (key == "normalMapPath")
		{
			profile.normalMapPath = value;
			return true;
		}

		if (key == "normalMapUnit")
		{
			return parseUnsigned(value, profile.normalMapUnit);
		}

		return false;
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
	std::ifstream input(path);
	if (!input)
	{
		return false;
	}

	bool enabled{ true };
	auto loadedEnvironmentProfile = environmentProfile;
	auto loadedPostProcessSettings = postProcessSettings;
	auto loadedPBRPreviewProfile = pbrPreviewProfile;

	std::string line{};
	while (std::getline(input, line))
	{
		line = trim(line);
		if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[')
		{
			continue;
		}

		const auto separator = line.find('=');
		if (separator == std::string::npos)
		{
			continue;
		}

		const auto key = trim(line.substr(0, separator));
		const auto value = trim(line.substr(separator + 1));
		if (key == "enabled")
		{
			parseBool(value, enabled);
			continue;
		}

		constexpr auto environmentPrefix = "environment.";
		constexpr auto postProcessPrefix = "postprocess.";
		constexpr auto pbrPreviewPrefix = "pbrPreview.";

		if (startsWith(key, environmentPrefix))
		{
			applyEnvironmentKey(key.substr(std::char_traits<char>::length(environmentPrefix)), value, loadedEnvironmentProfile);
			continue;
		}

		if (startsWith(key, postProcessPrefix))
		{
			applyPostProcessKey(key.substr(std::char_traits<char>::length(postProcessPrefix)), value, loadedPostProcessSettings);
			continue;
		}

		if (startsWith(key, pbrPreviewPrefix))
		{
			applyPBRPreviewKey(key.substr(std::char_traits<char>::length(pbrPreviewPrefix)), value, loadedPBRPreviewProfile);
		}
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

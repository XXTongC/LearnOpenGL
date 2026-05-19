#include "PostProcessSettings.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>

using namespace GLframework;

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

	ToneMappingMode toneMappingModeFromInt(int value)
	{
		return value == 1 ? ToneMappingMode::Reinhard : ToneMappingMode::Exposure;
	}
}

std::string PostProcessSettingsStorage::defaultPath()
{
	return "config/postprocess_settings.local.ini";
}

bool PostProcessSettingsStorage::loadFromFile(const std::string& path, PostProcessSettings& settings)
{
	std::ifstream input(path);
	if (!input)
	{
		return false;
	}

	PostProcessSettings loadedSettings = settings;
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
		if (key == "exposure")
		{
			float parsedExposure{ loadedSettings.exposure };
			if (parseFloat(value, parsedExposure))
			{
				loadedSettings.exposure = parsedExposure;
			}
			continue;
		}

		if (key == "toneMappingMode")
		{
			int parsedMode{ static_cast<int>(loadedSettings.toneMappingMode) };
			if (parseInt(value, parsedMode))
			{
				loadedSettings.toneMappingMode = toneMappingModeFromInt(parsedMode);
			}
			continue;
		}

		if (key == "bloomEnabled")
		{
			bool parsedBloomEnabled{ loadedSettings.bloomEnabled };
			if (parseBool(value, parsedBloomEnabled))
			{
				loadedSettings.bloomEnabled = parsedBloomEnabled;
			}
			continue;
		}

		if (key == "bloomThreshold")
		{
			float parsedBloomThreshold{ loadedSettings.bloomThreshold };
			if (parseFloat(value, parsedBloomThreshold))
			{
				loadedSettings.bloomThreshold = parsedBloomThreshold;
			}
			continue;
		}

		if (key == "bloomIntensity")
		{
			float parsedBloomIntensity{ loadedSettings.bloomIntensity };
			if (parseFloat(value, parsedBloomIntensity))
			{
				loadedSettings.bloomIntensity = parsedBloomIntensity;
			}
			continue;
		}

		if (key == "bloomIterations")
		{
			int parsedBloomIterations{ loadedSettings.bloomIterations };
			if (parseInt(value, parsedBloomIterations))
			{
				loadedSettings.bloomIterations = parsedBloomIterations;
			}
		}
	}

	settings = loadedSettings;
	return true;
}

bool PostProcessSettingsStorage::saveToFile(const std::string& path, const PostProcessSettings& settings)
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

	output << "# Local postprocess settings for HDR / Bloom experiments\n";
	output << "exposure=" << settings.exposure << '\n';
	output << "toneMappingMode=" << static_cast<int>(settings.toneMappingMode) << '\n';
	output << "bloomEnabled=" << (settings.bloomEnabled ? 1 : 0) << '\n';
	output << "bloomThreshold=" << settings.bloomThreshold << '\n';
	output << "bloomIntensity=" << settings.bloomIntensity << '\n';
	output << "bloomIterations=" << settings.bloomIterations << '\n';
	return true;
}

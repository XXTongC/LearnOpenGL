#include "PostProcessSettings.h"

#include <filesystem>
#include <fstream>
#include <string>

#include "tools/config/ProfileConfigParser.h"
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
	builder.addFloat("Exposure", &exposure, 0.0f, 4.0f);
	builder.addInt(
		"Tone Mapping Mode",
		[this]() { return toneMappingModeToInt(toneMappingMode); },
		[this](int value) { toneMappingMode = toneMappingModeFromInt(value); },
		0,
		1
	);
	builder.addText("Tone Mapping Modes", "0 = Exposure, 1 = Reinhard");

	builder.addSection("Bloom");
	builder.addBool("Bloom Enabled", &bloomEnabled);
	builder.addFloat("Bloom Threshold", &bloomThreshold, 0.0f, 20.0f);
	builder.addFloat("Bloom Intensity", &bloomIntensity, 0.0f, 2.0f);
	builder.addInt("Bloom Iterations", &bloomIterations, 0, 20);
}

bool PostProcessSettingsStorage::loadFromFile(const std::string& path, PostProcessSettings& settings)
{
	PostProcessSettings loadedSettings = settings;
	const bool loaded = GL_CONFIG::readKeyValueFile(path, [&loadedSettings](const std::string& key, const std::string& value)
	{
		if (key == "exposure")
		{
			float parsedExposure{ loadedSettings.exposure };
			if (GL_CONFIG::parseFloat(value, parsedExposure))
			{
				loadedSettings.exposure = parsedExposure;
			}
			return;
		}

		if (key == "toneMappingMode")
		{
			int parsedMode{ static_cast<int>(loadedSettings.toneMappingMode) };
			if (GL_CONFIG::parseInt(value, parsedMode))
			{
				loadedSettings.toneMappingMode = toneMappingModeFromInt(parsedMode);
			}
			return;
		}

		if (key == "bloomEnabled")
		{
			bool parsedBloomEnabled{ loadedSettings.bloomEnabled };
			if (GL_CONFIG::parseBool(value, parsedBloomEnabled))
			{
				loadedSettings.bloomEnabled = parsedBloomEnabled;
			}
			return;
		}

		if (key == "bloomThreshold")
		{
			float parsedBloomThreshold{ loadedSettings.bloomThreshold };
			if (GL_CONFIG::parseFloat(value, parsedBloomThreshold))
			{
				loadedSettings.bloomThreshold = parsedBloomThreshold;
			}
			return;
		}

		if (key == "bloomIntensity")
		{
			float parsedBloomIntensity{ loadedSettings.bloomIntensity };
			if (GL_CONFIG::parseFloat(value, parsedBloomIntensity))
			{
				loadedSettings.bloomIntensity = parsedBloomIntensity;
			}
			return;
		}

		if (key == "bloomIterations")
		{
			int parsedBloomIterations{ loadedSettings.bloomIterations };
			if (GL_CONFIG::parseInt(value, parsedBloomIterations))
			{
				loadedSettings.bloomIterations = parsedBloomIterations;
			}
		}
	});
	if (!loaded)
	{
		return false;
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

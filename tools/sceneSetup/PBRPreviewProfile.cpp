#include "PBRPreviewProfile.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
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
}

std::string GL_SCENE::PBRPreviewProfileStorage::defaultPath()
{
	return "config/pbr_preview.local.ini";
}

bool GL_SCENE::PBRPreviewProfileStorage::loadFromFile(const std::string& path, PBRPreviewProfile& profile)
{
	std::ifstream input(path);
	if (!input)
	{
		return false;
	}

	PBRPreviewProfile loadedProfile = profile;
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
			bool parsedEnabled{ loadedProfile.enabled };
			if (parseBool(value, parsedEnabled))
			{
				loadedProfile.enabled = parsedEnabled;
			}
			continue;
		}

		if (key == "positionX")
		{
			parseFloat(value, loadedProfile.position.x);
			continue;
		}

		if (key == "positionY")
		{
			parseFloat(value, loadedProfile.position.y);
			continue;
		}

		if (key == "positionZ")
		{
			parseFloat(value, loadedProfile.position.z);
			continue;
		}

		if (key == "radius")
		{
			parseFloat(value, loadedProfile.radius);
			continue;
		}

		if (key == "segments")
		{
			parseInt(value, loadedProfile.segments);
			continue;
		}

		if (key == "rings")
		{
			parseInt(value, loadedProfile.rings);
			continue;
		}

		if (key == "useMaterialGrid")
		{
			bool parsedUseMaterialGrid{ loadedProfile.useMaterialGrid };
			if (parseBool(value, parsedUseMaterialGrid))
			{
				loadedProfile.useMaterialGrid = parsedUseMaterialGrid;
			}
			continue;
		}

		if (key == "gridColumns")
		{
			parseInt(value, loadedProfile.gridColumns);
			continue;
		}

		if (key == "gridRows")
		{
			parseInt(value, loadedProfile.gridRows);
			continue;
		}

		if (key == "gridSpacing")
		{
			parseFloat(value, loadedProfile.gridSpacing);
			continue;
		}

		if (key == "gridRadius")
		{
			parseFloat(value, loadedProfile.gridRadius);
			continue;
		}

		if (key == "gridMetallicMin")
		{
			parseFloat(value, loadedProfile.gridMetallicMin);
			continue;
		}

		if (key == "gridMetallicMax")
		{
			parseFloat(value, loadedProfile.gridMetallicMax);
			continue;
		}

		if (key == "gridRoughnessMin")
		{
			parseFloat(value, loadedProfile.gridRoughnessMin);
			continue;
		}

		if (key == "gridRoughnessMax")
		{
			parseFloat(value, loadedProfile.gridRoughnessMax);
			continue;
		}

		if (key == "albedoR")
		{
			parseFloat(value, loadedProfile.albedo.r);
			continue;
		}

		if (key == "albedoG")
		{
			parseFloat(value, loadedProfile.albedo.g);
			continue;
		}

		if (key == "albedoB")
		{
			parseFloat(value, loadedProfile.albedo.b);
			continue;
		}

		if (key == "metallic")
		{
			parseFloat(value, loadedProfile.metallic);
			continue;
		}

		if (key == "roughness")
		{
			parseFloat(value, loadedProfile.roughness);
			continue;
		}

		if (key == "ao")
		{
			parseFloat(value, loadedProfile.ao);
			continue;
		}

		if (key == "useIBL")
		{
			bool parsedUseIBL{ loadedProfile.useIBL };
			if (parseBool(value, parsedUseIBL))
			{
				loadedProfile.useIBL = parsedUseIBL;
			}
			continue;
		}

		if (key == "iblDiffuseStrength")
		{
			parseFloat(value, loadedProfile.iblDiffuseStrength);
			continue;
		}

		if (key == "iblSpecularStrength")
		{
			parseFloat(value, loadedProfile.iblSpecularStrength);
			continue;
		}

		if (key == "normalMapPath")
		{
			loadedProfile.normalMapPath = value;
			continue;
		}

		if (key == "normalMapUnit")
		{
			parseUnsigned(value, loadedProfile.normalMapUnit);
		}
	}

	profile = loadedProfile;
	return true;
}

bool GL_SCENE::PBRPreviewProfileStorage::saveToFile(const std::string& path, const PBRPreviewProfile& profile)
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

	output << "# Local PBR preview profile for material and IBL experiments\n";
	output << "enabled=" << (profile.enabled ? 1 : 0) << '\n';
	output << "positionX=" << profile.position.x << '\n';
	output << "positionY=" << profile.position.y << '\n';
	output << "positionZ=" << profile.position.z << '\n';
	output << "radius=" << profile.radius << '\n';
	output << "segments=" << profile.segments << '\n';
	output << "rings=" << profile.rings << '\n';
	output << "useMaterialGrid=" << (profile.useMaterialGrid ? 1 : 0) << '\n';
	output << "gridColumns=" << profile.gridColumns << '\n';
	output << "gridRows=" << profile.gridRows << '\n';
	output << "gridSpacing=" << profile.gridSpacing << '\n';
	output << "gridRadius=" << profile.gridRadius << '\n';
	output << "gridMetallicMin=" << profile.gridMetallicMin << '\n';
	output << "gridMetallicMax=" << profile.gridMetallicMax << '\n';
	output << "gridRoughnessMin=" << profile.gridRoughnessMin << '\n';
	output << "gridRoughnessMax=" << profile.gridRoughnessMax << '\n';
	output << "albedoR=" << profile.albedo.r << '\n';
	output << "albedoG=" << profile.albedo.g << '\n';
	output << "albedoB=" << profile.albedo.b << '\n';
	output << "metallic=" << profile.metallic << '\n';
	output << "roughness=" << profile.roughness << '\n';
	output << "ao=" << profile.ao << '\n';
	output << "useIBL=" << (profile.useIBL ? 1 : 0) << '\n';
	output << "iblDiffuseStrength=" << profile.iblDiffuseStrength << '\n';
	output << "iblSpecularStrength=" << profile.iblSpecularStrength << '\n';
	output << "normalMapPath=" << profile.normalMapPath << '\n';
	output << "normalMapUnit=" << profile.normalMapUnit << '\n';
	return true;
}

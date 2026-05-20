#include "PBRPreviewProfile.h"

#include <filesystem>
#include <fstream>

#include "../config/ProfileConfigParser.h"

std::string GL_SCENE::PBRPreviewProfileStorage::defaultPath()
{
	return "config/pbr_preview.local.ini";
}

bool GL_SCENE::PBRPreviewProfileStorage::loadFromFile(const std::string& path, PBRPreviewProfile& profile)
{
	PBRPreviewProfile loadedProfile = profile;
	const bool loaded = GL_CONFIG::readKeyValueFile(path, [&loadedProfile](const std::string& key, const std::string& value)
	{
		if (key == "enabled")
		{
			bool parsedEnabled{ loadedProfile.enabled };
			if (GL_CONFIG::parseBool(value, parsedEnabled))
			{
				loadedProfile.enabled = parsedEnabled;
			}
			return;
		}

		if (key == "positionX")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.position.x);
			return;
		}

		if (key == "positionY")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.position.y);
			return;
		}

		if (key == "positionZ")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.position.z);
			return;
		}

		if (key == "radius")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.radius);
			return;
		}

		if (key == "segments")
		{
			GL_CONFIG::parseInt(value, loadedProfile.segments);
			return;
		}

		if (key == "rings")
		{
			GL_CONFIG::parseInt(value, loadedProfile.rings);
			return;
		}

		if (key == "useMaterialGrid")
		{
			bool parsedUseMaterialGrid{ loadedProfile.useMaterialGrid };
			if (GL_CONFIG::parseBool(value, parsedUseMaterialGrid))
			{
				loadedProfile.useMaterialGrid = parsedUseMaterialGrid;
			}
			return;
		}

		if (key == "gridColumns")
		{
			GL_CONFIG::parseInt(value, loadedProfile.gridColumns);
			return;
		}

		if (key == "gridRows")
		{
			GL_CONFIG::parseInt(value, loadedProfile.gridRows);
			return;
		}

		if (key == "gridSpacing")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.gridSpacing);
			return;
		}

		if (key == "gridRadius")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.gridRadius);
			return;
		}

		if (key == "gridMetallicMin")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.gridMetallicMin);
			return;
		}

		if (key == "gridMetallicMax")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.gridMetallicMax);
			return;
		}

		if (key == "gridRoughnessMin")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.gridRoughnessMin);
			return;
		}

		if (key == "gridRoughnessMax")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.gridRoughnessMax);
			return;
		}

		if (key == "albedoR")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.albedo.r);
			return;
		}

		if (key == "albedoG")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.albedo.g);
			return;
		}

		if (key == "albedoB")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.albedo.b);
			return;
		}

		if (key == "metallic")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.metallic);
			return;
		}

		if (key == "roughness")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.roughness);
			return;
		}

		if (key == "ao")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.ao);
			return;
		}

		if (key == "useIBL")
		{
			bool parsedUseIBL{ loadedProfile.useIBL };
			if (GL_CONFIG::parseBool(value, parsedUseIBL))
			{
				loadedProfile.useIBL = parsedUseIBL;
			}
			return;
		}

		if (key == "iblDiffuseStrength")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.iblDiffuseStrength);
			return;
		}

		if (key == "iblSpecularStrength")
		{
			GL_CONFIG::parseFloat(value, loadedProfile.iblSpecularStrength);
			return;
		}

		if (key == "normalMapPath")
		{
			loadedProfile.normalMapPath = value;
			return;
		}

		if (key == "normalMapUnit")
		{
			GL_CONFIG::parseUnsigned(value, loadedProfile.normalMapUnit);
		}
	});
	if (!loaded)
	{
		return false;
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

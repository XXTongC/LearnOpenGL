#include "PBRExperimentProfile.h"

#include "../config/ProfileConfigParser.h"

namespace
{
	bool applyEnvironmentKey(const std::string& key, const std::string& value, GLframework::EnvironmentProfile& profile)
	{
		if (key == "hdrEquirectangularPath")
		{
			profile.hdrEquirectangularPath = value;
			return true;
		}

		if (key == "hdrTextureUnit")
		{
			return GL_CONFIG::parseUnsigned(value, profile.hdrTextureUnit);
		}

		if (key == "precomputeOnPrepare")
		{
			return GL_CONFIG::parseBool(value, profile.precomputeOnPrepare);
		}

		if (key == "useProceduralEnvironment")
		{
			return GL_CONFIG::parseBool(value, profile.useProceduralEnvironment);
		}

		if (key == "proceduralWidth")
		{
			return GL_CONFIG::parseUnsigned(value, profile.proceduralWidth);
		}

		if (key == "proceduralHeight")
		{
			return GL_CONFIG::parseUnsigned(value, profile.proceduralHeight);
		}

		if (key == "proceduralSkyIntensity")
		{
			return GL_CONFIG::parseFloat(value, profile.proceduralSkyIntensity);
		}

		if (key == "proceduralGroundIntensity")
		{
			return GL_CONFIG::parseFloat(value, profile.proceduralGroundIntensity);
		}

		if (key == "proceduralSunIntensity")
		{
			return GL_CONFIG::parseFloat(value, profile.proceduralSunIntensity);
		}

		return false;
	}

	bool applyPostProcessKey(const std::string& key, const std::string& value, GLframework::PostProcessSettings& settings)
	{
		if (key == "exposure")
		{
			return GL_CONFIG::parseFloat(value, settings.exposure);
		}

		if (key == "toneMappingMode")
		{
			int parsedMode{ static_cast<int>(settings.toneMappingMode) };
			if (!GL_CONFIG::parseInt(value, parsedMode))
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
			return GL_CONFIG::parseBool(value, settings.bloomEnabled);
		}

		if (key == "bloomThreshold")
		{
			return GL_CONFIG::parseFloat(value, settings.bloomThreshold);
		}

		if (key == "bloomIntensity")
		{
			return GL_CONFIG::parseFloat(value, settings.bloomIntensity);
		}

		if (key == "bloomIterations")
		{
			return GL_CONFIG::parseInt(value, settings.bloomIterations);
		}

		return false;
	}

	bool applyPBRPreviewKey(const std::string& key, const std::string& value, GL_SCENE::PBRPreviewProfile& profile)
	{
		if (key == "enabled")
		{
			return GL_CONFIG::parseBool(value, profile.enabled);
		}

		if (key == "positionX")
		{
			return GL_CONFIG::parseFloat(value, profile.position.x);
		}

		if (key == "positionY")
		{
			return GL_CONFIG::parseFloat(value, profile.position.y);
		}

		if (key == "positionZ")
		{
			return GL_CONFIG::parseFloat(value, profile.position.z);
		}

		if (key == "radius")
		{
			return GL_CONFIG::parseFloat(value, profile.radius);
		}

		if (key == "segments")
		{
			return GL_CONFIG::parseInt(value, profile.segments);
		}

		if (key == "rings")
		{
			return GL_CONFIG::parseInt(value, profile.rings);
		}

		if (key == "useMaterialGrid")
		{
			return GL_CONFIG::parseBool(value, profile.useMaterialGrid);
		}

		if (key == "gridColumns")
		{
			return GL_CONFIG::parseInt(value, profile.gridColumns);
		}

		if (key == "gridRows")
		{
			return GL_CONFIG::parseInt(value, profile.gridRows);
		}

		if (key == "gridSpacing")
		{
			return GL_CONFIG::parseFloat(value, profile.gridSpacing);
		}

		if (key == "gridRadius")
		{
			return GL_CONFIG::parseFloat(value, profile.gridRadius);
		}

		if (key == "gridMetallicMin")
		{
			return GL_CONFIG::parseFloat(value, profile.gridMetallicMin);
		}

		if (key == "gridMetallicMax")
		{
			return GL_CONFIG::parseFloat(value, profile.gridMetallicMax);
		}

		if (key == "gridRoughnessMin")
		{
			return GL_CONFIG::parseFloat(value, profile.gridRoughnessMin);
		}

		if (key == "gridRoughnessMax")
		{
			return GL_CONFIG::parseFloat(value, profile.gridRoughnessMax);
		}

		if (key == "albedoR")
		{
			return GL_CONFIG::parseFloat(value, profile.albedo.r);
		}

		if (key == "albedoG")
		{
			return GL_CONFIG::parseFloat(value, profile.albedo.g);
		}

		if (key == "albedoB")
		{
			return GL_CONFIG::parseFloat(value, profile.albedo.b);
		}

		if (key == "metallic")
		{
			return GL_CONFIG::parseFloat(value, profile.metallic);
		}

		if (key == "roughness")
		{
			return GL_CONFIG::parseFloat(value, profile.roughness);
		}

		if (key == "ao")
		{
			return GL_CONFIG::parseFloat(value, profile.ao);
		}

		if (key == "useIBL")
		{
			return GL_CONFIG::parseBool(value, profile.useIBL);
		}

		if (key == "iblDiffuseStrength")
		{
			return GL_CONFIG::parseFloat(value, profile.iblDiffuseStrength);
		}

		if (key == "iblSpecularStrength")
		{
			return GL_CONFIG::parseFloat(value, profile.iblSpecularStrength);
		}

		if (key == "normalMapPath")
		{
			profile.normalMapPath = value;
			return true;
		}

		if (key == "normalMapUnit")
		{
			return GL_CONFIG::parseUnsigned(value, profile.normalMapUnit);
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
	bool enabled{ true };
	auto loadedEnvironmentProfile = environmentProfile;
	auto loadedPostProcessSettings = postProcessSettings;
	auto loadedPBRPreviewProfile = pbrPreviewProfile;

	const bool loaded = GL_CONFIG::readKeyValueFile(path, [&enabled, &loadedEnvironmentProfile, &loadedPostProcessSettings, &loadedPBRPreviewProfile](
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
			applyEnvironmentKey(key.substr(std::char_traits<char>::length(environmentPrefix)), value, loadedEnvironmentProfile);
			return;
		}

		if (GL_CONFIG::startsWith(key, postProcessPrefix))
		{
			applyPostProcessKey(key.substr(std::char_traits<char>::length(postProcessPrefix)), value, loadedPostProcessSettings);
			return;
		}

		if (GL_CONFIG::startsWith(key, pbrPreviewPrefix))
		{
			applyPBRPreviewKey(key.substr(std::char_traits<char>::length(pbrPreviewPrefix)), value, loadedPBRPreviewProfile);
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

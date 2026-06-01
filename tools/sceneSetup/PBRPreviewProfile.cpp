#include "PBRPreviewProfile.h"

#include "../config/ProfileConfigIO.h"
#include "PBRPreviewProfileConfig.h"

namespace
{
	bool hasMaterialProfilePath(const GL_SCENE::PBRPreviewProfile& profile)
	{
		return !profile.materialProfilePath.empty();
	}
}

GL_SCENE::PBRPreviewProfile::PBRPreviewProfile()
{
	material.albedo = { 0.9f, 0.42f, 0.18f };
	material.metallic = 0.2f;
	material.roughness = 0.35f;
	material.ao = 1.0f;
	material.useIBL = true;
	material.iblDiffuseStrength = 1.0f;
	material.iblSpecularStrength = 1.0f;
}

std::string GL_SCENE::PBRPreviewProfileStorage::defaultPath()
{
	return "config/pbr_preview.local.ini";
}

bool GL_SCENE::PBRPreviewProfileStorage::loadFromFile(const std::string& path, PBRPreviewProfile& profile)
{
	PBRPreviewProfile loadedProfile = profile;
	GL_EDITOR::PropertyBuilder builder{};
	buildPBRPreviewProfileConfigSchema(builder, loadedProfile);
	const bool loaded = GL_CONFIG::loadPropertyConfig(path, builder);
	if (!loaded)
	{
		return false;
	}

	PBRPreviewProfileStorage::applyMaterialProfileReference(loadedProfile);
	profile = loadedProfile;
	return true;
}

bool GL_SCENE::PBRPreviewProfileStorage::saveToFile(const std::string& path, const PBRPreviewProfile& profile)
{
	PBRPreviewProfile snapshot = profile;
	GL_EDITOR::PropertyBuilder builder{};
	buildPBRPreviewProfileConfigSchema(builder, snapshot);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local PBR preview profile for material and IBL experiments",
		builder
	);
}

bool GL_SCENE::PBRPreviewProfileStorage::applyMaterialProfileReference(PBRPreviewProfile& profile)
{
	if (!hasMaterialProfilePath(profile))
	{
		return false;
	}

	return GLframework::PBRMaterialProfileStorage::loadFromFile(profile.materialProfilePath, profile.material);
}

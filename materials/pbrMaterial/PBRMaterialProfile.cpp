#include "PBRMaterialProfile.h"

#include "PBRMaterial.h"
#include "PBRMaterialProfileConfig.h"
#include "tools/config/ProfileConfigIO.h"
#include "tools/inspector/PropertySchema.h"

using namespace GLframework;

void PBRMaterialProfile::applyTo(PBRMaterial& material) const
{
	material.setSurface({
		albedo,
		emissiveColor,
		metallic,
		roughness,
		ao,
		emissiveIntensity
	});
	material.setAlphaMask({ useAlphaMask, alphaCutoff });
	material.setIbl({ useIBL, iblDiffuseStrength, iblSpecularStrength });
}

void PBRMaterialProfile::copyFrom(const PBRMaterial& material)
{
	const auto surface = material.surfaceState();
	const auto alphaMask = material.alphaMaskState();
	const auto ibl = material.iblState();
	albedo = surface.albedo;
	emissiveColor = surface.emissiveColor;
	metallic = surface.metallic;
	roughness = surface.roughness;
	ao = surface.ao;
	emissiveIntensity = surface.emissiveIntensity;
	useAlphaMask = alphaMask.useAlphaMask;
	alphaCutoff = alphaMask.alphaCutoff;
	useIBL = ibl.useIbl;
	iblDiffuseStrength = ibl.diffuseStrength;
	iblSpecularStrength = ibl.specularStrength;
}

std::string PBRMaterialProfileStorage::defaultPath()
{
	return "config/pbr_material.local.ini";
}

bool PBRMaterialProfileStorage::loadFromFile(const std::string& path, PBRMaterialProfile& profile)
{
	PBRMaterialProfile loadedProfile = profile;
	GL_EDITOR::PropertyBuilder builder{};
	buildPBRMaterialProfileConfigSchema(builder, loadedProfile);
	const bool loaded = GL_CONFIG::loadPropertyConfig(path, builder);
	if (!loaded)
	{
		return false;
	}

	profile = loadedProfile;
	return true;
}

bool PBRMaterialProfileStorage::saveToFile(const std::string& path, const PBRMaterialProfile& profile)
{
	PBRMaterialProfile snapshot = profile;
	GL_EDITOR::PropertyBuilder builder{};
	buildPBRMaterialProfileConfigSchema(builder, snapshot);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local PBR material profile for material and IBL experiments",
		builder
	);
}

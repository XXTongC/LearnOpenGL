#include "PBRMaterial.h"

#include <cstddef>
#include <utility>

#include "tools/config/ProfileConfigIO.h"
#include "tools/inspector/PropertySchema.h"

using namespace GLframework;

namespace
{
	void addMaterialProfileSurfaceProperties(GL_EDITOR::PropertyBuilder& builder, PBRMaterialProfile& profile)
	{
		builder.addSection("PBR Surface");
		builder.addConfigColor3({ "albedoR", "albedoG", "albedoB" }, "Albedo", &profile.albedo);
		builder.addConfigFloat("metallic", "Metallic", &profile.metallic, 0.0f, 1.0f);
		builder.addConfigFloat("roughness", "Roughness", &profile.roughness, 0.04f, 1.0f);
		builder.addConfigFloat("ao", "AO", &profile.ao, 0.0f, 1.0f);
		builder.addConfigColor3({ "emissiveR", "emissiveG", "emissiveB" }, "Emissive Color", &profile.emissiveColor);
		builder.addConfigFloat("emissiveIntensity", "Emissive Intensity", &profile.emissiveIntensity, 0.0f, 20.0f);
		builder.addSection("PBR Alpha Mask");
		builder.addConfigBool("useAlphaMask", "Use Alpha Mask", &profile.useAlphaMask);
		builder.addConfigFloat("alphaCutoff", "Alpha Cutoff", &profile.alphaCutoff, 0.0f, 1.0f);
	}

	void addMaterialProfileIblProperties(GL_EDITOR::PropertyBuilder& builder, PBRMaterialProfile& profile)
	{
		builder.addSection("IBL");
		builder.addConfigBool("useIBL", "Use IBL", &profile.useIBL);
		builder.addConfigFloat("iblDiffuseStrength", "IBL Diffuse Strength", &profile.iblDiffuseStrength, 0.0f, 5.0f);
		builder.addConfigFloat("iblSpecularStrength", "IBL Specular Strength", &profile.iblSpecularStrength, 0.0f, 5.0f);
	}

}

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

void PBRMaterialProfile::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	addMaterialProfileSurfaceProperties(builder, *this);
	addMaterialProfileIblProperties(builder, *this);
}

std::string PBRMaterialProfileStorage::defaultPath()
{
	return "config/pbr_material.local.ini";
}

bool PBRMaterialProfileStorage::loadFromFile(const std::string& path, PBRMaterialProfile& profile)
{
	PBRMaterialProfile loadedProfile = profile;
	GL_EDITOR::PropertyBuilder builder{};
	loadedProfile.visitEditableProperties(builder);
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
	snapshot.visitEditableProperties(builder);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local PBR material profile for material and IBL experiments",
		builder
	);
}

PBRMaterial::PBRMaterial()
{
	setMaterialType(MaterialType::PBRMaterial);
}

void PBRMaterial::setSurface(PBRSurfaceInput surface)
{
	mAlbedo = surface.albedo;
	mEmissiveColor = surface.emissiveColor;
	mMetallic = surface.metallic;
	mRoughness = surface.roughness;
	mAo = surface.ao;
	mEmissiveIntensity = surface.emissiveIntensity;
}

void PBRMaterial::setTextures(PBRTextureInput textures)
{
	mAlbedoMap = std::move(textures.albedoMap);
	mMetallicMap = std::move(textures.metallicMap);
	mRoughnessMap = std::move(textures.roughnessMap);
	mAoMap = std::move(textures.aoMap);
	mNormalMap = std::move(textures.normalMap);
	mEmissiveMap = std::move(textures.emissiveMap);
}

void PBRMaterial::setTextureChannels(PBRTextureChannelInput channels)
{
	mMetallicMapChannel = channels.metallicMapChannel;
	mRoughnessMapChannel = channels.roughnessMapChannel;
	mAoMapChannel = channels.aoMapChannel;
}

void PBRMaterial::setAlphaMask(PBRAlphaMaskInput alphaMask)
{
	mUseAlphaMask = alphaMask.useAlphaMask;
	mAlphaCutoff = alphaMask.alphaCutoff;
}

void PBRMaterial::setIbl(PBRIblInput ibl)
{
	mUseIBL = ibl.useIbl;
	mIblDiffuseStrength = ibl.diffuseStrength;
	mIblSpecularStrength = ibl.specularStrength;
}

void PBRMaterial::setAlbedo(glm::vec3 albedo)
{
	mAlbedo = albedo;
}

void PBRMaterial::setEmissiveColor(glm::vec3 emissiveColor)
{
	mEmissiveColor = emissiveColor;
}

void PBRMaterial::setMetallic(float metallic)
{
	mMetallic = metallic;
}

void PBRMaterial::setRoughness(float roughness)
{
	mRoughness = roughness;
}

void PBRMaterial::setAo(float ao)
{
	mAo = ao;
}

void PBRMaterial::setEmissiveIntensity(float emissiveIntensity)
{
	mEmissiveIntensity = emissiveIntensity;
}

void PBRMaterial::setAlbedoMap(std::shared_ptr<Texture> texture)
{
	mAlbedoMap = std::move(texture);
}

void PBRMaterial::setMetallicMap(std::shared_ptr<Texture> texture)
{
	mMetallicMap = std::move(texture);
}

void PBRMaterial::setRoughnessMap(std::shared_ptr<Texture> texture)
{
	mRoughnessMap = std::move(texture);
}

void PBRMaterial::setAoMap(std::shared_ptr<Texture> texture)
{
	mAoMap = std::move(texture);
}

void PBRMaterial::setNormalMap(std::shared_ptr<Texture> texture)
{
	mNormalMap = std::move(texture);
}

void PBRMaterial::setEmissiveMap(std::shared_ptr<Texture> texture)
{
	mEmissiveMap = std::move(texture);
}

void PBRMaterial::setUseAlphaMask(bool useAlphaMask)
{
	mUseAlphaMask = useAlphaMask;
}

void PBRMaterial::setAlphaCutoff(float alphaCutoff)
{
	mAlphaCutoff = alphaCutoff;
}

void PBRMaterial::setUseIbl(bool useIbl)
{
	mUseIBL = useIbl;
}

void PBRMaterial::setIblStrengths(float diffuseStrength, float specularStrength)
{
	mIblDiffuseStrength = diffuseStrength;
	mIblSpecularStrength = specularStrength;
}

PBRSurfaceRuntimeState PBRMaterial::surfaceState() const
{
	return PBRSurfaceRuntimeState{
		mAlbedo,
		mEmissiveColor,
		mMetallic,
		mRoughness,
		mAo,
		mEmissiveIntensity
	};
}

PBRTextureRuntimeState PBRMaterial::textureState() const
{
	return PBRTextureRuntimeState{
		&mAlbedoMap,
		&mMetallicMap,
		&mRoughnessMap,
		&mAoMap,
		&mNormalMap,
		&mEmissiveMap
	};
}

PBRTextureChannelRuntimeState PBRMaterial::textureChannelState() const
{
	return PBRTextureChannelRuntimeState{
		mMetallicMapChannel,
		mRoughnessMapChannel,
		mAoMapChannel
	};
}

PBRAlphaMaskRuntimeState PBRMaterial::alphaMaskState() const
{
	return PBRAlphaMaskRuntimeState{
		mUseAlphaMask,
		mAlphaCutoff
	};
}

PBRIblRuntimeState PBRMaterial::iblState() const
{
	return PBRIblRuntimeState{
		mUseIBL,
		mIblDiffuseStrength,
		mIblSpecularStrength
	};
}

std::array<PBRTextureSlot, 6> PBRMaterial::getTextureSlots()
{
	return std::array<PBRTextureSlot, 6>{
		PBRTextureSlot{ "Albedo Map", "albedoMap", "useAlbedoMap", &mAlbedoMap },
		PBRTextureSlot{ "Metallic Map", "metallicMap", "useMetallicMap", &mMetallicMap },
		PBRTextureSlot{ "Roughness Map", "roughnessMap", "useRoughnessMap", &mRoughnessMap },
		PBRTextureSlot{ "AO Map", "aoMap", "useAoMap", &mAoMap },
		PBRTextureSlot{ "Normal Map", "normalMap", "useNormalMap", &mNormalMap },
		PBRTextureSlot{ "Emissive Map", "emissiveMap", "useEmissiveMap", &mEmissiveMap },
	};
}

std::array<PBRConstTextureSlot, 6> PBRMaterial::getTextureSlots() const
{
	return std::array<PBRConstTextureSlot, 6>{
		PBRConstTextureSlot{ "Albedo Map", "albedoMap", "useAlbedoMap", &mAlbedoMap },
		PBRConstTextureSlot{ "Metallic Map", "metallicMap", "useMetallicMap", &mMetallicMap },
		PBRConstTextureSlot{ "Roughness Map", "roughnessMap", "useRoughnessMap", &mRoughnessMap },
		PBRConstTextureSlot{ "AO Map", "aoMap", "useAoMap", &mAoMap },
		PBRConstTextureSlot{ "Normal Map", "normalMap", "useNormalMap", &mNormalMap },
		PBRConstTextureSlot{ "Emissive Map", "emissiveMap", "useEmissiveMap", &mEmissiveMap },
	};
}

std::array<PBRVec3UniformSlot, 2> PBRMaterial::getVec3UniformSlots()
{
	return std::array<PBRVec3UniformSlot, 2>{
		PBRVec3UniformSlot{ "Albedo", "pbrAlbedo", &mAlbedo },
		PBRVec3UniformSlot{ "Emissive Color", "pbrEmissiveColor", &mEmissiveColor },
	};
}

std::array<PBRConstVec3UniformSlot, 2> PBRMaterial::getVec3UniformSlots() const
{
	return std::array<PBRConstVec3UniformSlot, 2>{
		PBRConstVec3UniformSlot{ "Albedo", "pbrAlbedo", &mAlbedo },
		PBRConstVec3UniformSlot{ "Emissive Color", "pbrEmissiveColor", &mEmissiveColor },
	};
}

std::array<PBRFloatUniformSlot, 4> PBRMaterial::getSurfaceFloatUniformSlots()
{
	return std::array<PBRFloatUniformSlot, 4>{
		PBRFloatUniformSlot{ "Metallic", "pbrMetallic", &mMetallic, 0.0f, 1.0f },
		PBRFloatUniformSlot{ "Roughness", "pbrRoughness", &mRoughness, 0.04f, 1.0f },
		PBRFloatUniformSlot{ "AO", "pbrAo", &mAo, 0.0f, 1.0f },
		PBRFloatUniformSlot{ "Emissive Intensity", "pbrEmissiveIntensity", &mEmissiveIntensity, 0.0f, 20.0f },
	};
}

std::array<PBRConstFloatUniformSlot, 4> PBRMaterial::getSurfaceFloatUniformSlots() const
{
	return std::array<PBRConstFloatUniformSlot, 4>{
		PBRConstFloatUniformSlot{ "Metallic", "pbrMetallic", &mMetallic, 0.0f, 1.0f },
		PBRConstFloatUniformSlot{ "Roughness", "pbrRoughness", &mRoughness, 0.04f, 1.0f },
		PBRConstFloatUniformSlot{ "AO", "pbrAo", &mAo, 0.0f, 1.0f },
		PBRConstFloatUniformSlot{ "Emissive Intensity", "pbrEmissiveIntensity", &mEmissiveIntensity, 0.0f, 20.0f },
	};
}

std::array<PBRFloatUniformSlot, 2> PBRMaterial::getIblFloatUniformSlots()
{
	return std::array<PBRFloatUniformSlot, 2>{
		PBRFloatUniformSlot{ "IBL Diffuse Strength", "iblDiffuseStrength", &mIblDiffuseStrength, 0.0f, 5.0f },
		PBRFloatUniformSlot{ "IBL Specular Strength", "iblSpecularStrength", &mIblSpecularStrength, 0.0f, 5.0f },
	};
}

std::array<PBRConstFloatUniformSlot, 2> PBRMaterial::getIblFloatUniformSlots() const
{
	return std::array<PBRConstFloatUniformSlot, 2>{
		PBRConstFloatUniformSlot{ "IBL Diffuse Strength", "iblDiffuseStrength", &mIblDiffuseStrength, 0.0f, 5.0f },
		PBRConstFloatUniformSlot{ "IBL Specular Strength", "iblSpecularStrength", &mIblSpecularStrength, 0.0f, 5.0f },
	};
}

PBRMaterialEditControls PBRMaterial::editControls()
{
	return PBRMaterialEditControls{
		&mUseAlphaMask,
		&mAlphaCutoff,
		&mMetallicMapChannel,
		&mRoughnessMapChannel,
		&mAoMapChannel,
		&mUseIBL
	};
}

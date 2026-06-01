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

	struct PBRTextureSlotMetadata
	{
		const char* label{ "" };
		const char* samplerUniform{ "" };
		const char* useFlagUniform{ "" };
		std::shared_ptr<Texture> PBRMaterial::* texture{ nullptr };
	};

	struct PBRVec3UniformMetadata
	{
		const char* label{ "" };
		const char* uniformName{ "" };
		glm::vec3 PBRMaterial::* value{ nullptr };
	};

	struct PBRFloatUniformMetadata
	{
		const char* label{ "" };
		const char* uniformName{ "" };
		float PBRMaterial::* value{ nullptr };
		float minValue{ 0.0f };
		float maxValue{ 1.0f };
	};

	constexpr std::array<PBRTextureSlotMetadata, 6> pbrTextureSlotMetadata{
		PBRTextureSlotMetadata{ "Albedo Map", "albedoMap", "useAlbedoMap", &PBRMaterial::mAlbedoMap },
		PBRTextureSlotMetadata{ "Metallic Map", "metallicMap", "useMetallicMap", &PBRMaterial::mMetallicMap },
		PBRTextureSlotMetadata{ "Roughness Map", "roughnessMap", "useRoughnessMap", &PBRMaterial::mRoughnessMap },
		PBRTextureSlotMetadata{ "AO Map", "aoMap", "useAoMap", &PBRMaterial::mAoMap },
		PBRTextureSlotMetadata{ "Normal Map", "normalMap", "useNormalMap", &PBRMaterial::mNormalMap },
		PBRTextureSlotMetadata{ "Emissive Map", "emissiveMap", "useEmissiveMap", &PBRMaterial::mEmissiveMap },
	};

	constexpr std::array<PBRVec3UniformMetadata, 2> pbrVec3UniformMetadata{
		PBRVec3UniformMetadata{ "Albedo", "pbrAlbedo", &PBRMaterial::mAlbedo },
		PBRVec3UniformMetadata{ "Emissive Color", "pbrEmissiveColor", &PBRMaterial::mEmissiveColor },
	};

	constexpr std::array<PBRFloatUniformMetadata, 4> pbrSurfaceFloatUniformMetadata{
		PBRFloatUniformMetadata{ "Metallic", "pbrMetallic", &PBRMaterial::mMetallic, 0.0f, 1.0f },
		PBRFloatUniformMetadata{ "Roughness", "pbrRoughness", &PBRMaterial::mRoughness, 0.04f, 1.0f },
		PBRFloatUniformMetadata{ "AO", "pbrAo", &PBRMaterial::mAo, 0.0f, 1.0f },
		PBRFloatUniformMetadata{ "Emissive Intensity", "pbrEmissiveIntensity", &PBRMaterial::mEmissiveIntensity, 0.0f, 20.0f },
	};

	constexpr std::array<PBRFloatUniformMetadata, 2> pbrIblFloatUniformMetadata{
		PBRFloatUniformMetadata{ "IBL Diffuse Strength", "iblDiffuseStrength", &PBRMaterial::mIblDiffuseStrength, 0.0f, 5.0f },
		PBRFloatUniformMetadata{ "IBL Specular Strength", "iblSpecularStrength", &PBRMaterial::mIblSpecularStrength, 0.0f, 5.0f },
	};

	template <std::size_t SlotCount>
	std::array<PBRFloatUniformSlot, SlotCount> makeFloatUniformSlots(
		PBRMaterial& material,
		const std::array<PBRFloatUniformMetadata, SlotCount>& metadataList
	)
	{
		std::array<PBRFloatUniformSlot, SlotCount> slots{};
		for (std::size_t index = 0; index < metadataList.size(); ++index)
		{
			const auto& metadata = metadataList[index];
			slots[index] = PBRFloatUniformSlot{
				metadata.label,
				metadata.uniformName,
				&(material.*metadata.value),
				metadata.minValue,
				metadata.maxValue
			};
		}
		return slots;
	}

	template <std::size_t SlotCount>
	std::array<PBRConstFloatUniformSlot, SlotCount> makeFloatUniformSlots(
		const PBRMaterial& material,
		const std::array<PBRFloatUniformMetadata, SlotCount>& metadataList
	)
	{
		std::array<PBRConstFloatUniformSlot, SlotCount> slots{};
		for (std::size_t index = 0; index < metadataList.size(); ++index)
		{
			const auto& metadata = metadataList[index];
			slots[index] = PBRConstFloatUniformSlot{
				metadata.label,
				metadata.uniformName,
				&(material.*metadata.value),
				metadata.minValue,
				metadata.maxValue
			};
		}
		return slots;
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
	std::array<PBRTextureSlot, 6> slots{};
	for (std::size_t index = 0; index < pbrTextureSlotMetadata.size(); ++index)
	{
		const auto& metadata = pbrTextureSlotMetadata[index];
		slots[index] = PBRTextureSlot{
			metadata.label,
			metadata.samplerUniform,
			metadata.useFlagUniform,
			&(this->*metadata.texture)
		};
	}
	return slots;
}

std::array<PBRConstTextureSlot, 6> PBRMaterial::getTextureSlots() const
{
	std::array<PBRConstTextureSlot, 6> slots{};
	for (std::size_t index = 0; index < pbrTextureSlotMetadata.size(); ++index)
	{
		const auto& metadata = pbrTextureSlotMetadata[index];
		slots[index] = PBRConstTextureSlot{
			metadata.label,
			metadata.samplerUniform,
			metadata.useFlagUniform,
			&(this->*metadata.texture)
		};
	}
	return slots;
}

std::array<PBRVec3UniformSlot, 2> PBRMaterial::getVec3UniformSlots()
{
	std::array<PBRVec3UniformSlot, 2> slots{};
	for (std::size_t index = 0; index < pbrVec3UniformMetadata.size(); ++index)
	{
		const auto& metadata = pbrVec3UniformMetadata[index];
		slots[index] = PBRVec3UniformSlot{
			metadata.label,
			metadata.uniformName,
			&(this->*metadata.value)
		};
	}
	return slots;
}

std::array<PBRConstVec3UniformSlot, 2> PBRMaterial::getVec3UniformSlots() const
{
	std::array<PBRConstVec3UniformSlot, 2> slots{};
	for (std::size_t index = 0; index < pbrVec3UniformMetadata.size(); ++index)
	{
		const auto& metadata = pbrVec3UniformMetadata[index];
		slots[index] = PBRConstVec3UniformSlot{
			metadata.label,
			metadata.uniformName,
			&(this->*metadata.value)
		};
	}
	return slots;
}

std::array<PBRFloatUniformSlot, 4> PBRMaterial::getSurfaceFloatUniformSlots()
{
	return makeFloatUniformSlots(*this, pbrSurfaceFloatUniformMetadata);
}

std::array<PBRConstFloatUniformSlot, 4> PBRMaterial::getSurfaceFloatUniformSlots() const
{
	return makeFloatUniformSlots(*this, pbrSurfaceFloatUniformMetadata);
}

std::array<PBRFloatUniformSlot, 2> PBRMaterial::getIblFloatUniformSlots()
{
	return makeFloatUniformSlots(*this, pbrIblFloatUniformMetadata);
}

std::array<PBRConstFloatUniformSlot, 2> PBRMaterial::getIblFloatUniformSlots() const
{
	return makeFloatUniformSlots(*this, pbrIblFloatUniformMetadata);
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

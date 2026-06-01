#include "PBRMaterial.h"

#include <cstddef>

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
	material.mAlbedo = albedo;
	material.mEmissiveColor = emissiveColor;
	material.mMetallic = metallic;
	material.mRoughness = roughness;
	material.mAo = ao;
	material.mEmissiveIntensity = emissiveIntensity;
	material.mUseAlphaMask = useAlphaMask;
	material.mAlphaCutoff = alphaCutoff;
	material.mUseIBL = useIBL;
	material.mIblDiffuseStrength = iblDiffuseStrength;
	material.mIblSpecularStrength = iblSpecularStrength;
}

void PBRMaterialProfile::copyFrom(const PBRMaterial& material)
{
	albedo = material.mAlbedo;
	emissiveColor = material.mEmissiveColor;
	metallic = material.mMetallic;
	roughness = material.mRoughness;
	ao = material.mAo;
	emissiveIntensity = material.mEmissiveIntensity;
	useAlphaMask = material.mUseAlphaMask;
	alphaCutoff = material.mAlphaCutoff;
	useIBL = material.mUseIBL;
	iblDiffuseStrength = material.mIblDiffuseStrength;
	iblSpecularStrength = material.mIblSpecularStrength;
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

bool* PBRMaterial::useAlphaMaskControl()
{
	return &mUseAlphaMask;
}

float* PBRMaterial::alphaCutoffControl()
{
	return &mAlphaCutoff;
}

int* PBRMaterial::metallicMapChannelControl()
{
	return &mMetallicMapChannel;
}

int* PBRMaterial::roughnessMapChannelControl()
{
	return &mRoughnessMapChannel;
}

int* PBRMaterial::aoMapChannelControl()
{
	return &mAoMapChannel;
}

bool* PBRMaterial::useIblControl()
{
	return &mUseIBL;
}

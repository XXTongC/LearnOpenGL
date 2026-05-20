#include "PBRMaterial.h"

#include <cstddef>

#include "tools/inspector/MaterialInspector.h"

using namespace GLframework;

namespace
{
	struct PBRTextureSlotMetadata
	{
		const char* label{ "" };
		const char* samplerUniform{ "" };
		const char* useFlagUniform{ "" };
		std::shared_ptr<Texture> PBRMaterial::* texture{ nullptr };
	};

	constexpr std::array<PBRTextureSlotMetadata, 6> pbrTextureSlotMetadata{
		PBRTextureSlotMetadata{ "Albedo Map", "albedoMap", "useAlbedoMap", &PBRMaterial::mAlbedoMap },
		PBRTextureSlotMetadata{ "Metallic Map", "metallicMap", "useMetallicMap", &PBRMaterial::mMetallicMap },
		PBRTextureSlotMetadata{ "Roughness Map", "roughnessMap", "useRoughnessMap", &PBRMaterial::mRoughnessMap },
		PBRTextureSlotMetadata{ "AO Map", "aoMap", "useAoMap", &PBRMaterial::mAoMap },
		PBRTextureSlotMetadata{ "Normal Map", "normalMap", "useNormalMap", &PBRMaterial::mNormalMap },
		PBRTextureSlotMetadata{ "Emissive Map", "emissiveMap", "useEmissiveMap", &PBRMaterial::mEmissiveMap },
	};
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

void PBRMaterial::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	Material::visitEditableProperties(builder);

	builder.addSection("PBR Surface");
	builder.addColor3("Albedo", &mAlbedo);
	builder.addFloat("Metallic", &mMetallic, 0.0f, 1.0f);
	builder.addFloat("Roughness", &mRoughness, 0.04f, 1.0f);
	builder.addFloat("AO", &mAo, 0.0f, 1.0f);
	builder.addColor3("Emissive Color", &mEmissiveColor);
	builder.addFloat("Emissive Intensity", &mEmissiveIntensity, 0.0f, 20.0f);

	builder.addSection("PBR IBL");
	builder.addBool("Use IBL", &mUseIBL);
	builder.addFloat("IBL Diffuse Strength", &mIblDiffuseStrength, 0.0f, 5.0f);
	builder.addFloat("IBL Specular Strength", &mIblSpecularStrength, 0.0f, 5.0f);

	builder.addSection("PBR Textures");
	for (const auto& slot : getTextureSlots())
	{
		builder.addText(slot.label, [texture = slot.texture]()
		{
			return GL_EDITOR::describeTexture(texture ? *texture : nullptr);
		});
	}
}

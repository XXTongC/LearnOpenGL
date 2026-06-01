#pragma once

#include <array>
#include <memory>

#include "../MaterialEditControls.h"
#include "PBRMaterialProfile.h"
#include "../material.h"
#include "../../framework/texture.h"

namespace GLframework
{
	struct PBRTextureSlot
	{
		const char* label{ "" };
		const char* samplerUniform{ "" };
		const char* useFlagUniform{ "" };
		std::shared_ptr<Texture>* texture{ nullptr };
	};

	struct PBRConstTextureSlot
	{
		const char* label{ "" };
		const char* samplerUniform{ "" };
		const char* useFlagUniform{ "" };
		const std::shared_ptr<Texture>* texture{ nullptr };
	};

	struct PBRVec3UniformSlot
	{
		const char* label{ "" };
		const char* uniformName{ "" };
		glm::vec3* value{ nullptr };
	};

	struct PBRConstVec3UniformSlot
	{
		const char* label{ "" };
		const char* uniformName{ "" };
		const glm::vec3* value{ nullptr };
	};

	struct PBRFloatUniformSlot
	{
		const char* label{ "" };
		const char* uniformName{ "" };
		float* value{ nullptr };
		float minValue{ 0.0f };
		float maxValue{ 1.0f };
	};

	struct PBRConstFloatUniformSlot
	{
		const char* label{ "" };
		const char* uniformName{ "" };
		const float* value{ nullptr };
		float minValue{ 0.0f };
		float maxValue{ 1.0f };
	};

	class PBRMaterial : public Material
	{
	public:
		PBRMaterial();
		void setSurface(PBRSurfaceInput surface);
		void setTextures(PBRTextureInput textures);
		void setTextureChannels(PBRTextureChannelInput channels);
		void setAlphaMask(PBRAlphaMaskInput alphaMask);
		void setIbl(PBRIblInput ibl);
		void setAlbedo(glm::vec3 albedo);
		void setEmissiveColor(glm::vec3 emissiveColor);
		void setMetallic(float metallic);
		void setRoughness(float roughness);
		void setAo(float ao);
		void setEmissiveIntensity(float emissiveIntensity);
		void setAlbedoMap(std::shared_ptr<Texture> texture);
		void setMetallicMap(std::shared_ptr<Texture> texture);
		void setRoughnessMap(std::shared_ptr<Texture> texture);
		void setAoMap(std::shared_ptr<Texture> texture);
		void setNormalMap(std::shared_ptr<Texture> texture);
		void setEmissiveMap(std::shared_ptr<Texture> texture);
		void setUseAlphaMask(bool useAlphaMask);
		void setAlphaCutoff(float alphaCutoff);
		void setUseIbl(bool useIbl);
		void setIblStrengths(float diffuseStrength, float specularStrength);
		PBRSurfaceRuntimeState surfaceState() const;
		PBRTextureRuntimeState textureState() const;
		PBRTextureChannelRuntimeState textureChannelState() const;
		PBRAlphaMaskRuntimeState alphaMaskState() const;
		PBRIblRuntimeState iblState() const;
		std::array<PBRTextureSlot, 6> getTextureSlots();
		std::array<PBRConstTextureSlot, 6> getTextureSlots() const;
		std::array<PBRVec3UniformSlot, 2> getVec3UniformSlots();
		std::array<PBRConstVec3UniformSlot, 2> getVec3UniformSlots() const;
		std::array<PBRFloatUniformSlot, 4> getSurfaceFloatUniformSlots();
		std::array<PBRConstFloatUniformSlot, 4> getSurfaceFloatUniformSlots() const;
		std::array<PBRFloatUniformSlot, 2> getIblFloatUniformSlots();
		std::array<PBRConstFloatUniformSlot, 2> getIblFloatUniformSlots() const;
		PBRMaterialEditControls editControls();

	private:
		std::shared_ptr<Texture> mAlbedoMap{ nullptr };
		std::shared_ptr<Texture> mMetallicMap{ nullptr };
		std::shared_ptr<Texture> mRoughnessMap{ nullptr };
		std::shared_ptr<Texture> mAoMap{ nullptr };
		std::shared_ptr<Texture> mNormalMap{ nullptr };
		std::shared_ptr<Texture> mEmissiveMap{ nullptr };

		glm::vec3 mAlbedo{ 1.0f, 1.0f, 1.0f };
		glm::vec3 mEmissiveColor{ 0.0f, 0.0f, 0.0f };
		float mMetallic{ 0.0f };
		float mRoughness{ 0.5f };
		float mAo{ 1.0f };
		float mEmissiveIntensity{ 0.0f };
		int mMetallicMapChannel{ 0 };
		int mRoughnessMapChannel{ 0 };
		int mAoMapChannel{ 0 };
		bool mUseAlphaMask{ false };
		float mAlphaCutoff{ 0.5f };
		bool mUseIBL{ false };
		float mIblDiffuseStrength{ 1.0f };
		float mIblSpecularStrength{ 1.0f };
	};
}

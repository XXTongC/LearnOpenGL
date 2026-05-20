#pragma once

#include <array>
#include <string>

#include "../material.h"
#include "../../framework/texture.h"

namespace GLframework
{
	class PBRMaterial;

	struct PBRMaterialProfile
	{
		glm::vec3 albedo{ 1.0f, 1.0f, 1.0f };
		glm::vec3 emissiveColor{ 0.0f, 0.0f, 0.0f };
		float metallic{ 0.0f };
		float roughness{ 0.5f };
		float ao{ 1.0f };
		float emissiveIntensity{ 0.0f };
		bool useIBL{ false };
		float iblDiffuseStrength{ 1.0f };
		float iblSpecularStrength{ 1.0f };

		void applyTo(PBRMaterial& material) const;
		void copyFrom(const PBRMaterial& material);
		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder);
	};

	class PBRMaterialProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, PBRMaterialProfile& profile);
		static bool saveToFile(const std::string& path, const PBRMaterialProfile& profile);
	};

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
		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder) override;
		std::array<PBRTextureSlot, 6> getTextureSlots();
		std::array<PBRConstTextureSlot, 6> getTextureSlots() const;
		std::array<PBRVec3UniformSlot, 2> getVec3UniformSlots();
		std::array<PBRConstVec3UniformSlot, 2> getVec3UniformSlots() const;
		std::array<PBRFloatUniformSlot, 4> getSurfaceFloatUniformSlots();
		std::array<PBRConstFloatUniformSlot, 4> getSurfaceFloatUniformSlots() const;
		std::array<PBRFloatUniformSlot, 2> getIblFloatUniformSlots();
		std::array<PBRConstFloatUniformSlot, 2> getIblFloatUniformSlots() const;

	public:
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
		bool mUseIBL{ false };
		float mIblDiffuseStrength{ 1.0f };
		float mIblSpecularStrength{ 1.0f };
	};
}

#pragma once

#include <array>

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

	class PBRMaterial : public Material
	{
	public:
		PBRMaterial();
		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder) override;
		std::array<PBRTextureSlot, 6> getTextureSlots();
		std::array<PBRConstTextureSlot, 6> getTextureSlots() const;

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

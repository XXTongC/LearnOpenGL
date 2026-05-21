#include "AssimpMaterialImporter.h"

#include <algorithm>

#include "materials/pbrMaterial/PBRMaterial.h"
#include "materials/phongMaterial.h"

using namespace GL_APPLICATION;
using namespace GLframework;

namespace
{
	struct LoadedTexture
	{
		std::shared_ptr<Texture> texture{ nullptr };
		std::string sourceKey{};
	};

	glm::vec3 toVec3(const aiColor3D& color)
	{
		return { color.r, color.g, color.b };
	}

	glm::vec3 toVec3(const aiColor4D& color)
	{
		return { color.r, color.g, color.b };
	}

	bool getColor3(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, glm::vec3& value)
	{
		if (!material)
		{
			return false;
		}

		aiColor3D color{};
		if (material->Get(key, type, index, color) == AI_SUCCESS)
		{
			value = toVec3(color);
			return true;
		}

		return false;
	}

	bool getColor4(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, aiColor4D& value)
	{
		return material && material->Get(key, type, index, value) == AI_SUCCESS;
	}

	bool getFloat(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, float& value)
	{
		return material && material->Get(key, type, index, value) == AI_SUCCESS;
	}

	bool getInt(const aiMaterial* material, const char* key, unsigned int type, unsigned int index, int& value)
	{
		return material && material->Get(key, type, index, value) == AI_SUCCESS;
	}

	LoadedTexture loadTexture(
		const aiMaterial* material,
		aiTextureType type,
		const aiScene* scene,
		const std::string& rootPath,
		unsigned int unit,
		bool srgb
	)
	{
		LoadedTexture result{};
		if (!material || !scene)
		{
			return result;
		}

		aiString texturePath{};
		material->Get(AI_MATKEY_TEXTURE(type, 0), texturePath);
		if (texturePath.length == 0)
		{
			return result;
		}

		result.sourceKey = texturePath.C_Str();
		const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
		if (embeddedTexture)
		{
			auto dataIn = reinterpret_cast<unsigned char*>(embeddedTexture->pcData);
			const uint32_t widthIn = embeddedTexture->mWidth;
			const uint32_t heightIn = embeddedTexture->mHeight;
			result.texture = std::make_shared<Texture>(
				unit,
				dataIn,
				widthIn,
				heightIn,
				srgb ? GL_SRGB_ALPHA : GL_RGBA
			);
			return result;
		}

		const std::string fullPath = rootPath + texturePath.C_Str();
		result.sourceKey = fullPath;
		result.texture = srgb
			? Texture::createTexture(fullPath, unit)
			: std::make_shared<Texture>(fullPath, unit, GL_RGBA);
		return result;
	}

	LoadedTexture loadFirstTexture(
		const aiMaterial* material,
		const std::initializer_list<aiTextureType>& textureTypes,
		const aiScene* scene,
		const std::string& rootPath,
		unsigned int unit,
		bool srgb
	)
	{
		for (const auto textureType : textureTypes)
		{
			auto texture = loadTexture(material, textureType, scene, rootPath, unit, srgb);
			if (texture.texture)
			{
				return texture;
			}
		}

		return {};
	}

	std::shared_ptr<Material> createPhongMaterial(
		const aiMaterial* material,
		const aiScene* scene,
		const std::string& rootPath,
		const AssimpMaterialImportOptions& options
	)
	{
		auto phongMaterial = std::make_shared<PhongMaterial>();
		const auto diffuse = loadFirstTexture(
			material,
			{ aiTextureType_DIFFUSE },
			scene,
			rootPath,
			0,
			true
		);
		phongMaterial->mDiffuse = diffuse.texture;
		if (!phongMaterial->mDiffuse && options.useDefaultPhongTextures)
		{
			phongMaterial->mDiffuse = Texture::createTexture("Texture/defaultTexture.jpg", 0);
		}

		const auto specular = loadFirstTexture(
			material,
			{ aiTextureType_SPECULAR },
			scene,
			rootPath,
			1,
			false
		);
		phongMaterial->mSpecularMask = specular.texture;
		if (!phongMaterial->mSpecularMask && options.useDefaultPhongTextures)
		{
			phongMaterial->mSpecularMask = Texture::createTexture("Texture/defaultTexture.jpg", 1);
		}

		float shininess{ phongMaterial->mShiness };
		if (getFloat(material, AI_MATKEY_SHININESS, shininess))
		{
			phongMaterial->mShiness = shininess;
		}

		return phongMaterial;
	}

	std::shared_ptr<Material> createPbrMaterial(
		const aiMaterial* material,
		const aiScene* scene,
		const std::string& rootPath,
		const AssimpMaterialImportOptions& options
	)
	{
		auto pbrMaterial = std::make_shared<PBRMaterial>();
		pbrMaterial->mUseIBL = options.enablePbrIbl;
		pbrMaterial->mAlphaCutoff = options.defaultAlphaCutoff;

		aiColor4D baseColor{};
		if (getColor4(material, AI_MATKEY_BASE_COLOR, baseColor))
		{
			pbrMaterial->mAlbedo = toVec3(baseColor);
			pbrMaterial->setOpacity(std::clamp(baseColor.a, 0.0f, 1.0f));
		}
		else
		{
			glm::vec3 diffuseColor{ pbrMaterial->mAlbedo };
			if (getColor3(material, AI_MATKEY_COLOR_DIFFUSE, diffuseColor))
			{
				pbrMaterial->mAlbedo = diffuseColor;
			}
		}

		float opacity{ pbrMaterial->getOpacity() };
		if (getFloat(material, AI_MATKEY_OPACITY, opacity))
		{
			pbrMaterial->setOpacity(opacity);
		}
		if (pbrMaterial->getOpacity() < 0.999f)
		{
			pbrMaterial->setColorBlendState(true);
			pbrMaterial->setDepthWrite(false);
		}

		getFloat(material, AI_MATKEY_METALLIC_FACTOR, pbrMaterial->mMetallic);
		getFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, pbrMaterial->mRoughness);

		glm::vec3 emissiveColor{ pbrMaterial->mEmissiveColor };
		if (getColor3(material, AI_MATKEY_COLOR_EMISSIVE, emissiveColor))
		{
			pbrMaterial->mEmissiveColor = emissiveColor;
			if (glm::length(emissiveColor) > 0.0001f && pbrMaterial->mEmissiveIntensity <= 0.0f)
			{
				pbrMaterial->mEmissiveIntensity = 1.0f;
			}
		}
		getFloat(material, AI_MATKEY_EMISSIVE_INTENSITY, pbrMaterial->mEmissiveIntensity);

		const auto albedo = loadFirstTexture(
			material,
			{ aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE },
			scene,
			rootPath,
			0,
			true
		);
		pbrMaterial->mAlbedoMap = albedo.texture;

		auto metallic = loadFirstTexture(
			material,
			{ aiTextureType_METALNESS },
			scene,
			rootPath,
			1,
			false
		);
		auto roughness = loadFirstTexture(
			material,
			{ aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_SHININESS },
			scene,
			rootPath,
			2,
			false
		);
		pbrMaterial->mMetallicMap = metallic.texture;
		pbrMaterial->mRoughnessMap = roughness.texture;
		if (metallic.texture && roughness.texture && metallic.sourceKey == roughness.sourceKey)
		{
			pbrMaterial->mMetallicMapChannel = 2;
			pbrMaterial->mRoughnessMapChannel = 1;
		}

		const auto ao = loadFirstTexture(
			material,
			{ aiTextureType_AMBIENT_OCCLUSION, aiTextureType_LIGHTMAP, aiTextureType_AMBIENT },
			scene,
			rootPath,
			3,
			false
		);
		pbrMaterial->mAoMap = ao.texture;

		const auto normal = loadFirstTexture(
			material,
			{ aiTextureType_NORMALS, aiTextureType_NORMAL_CAMERA, aiTextureType_HEIGHT },
			scene,
			rootPath,
			4,
			false
		);
		pbrMaterial->mNormalMap = normal.texture;

		const auto emissive = loadFirstTexture(
			material,
			{ aiTextureType_EMISSION_COLOR, aiTextureType_EMISSIVE },
			scene,
			rootPath,
			5,
			true
		);
		pbrMaterial->mEmissiveMap = emissive.texture;
		if (pbrMaterial->mEmissiveMap && pbrMaterial->mEmissiveIntensity <= 0.0f)
		{
			pbrMaterial->mEmissiveIntensity = 1.0f;
		}

		int blendFunc{ 0 };
		if (getInt(material, AI_MATKEY_BLEND_FUNC, blendFunc))
		{
			pbrMaterial->setColorBlendState(true);
			pbrMaterial->setDepthWrite(false);
		}

		return pbrMaterial;
	}
}

std::shared_ptr<Material> AssimpMaterialImporter::createMaterial(
	const aiMesh* mesh,
	const aiScene* scene,
	const std::string& rootPath,
	const AssimpMaterialImportOptions& options
)
{
	const aiMaterial* material = nullptr;
	if (mesh && scene && mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < scene->mNumMaterials)
	{
		material = scene->mMaterials[mesh->mMaterialIndex];
	}

	if (options.mode == AssimpMaterialImportMode::PBRMetallicRoughness)
	{
		return createPbrMaterial(material, scene, rootPath, options);
	}

	return createPhongMaterial(material, scene, rootPath, options);
}

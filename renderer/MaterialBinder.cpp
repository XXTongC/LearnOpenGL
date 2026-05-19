#include "MaterialBinder.h"

#include <string>

#include "materials/pbrMaterial/PBRMaterial.h"
#include "materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h"
#include "materials/phongMaterial.h"
#include "materials/phongNormalMaterial/phongNormalMaterial.h"
#include "materials/phongParallaxMaterial/phongParallaxMaterial.h"
#include "materials/phongPointShadowMaterial/phongPointShadowMaterial.h"
#include "materials/phongShadowMaterial/phongShadowMaterial.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "renderer/ShadowResourceBinder.h"

using namespace GLframework;

namespace
{
	void setMVPMatrices(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh, Camera* camera)
	{
		shader->setMat4("modelMatrix", mesh->getModelMatrix());
		shader->setMat4("viewMatrix", camera->getViewMatrix());
		shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
	}

	void setNormalMatrix(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh)
	{
		shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrix()))));
	}

	void setCommonMaterialUniforms(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Material>& material, Camera* camera)
	{
		shader->setFloat("opacity", material->getOpacity());
		shader->setFloat("time", static_cast<float>(glfwGetTime()));
		shader->setFloat("speed", 0.5f);
		shader->setVector3("cameraPosition", camera->mPosition);
	}

	void setLightingUniforms(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<DirectionalLight>& dirLight,
		const std::shared_ptr<SpotLight>& spotLight,
		const std::vector<std::shared_ptr<PointLight>>& pointLights,
		const std::shared_ptr<AmbientLight>& ambient
	)
	{
		shader->setVector3("spotLight.position", spotLight->getPosition());
		shader->setVector3("spotLight.color", spotLight->getColor());
		shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
		shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
		shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
		shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));

		shader->setVector3("directionalLight.color", dirLight->getColor());
		shader->setVector3("directionalLight.direction", dirLight->getDirection());
		shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
		shader->setFloat("directionalLight.intensity", dirLight->getIntensity());

		for (size_t i = 0; i < pointLights.size(); i++)
		{
			auto& pointLight = pointLights[i];
			std::string baseName = "pointLights[" + std::to_string(i) + "]";

			shader->setVector3(baseName + ".color", pointLight->getColor());
			shader->setVector3(baseName + ".position", pointLight->getPosition());
			shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
			shader->setFloat(baseName + ".k2", pointLight->getK2());
			shader->setFloat(baseName + ".k1", pointLight->getK1());
			shader->setFloat(baseName + ".k0", pointLight->getK0());
		}
		shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

		shader->setVector3("ambientColor", ambient->getColor());
	}

	void bindTexture(const std::shared_ptr<Shader>& shader, const char* samplerName, const std::shared_ptr<Texture>& texture)
	{
		shader->setInt(samplerName, texture->getUnit());
		texture->Bind();
	}

	void setPhongTextures(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Texture>& diffuse, const std::shared_ptr<Texture>& specularMask)
	{
		bindTexture(shader, "samplerGrass", diffuse);
		bindTexture(shader, "MaskSampler", specularMask);
	}

	void bindOptionalTexture(
		const std::shared_ptr<Shader>& shader,
		const char* samplerName,
		const char* useFlagName,
		const std::shared_ptr<Texture>& texture
	)
	{
		shader->setInt(useFlagName, texture != nullptr ? 1 : 0);
		if (!texture)
		{
			return;
		}

		shader->setInt(samplerName, texture->getUnit());
		texture->Bind();
	}

	void bindPhongMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera,
		const std::shared_ptr<DirectionalLight>& dirLight,
		const std::shared_ptr<SpotLight>& spotLight,
		const std::vector<std::shared_ptr<PointLight>>& pointLights,
		const std::shared_ptr<AmbientLight>& ambient
	)
	{
		std::shared_ptr<PhongMaterial> phongMat = std::static_pointer_cast<PhongMaterial>(material);

		setCommonMaterialUniforms(shader, material, camera);
		setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("shiness", phongMat->mShiness);
	}

	void bindPhongNormalMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera,
		const std::shared_ptr<DirectionalLight>& dirLight,
		const std::shared_ptr<SpotLight>& spotLight,
		const std::vector<std::shared_ptr<PointLight>>& pointLights,
		const std::shared_ptr<AmbientLight>& ambient
	)
	{
		std::shared_ptr<PhongNormalMaterial> phongMat = std::static_pointer_cast<PhongNormalMaterial>(material);

		setCommonMaterialUniforms(shader, material, camera);
		setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
		bindTexture(shader, "NormalMapSampler", phongMat->mNormal);
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("shiness", phongMat->mShiness);
	}

	void bindPhongParallaxMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera,
		const std::shared_ptr<DirectionalLight>& dirLight,
		const std::shared_ptr<SpotLight>& spotLight,
		const std::vector<std::shared_ptr<PointLight>>& pointLights,
		const std::shared_ptr<AmbientLight>& ambient
	)
	{
		std::shared_ptr<PhongParallaxMaterial> phongMat = std::static_pointer_cast<PhongParallaxMaterial>(material);

		setCommonMaterialUniforms(shader, material, camera);
		setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
		bindTexture(shader, "NormalMapSampler", phongMat->mNormal);
		bindTexture(shader, "ParallaxMapSampler", phongMat->mParallaxMap);
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("heightScale", phongMat->mHeightScale);
		shader->setInt("layerNum", phongMat->mLayerNum);
		shader->setFloat("shiness", phongMat->mShiness);
	}

	void bindPBRMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera,
		const std::shared_ptr<DirectionalLight>& dirLight,
		const std::shared_ptr<SpotLight>& spotLight,
		const std::vector<std::shared_ptr<PointLight>>& pointLights,
		const std::shared_ptr<AmbientLight>& ambient
	)
	{
		std::shared_ptr<PBRMaterial> pbrMat = std::static_pointer_cast<PBRMaterial>(material);

		setCommonMaterialUniforms(shader, material, camera);
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);

		shader->setVector3("pbrAlbedo", pbrMat->mAlbedo);
		shader->setFloat("pbrMetallic", pbrMat->mMetallic);
		shader->setFloat("pbrRoughness", pbrMat->mRoughness);
		shader->setFloat("pbrAo", pbrMat->mAo);
		shader->setVector3("pbrEmissiveColor", pbrMat->mEmissiveColor);
		shader->setFloat("pbrEmissiveIntensity", pbrMat->mEmissiveIntensity);

		bindOptionalTexture(shader, "albedoMap", "useAlbedoMap", pbrMat->mAlbedoMap);
		bindOptionalTexture(shader, "metallicMap", "useMetallicMap", pbrMat->mMetallicMap);
		bindOptionalTexture(shader, "roughnessMap", "useRoughnessMap", pbrMat->mRoughnessMap);
		bindOptionalTexture(shader, "aoMap", "useAoMap", pbrMat->mAoMap);
		bindOptionalTexture(shader, "normalMap", "useNormalMap", pbrMat->mNormalMap);
		bindOptionalTexture(shader, "emissiveMap", "useEmissiveMap", pbrMat->mEmissiveMap);
	}

	void bindPhongShadowMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera,
		const std::shared_ptr<DirectionalLight>& dirLight,
		const std::shared_ptr<SpotLight>& spotLight,
		const std::vector<std::shared_ptr<PointLight>>& pointLights,
		const std::shared_ptr<AmbientLight>& ambient
	)
	{
		std::shared_ptr<PhongShadowMaterial> phongMat = std::static_pointer_cast<PhongShadowMaterial>(material);

		setCommonMaterialUniforms(shader, material, camera);
		setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("shiness", phongMat->mShiness);
	}

	void bindPhongCSMShadowMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera,
		const std::shared_ptr<DirectionalLight>& dirLight,
		const std::shared_ptr<SpotLight>& spotLight,
		const std::vector<std::shared_ptr<PointLight>>& pointLights,
		const std::shared_ptr<AmbientLight>& ambient
	)
	{
		std::shared_ptr<PhongCSMShadowMaterial> phongMat = std::static_pointer_cast<PhongCSMShadowMaterial>(material);
		setCommonMaterialUniforms(shader, material, camera);
		setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
		ShadowResourceBinder::bindCSMShadowResources(shader, camera, dirLight);

		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("shiness", phongMat->mShiness);
	}

	void bindPhongPointShadowMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera,
		const std::shared_ptr<DirectionalLight>& dirLight,
		const std::shared_ptr<SpotLight>& spotLight,
		const std::vector<std::shared_ptr<PointLight>>& pointLights,
		const std::shared_ptr<AmbientLight>& ambient
	)
	{
		std::shared_ptr<PhongPointShadowMaterial> phongMat = std::static_pointer_cast<PhongPointShadowMaterial>(material);
		setCommonMaterialUniforms(shader, material, camera);
		setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
		ShadowResourceBinder::bindPointShadowResources(shader, pointLights);
		ShadowResourceBinder::bindDirectionalFallbackShadow(shader, dirLight);

		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);

		shader->setFloat("shiness", phongMat->mShiness);
		shader->setInt("debugShadowMap", 1);
		shader->setInt("debugLightIndex", 0);
	}
}

bool MaterialBinder::bind(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<Material>& material,
	const std::shared_ptr<Mesh>& mesh,
	Camera* camera,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::shared_ptr<SpotLight>& spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const std::shared_ptr<AmbientLight>& ambient
)
{
	switch (material->getMaterialType())
	{
	case MaterialType::PhongMaterial:
		bindPhongMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::PhongNormalMaterial:
		bindPhongNormalMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::PhongParallaxMaterial:
		bindPhongParallaxMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::PBRMaterial:
		bindPBRMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::PhongShadowMaterial:
		bindPhongShadowMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::PhongCSMShadowMaterial:
		bindPhongCSMShadowMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::PhongPointShadowMaterial:
		bindPhongPointShadowMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	default:
		return false;
	}
}

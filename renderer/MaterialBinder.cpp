#include "MaterialBinder.h"

#include <string>

#include "materials/cubeMaterial.h"
#include "materials/cubeSphereMaterial.h"
#include "materials/depthMaterial.h"
#include "materials/opacityMaskMatetial.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "materials/grassInstanceMaterial/grassInstanceMaterial.h"
#include "materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h"
#include "materials/phongEnvMaterial.h"
#include "materials/phongEnvSphereMaterial.h"
#include "materials/phongInstanceMaterial.h"
#include "materials/phongMaterial.h"
#include "materials/phongNormalMaterial/phongNormalMaterial.h"
#include "materials/phongParallaxMaterial/phongParallaxMaterial.h"
#include "materials/phongPointShadowMaterial/phongPointShadowMaterial.h"
#include "materials/phongShadowMaterial/phongShadowMaterial.h"
#include "materials/screenMaterial.h"
#include "materials/whiteMaterial.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "mesh/instancedMesh.h"
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

	void setInstanceMatrixUniforms(const std::shared_ptr<Shader>& shader, const std::shared_ptr<InstancedMesh>& mesh)
	{
		if (mesh->getMatricesUpdateState())
		{
			shader->setMat4Array("matrices", mesh->mInstanceMatrices.data(), mesh->getInstanceCount());
			shader->setInt("matricesUpdateState", 1);
			return;
		}

		shader->setInt("matricesUpdateState", 0);
	}

	void bindWhiteMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera
	)
	{
		setMVPMatrices(shader, mesh, camera);
	}

	void bindDepthMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera
	)
	{
		setMVPMatrices(shader, mesh, camera);
		shader->setFloat("near", camera->mNear);
		shader->setFloat("far", camera->mFar);
	}

	void bindScreenMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material
	)
	{
		std::shared_ptr<ScreenMaterial> screenMaterial = std::static_pointer_cast<ScreenMaterial>(material);
		shader->setInt("screenTextureSampler", 0);
		shader->setInt("depthTextureSampler", 1);
		shader->setFloat("texWidth", 1200.0f);
		shader->setFloat("texHeight", 900.0f);
		shader->setFloat("exposure", screenMaterial->mExposure);
		screenMaterial->mScreenTexture->Bind();
	}

	void bindCubeMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera
	)
	{
		std::shared_ptr<CubeMaterial> cubeMat = std::static_pointer_cast<CubeMaterial>(material);
		mesh->setPosition(camera->mPosition);
		setMVPMatrices(shader, mesh, camera);
		shader->setInt("cubeSampler", 0);
		cubeMat->mDiffuse->setUnit(0);
		cubeMat->mDiffuse->Bind();
		cubeMat->mDiffuse->setUnit(2);
	}

	void bindCubeSphereMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		Camera* camera
	)
	{
		std::shared_ptr<CubeSphereMaterial> cubeMat = std::static_pointer_cast<CubeSphereMaterial>(material);
		mesh->setPosition(camera->mPosition);
		setMVPMatrices(shader, mesh, camera);
		shader->setInt("cubeSampler", 0);
		cubeMat->mDiffuse->setUnit(0);
		cubeMat->mDiffuse->Bind();
		cubeMat->mDiffuse->setUnit(2);
	}

	void bindOpacityMaskMaterial(
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
		std::shared_ptr<OpacityMaskMaterial> opacityMat = std::static_pointer_cast<OpacityMaskMaterial>(material);

		setCommonMaterialUniforms(shader, material, camera);
		bindTexture(shader, "samplerGrass", opacityMat->mDiffuse);
		bindTexture(shader, "opacityMaskSampler", opacityMat->mOpacityrMask);
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("shiness", opacityMat->mShiness);
	}

	void bindPhongEnvMaterial(
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
		std::shared_ptr<PhongEnvMaterial> phongMat = std::static_pointer_cast<PhongEnvMaterial>(material);

		setCommonMaterialUniforms(shader, material, camera);
		setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
		bindTexture(shader, "envSampler", phongMat->mEnv);
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("shiness", phongMat->mShiness);
	}

	void bindPhongEnvSphereMaterial(
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
		std::shared_ptr<PhongEnvSphereMaterial> phongMat = std::static_pointer_cast<PhongEnvSphereMaterial>(material);

		setCommonMaterialUniforms(shader, material, camera);
		setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
		bindTexture(shader, "envSampler", phongMat->mEnv);
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("shiness", phongMat->mShiness);
	}

	void bindPhongInstanceMaterial(
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
		std::shared_ptr<PhongInstanceMaterial> phongMat = std::static_pointer_cast<PhongInstanceMaterial>(material);
		std::shared_ptr<InstancedMesh> instancedMesh = std::static_pointer_cast<InstancedMesh>(mesh);

		setCommonMaterialUniforms(shader, material, camera);
		setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
		setMVPMatrices(shader, mesh, camera);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("shiness", phongMat->mShiness);
		setInstanceMatrixUniforms(shader, instancedMesh);
	}

	void bindGrassInstanceMaterial(
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
		std::shared_ptr<GrassInstanceMaterial> grassMat = std::static_pointer_cast<GrassInstanceMaterial>(material);
		std::shared_ptr<InstancedMesh> instancedMesh = std::static_pointer_cast<InstancedMesh>(mesh);

		instancedMesh->updateMatrices();

		setCommonMaterialUniforms(shader, material, camera);
		shader->setFloat("uvScale", grassMat->getUVScale());
		shader->setFloat("brightness", grassMat->getBrightness());
		shader->setFloat("windScale", grassMat->getWindScale());
		shader->setFloat("phaseScale", grassMat->getPhaseScale());
		shader->setVector3("windDirection", grassMat->getWindDirection());
		shader->setVector3("cloudWhiteColor", grassMat->getCloudWhiteColor());
		shader->setVector3("cloudBlackColor", grassMat->getCloudBlackColor());
		shader->setFloat("cloudUVScale", grassMat->getCloudUVScale());
		shader->setFloat("cloudSpeed", grassMat->getCloudSpeed());
		shader->setFloat("cloudLerp", grassMat->getCloudLerp());
		bindTexture(shader, "samplerGrass", grassMat->mDiffuse);
		bindTexture(shader, "MaskSampler", grassMat->mSpecularMask);
		bindTexture(shader, "opacityMask", grassMat->mOpacityMask);
		bindTexture(shader, "cloudMask", grassMat->mCloudMask);
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("shiness", grassMat->mShiness);
		setInstanceMatrixUniforms(shader, instancedMesh);
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
		ShadowResourceBinder::bindCSMShadowResources(shader, camera, dirLight, 8);

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
	case MaterialType::WhiteMaterial:
		bindWhiteMaterial(shader, mesh, camera);
		return true;
	case MaterialType::DepthMaterial:
		bindDepthMaterial(shader, mesh, camera);
		return true;
	case MaterialType::ScreenMaterial:
		bindScreenMaterial(shader, material);
		return true;
	case MaterialType::CubeMaterial:
		bindCubeMaterial(shader, material, mesh, camera);
		return true;
	case MaterialType::CubeSphereMaterial:
		bindCubeSphereMaterial(shader, material, mesh, camera);
		return true;
	case MaterialType::OpacityMaskMaterial:
		bindOpacityMaskMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::PhongEnvMaterial:
		bindPhongEnvMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::PhongEnvSphereMaterial:
		bindPhongEnvSphereMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::PhongInstanceMaterial:
		bindPhongInstanceMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
	case MaterialType::GrassInstanceMaterial:
		bindGrassInstanceMaterial(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient);
		return true;
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

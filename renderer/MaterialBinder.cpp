#include "MaterialBinder.h"

#include "camera/camera.h"
#include "framework/shader.h"
#include "framework/texture.h"
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
#include "materials/material.h"
#include "materials/whiteMaterial.h"
#include "mesh/mesh.h"
#include "mesh/instancedMesh.h"
#include "renderer/EnvironmentRenderTargets.h"
#include "renderer/LightResourceBinder.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/PBRMaterialBinder.h"
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

	void setPhongSurface(const std::shared_ptr<Shader>& shader, const PhongSurfaceRuntimeState& surface)
	{
		setPhongTextures(shader, *surface.diffuseTexture, *surface.specularMaskTexture);
		shader->setFloat("shiness", surface.shininess);
	}

	void setGrassSurface(const std::shared_ptr<Shader>& shader, const GrassSurfaceRuntimeState& surface)
	{
		bindTexture(shader, "samplerGrass", *surface.diffuseTexture);
		bindTexture(shader, "MaskSampler", *surface.specularMaskTexture);
		bindTexture(shader, "opacityMask", *surface.opacityMaskTexture);
		bindTexture(shader, "cloudMask", *surface.cloudMaskTexture);
		shader->setFloat("shiness", surface.shininess);
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
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
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
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
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
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
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
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
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
		setGrassSurface(shader, grassMat->surfaceState());
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
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
		setPhongSurface(shader, phongMat->surfaceState());
		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
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
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
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
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
		shader->setFloat("heightScale", phongMat->mHeightScale);
		shader->setInt("layerNum", phongMat->mLayerNum);
		shader->setFloat("shiness", phongMat->mShiness);
	}

	void bindPBRMaterial(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<Material>& material,
		const std::shared_ptr<Mesh>& mesh,
		const MaterialBindingContext& context
	)
	{
		PBRMaterialBinder::bind(
			shader,
			std::static_pointer_cast<PBRMaterial>(material),
			mesh,
			context
		);
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
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
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
		setPhongSurface(shader, phongMat->surfaceState());
		ShadowResourceBinder::bindCSMShadowResources(shader, camera, dirLight);

		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
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
		setPhongSurface(shader, phongMat->surfaceState());
		ShadowResourceBinder::bindPointShadowResources(shader, pointLights);
		ShadowResourceBinder::bindDirectionalFallbackShadow(shader, dirLight);

		setMVPMatrices(shader, mesh, camera);
		setNormalMatrix(shader, mesh);
		LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);

		shader->setInt("debugShadowMap", 1);
		shader->setInt("debugLightIndex", 0);
	}
}

bool MaterialBinder::bind(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<Material>& material,
	const std::shared_ptr<Mesh>& mesh,
	const MaterialBindingContext& context
)
{
	switch (material->getMaterialType())
	{
	case MaterialType::WhiteMaterial:
		bindWhiteMaterial(shader, mesh, context.camera);
		return true;
	case MaterialType::DepthMaterial:
		bindDepthMaterial(shader, mesh, context.camera);
		return true;
	case MaterialType::CubeMaterial:
		bindCubeMaterial(shader, material, mesh, context.camera);
		return true;
	case MaterialType::CubeSphereMaterial:
		bindCubeSphereMaterial(shader, material, mesh, context.camera);
		return true;
	case MaterialType::OpacityMaskMaterial:
		bindOpacityMaskMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::PhongEnvMaterial:
		bindPhongEnvMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::PhongEnvSphereMaterial:
		bindPhongEnvSphereMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::PhongInstanceMaterial:
		bindPhongInstanceMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::GrassInstanceMaterial:
		bindGrassInstanceMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::PhongMaterial:
		bindPhongMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::PhongNormalMaterial:
		bindPhongNormalMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::PhongParallaxMaterial:
		bindPhongParallaxMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::PBRMaterial:
		bindPBRMaterial(shader, material, mesh, context);
		return true;
	case MaterialType::PhongShadowMaterial:
		bindPhongShadowMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::PhongCSMShadowMaterial:
		bindPhongCSMShadowMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	case MaterialType::PhongPointShadowMaterial:
		bindPhongPointShadowMaterial(shader, material, mesh, context.camera, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
		return true;
	default:
		return false;
	}
}

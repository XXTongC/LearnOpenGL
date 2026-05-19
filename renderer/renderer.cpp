#include "renderer.h"
#include "RenderState.h"
#include <iostream>
#include <memory>
#include <string>
#include "phongMaterial.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "../tools/ScreenShot.h"
#include "whiteMaterial.h"
#include "opacityMaskMatetial.h"
#include "phongEnvMaterial.h"
#include "phongEnvSphereMaterial.h"
#include "phongInstanceMaterial.h"
#include "materials/grassInstanceMaterial/grassInstanceMaterial.h"
#include "materials/phongParallaxMaterial/phongParallaxMaterial.h"
#include "cubeSphereMaterial.h"
#include "materials/phongNormalMaterial/phongNormalMaterial.h"
#include "materials/phongShadowMaterial/phongShadowMaterial.h"
#include "light/shadow/directionalLightShadow/directionalLightShadow.h"
#include "../camera/perspectivecamera.h"
#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h"
#include "materials/phongPointShadowMaterial/phongPointShadowMaterial.h"
#include "orthographiccamera.h"
#include "../mesh/instancedMesh.h"
#include "cubeMaterial.h"
#include "screenMaterial.h"
#include <algorithm>

#include "tools/tools.h"

using namespace GLframework;

namespace
{
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
}

Renderer::Renderer()
{
	mShaderLibrary.initialize();
}

void Renderer::setMVPMatrices(std::shared_ptr<Shader> shader, std::shared_ptr<Mesh> mesh, Camera* camera)
{
	shader->setMat4("modelMatrix", mesh->getModelMatrix());
	shader->setMat4("viewMatrix", camera->getViewMatrix());
	shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
}

void Renderer::setNormalMatrix(std::shared_ptr<Shader> shader, std::shared_ptr<Mesh> mesh)
{
	shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrix()))));
}

// 璁剧疆鎵€鏈夊厜婧愬弬鏁?
void Renderer::setLightingUniforms(
	std::shared_ptr<Shader> shader,
	std::shared_ptr<DirectionalLight> dirLight,
	std::shared_ptr<SpotLight> spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	std::shared_ptr<AmbientLight> ambient
)
{
	// Spotlight
	shader->setVector3("spotLight.position", spotLight->getPosition());
	shader->setVector3("spotLight.color", spotLight->getColor());
	shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
	shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
	shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
	shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));

	// Directional Light
	shader->setVector3("directionalLight.color", dirLight->getColor());
	shader->setVector3("directionalLight.direction", dirLight->getDirection());
	shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
	shader->setFloat("directionalLight.intensity", dirLight->getIntensity());

	// Point Lights
	for (int i = 0; i < pointLights.size(); i++)
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

	// Ambient
	shader->setVector3("ambientColor", ambient->getColor());
}

// 璁剧疆閫氱敤鏉愯川鍙傛暟
void Renderer::setCommonMaterialUniforms(
	std::shared_ptr<Shader> shader,
	std::shared_ptr<Material> material,
	Camera* camera
)
{
	shader->setFloat("opacity", material->getOpacity());
	shader->setFloat("time", glfwGetTime());
	shader->setFloat("speed", 0.5);
	shader->setVector3("cameraPosition", camera->mPosition);
}

// 璁剧疆 Phong 鏉愯川鐨勭汗鐞?
void Renderer::setPhongTextures(std::shared_ptr<Shader> shader, std::shared_ptr<Texture> diffuse, std::shared_ptr<Texture> specularMask)
{
	shader->setInt("samplerGrass", diffuse->getUnit());
	diffuse->Bind();

	shader->setInt("MaskSampler", 1);
	specularMask->Bind();
}

// 璁剧疆鐐瑰厜婧愰槾褰卞弬鏁?
void Renderer::setPointLightShadowUniforms(
	std::shared_ptr<Shader> shader,
	const std::vector<std::shared_ptr<PointLight>>& pointLights
)
{
	for (int i = 0; i < pointLights.size(); i++)
	{
		const auto& pointLight = pointLights[i];
		const auto& pointShadow = std::static_pointer_cast<PointLightShadow>(pointLight->getShadow());
		std::string baseName = "pointLights[" + std::to_string(i) + "]";

		shader->setVector3(baseName + ".color", pointLight->getColor());
		shader->setVector3(baseName + ".position", pointLight->getPosition());
		shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
		shader->setFloat(baseName + ".k2", pointLight->getK2());
		shader->setFloat(baseName + ".k1", pointLight->getK1());
		shader->setFloat(baseName + ".k0", pointLight->getK0());
		shader->setFloat(baseName + ".far", pointShadow->mCamera->mFar);
		shader->setFloat(baseName + ".near", pointShadow->mCamera->mNear);
	}
	shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());
}

// 缁樺埗缃戞牸锛堝鐞嗘櫘閫氬拰瀹炰緥鍖栫綉鏍硷級
void Renderer::drawMesh(std::shared_ptr<Mesh> mesh)
{
	auto geometry = mesh->getGeometry();
	glBindVertexArray(geometry->getVao());

	if (mesh->getType() == ObjectType::InstancedMesh)
	{
		std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
		glDrawElementsInstanced(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, nullptr, im->getInstanceCount());
	}
	else
	{
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
	}

	glBindVertexArray(0);
}


void Renderer::renderShadowMap(Camera* camera, const std::vector<std::shared_ptr<Mesh>>& meshes, std::shared_ptr<DirectionalLight> dirLight, const std::vector<std::shared_ptr<GLframework::PointLight>>& pointLights)
{
	renderDirShadowMap(camera, mOpacityObjects, dirLight);
	renderPointShadowMap(camera, mOpacityObjects, pointLights);
}

void Renderer::projectObject(std::shared_ptr<Object> obj)
{
	if(obj->getType()==ObjectType::Mesh|| obj->getType() == ObjectType::InstancedMesh)
	{
		std::shared_ptr<Mesh> mesh = std::static_pointer_cast<Mesh>(obj);
		std::shared_ptr<Material> material = mesh->getMaterial();
		if(material->getColorBlendState())
		{
			mTransparentObjects.push_back(mesh);
		}else
		{
			mOpacityObjects.push_back(mesh);
		}
	}
	for(auto& t:obj->getChildren())
	{
		projectObject(t);
	}
}


void Renderer::setClearColor(glm::vec3 color)
{
	glClearColor(color.r, color.g, color.b, 1.0f);
}

std::shared_ptr<Shader> Renderer::getShader(MaterialType type)
{
	return mShaderLibrary.get(type);
}


void Renderer::render(
	std::shared_ptr<Scene> scene,
	Camera* camera,
	std::shared_ptr<DirectionalLight> dirLight,
	std::shared_ptr<SpotLight> spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	std::shared_ptr<AmbientLight> ambient,
	unsigned int fbo
)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// 1. 璁剧疆褰撳墠甯х粯鍒剁殑鏃跺€欙紝opengl鐨勫繀瑕佺姸鎬佹満鍙傛暟
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	// 寮€鍚祴璇曘€佽缃熀鏈啓鍏ョ姸鎬侊紝鎵撳紑妯℃澘娴嬭瘯鍐欏叆
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilMask(0xff); //淇濊瘉浜嗘ā鏉跨紦鍐插彲浠ヨ娓呯悊

	// 榛樿棰滆壊娣峰悎
	glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// 2. 娓呯悊鐢诲竷 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// 娓呯┖涓や釜闃熷垪
	mOpacityObjects.clear();
	mTransparentObjects.clear();

	projectObject(scene);

	std::sort(mTransparentObjects.begin(), mTransparentObjects.end(), [camera](const std::shared_ptr<Mesh>& A,const std::shared_ptr<Mesh>& B)
		{
			//	1. 璁＄畻a鐨勭浉鏈虹郴鐨刏
			auto viewMatrix = camera->getViewMatrix();

			auto modelMatrixA = A->getModelMatrix();
			auto worldPositionA = modelMatrixA * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionA = viewMatrix * worldPositionA;

			//2 璁＄畻b鐨勭浉鏈虹郴鐨刏
			auto modelMatrixB = B->getModelMatrix();
			auto worldPositionB = modelMatrixB * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionB = viewMatrix * worldPositionB;

			return cameraPositionA.z < cameraPositionB.z;
		});

	//	render shadowmap
	renderShadowMap(camera,mOpacityObjects, dirLight,pointLights);

	// 3. 娓叉煋涓や釜闃熷垪
	for(auto& t : mOpacityObjects)
	{
		renderObject(t, camera, dirLight, spotLight, pointLights, ambient);
	}
	for(auto& t : mTransparentObjects)
	{
		renderObject(t, camera, dirLight, spotLight, pointLights, ambient);
	}
	
}


void Renderer::renderObject(
	std::shared_ptr<Object> object,
	Camera* camera,
	std::shared_ptr<DirectionalLight> dirLight,
	std::shared_ptr<SpotLight> spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	std::shared_ptr<AmbientLight> ambient
)
{
	//鍒ゆ柇鏄疢esh杩樻槸Object锛屽鏋滄槸Mesh闇€瑕佹覆鏌?
	if (object->getType() == ObjectType::Mesh||object->getType() == ObjectType::InstancedMesh)
	{
		auto mesh = std::static_pointer_cast<Mesh>(object);
		std::shared_ptr<Geometry> geometry = mesh->getGeometry();

		std::shared_ptr<Material> material = nullptr;
		//鑰冨療鏄惁鎷ユ湁鍏ㄥ眬鏉愯川
		if(mGlobalMaterial!=nullptr)
		{
			material = mGlobalMaterial;
		}else
		{
			material = mesh->getMaterial();
		}

		

		RenderState::applyMaterialState(*material);
		auto shader = getShader(material->getMaterialType());
		shader->begin();


		switch (material->getMaterialType())
		{
		case MaterialType::PhongMaterial:
			{
				std::shared_ptr<PhongMaterial> phongMat = std::static_pointer_cast<PhongMaterial>(material);

				setCommonMaterialUniforms(shader, material, camera);
				setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
				setMVPMatrices(shader, mesh, camera);
				setNormalMatrix(shader, mesh);
				setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);

				shader->setFloat("shiness", phongMat->mShiness);
			}
			break;
		case MaterialType::PBRMaterial:
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
				bindOptionalTexture(shader, "emissiveMap", "useEmissiveMap", pbrMat->mEmissiveMap);
			}
			break;
		case MaterialType::PhongShadowMaterial:
			{
				//std::shared_ptr<PhongShadowMaterial> phongMat = std::static_pointer_cast<PhongShadowMaterial>(material);
				//std::shared_ptr<DirectionalLightShadow> dirShadow = std::static_pointer_cast<DirectionalLightShadow>(dirLight->getShadow());
				//if (phongMat->mDiffuse == nullptr)
				//	std::cout << "null\n";
				//锟斤拷锟斤拷锟斤拷锟斤拷默锟斤拷透锟斤拷锟斤拷--------
				//GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------

				//	diffuse
				//GL_CALL(shader->setInt("samplerGrass", phongMat->mDiffuse->getUnit()));
				//phongMat->mDiffuse->Bind();

				//	mask锟斤拷图
				//GL_CALL(shader->setInt("MaskSampler", 1));
				//phongMat->mSpecularMask->Bind();
				///*
				//GL_CALL(shader->setInt("shadowMapSampler", 2));
				//dirShadow-> mRenderTarget->getDepthAttachment()->setUnit(2);
				//dirShadow->mRenderTarget->getDepthAttachment()->Bind();

				//	PCSS
				//shader->setFloat("lightSize", dirShadow->mLightSize);
				//shader->setMat4("lightViewMatrix", glm::inverse(dirLight->getModelMatrix()));
				//	frustum & nearPlane
				//std::shared_ptr<OrthographicCamera> aCamera = std::static_pointer_cast<OrthographicCamera>(dirShadow->mCamera);
				//shader->setFloat("frustum", aCamera->mR - aCamera->mL);
				//shader->setFloat("nearPlane", aCamera->mNear);

				//shader->setMat4("lightMatrix", dirShadow->getLightMatrix(dirLight->getModelMatrix()));
				//shader->setFloat("bias", dirShadow->mBias);
				//shader->setFloat("diskTightness", dirShadow->mDiskTightness);
				//shader->setFloat("pcfRadius", dirShadow->mPcfRadius);
				//*/
				//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷元锟斤拷锟叫挂癸拷
				//	mvp锟戒化锟斤拷锟斤拷
				//shader->setMat4("modelMatrix", mesh->getModelMatrix());
				//shader->setMat4("viewMatrix", camera->getViewMatrix());
				//shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//锟斤拷锟竭撅拷锟斤拷锟斤拷拢锟斤拷锟斤拷锟阶拷锟斤拷锟斤拷蟹锟斤拷叩谋浠拷锟斤拷锟?
				//shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				//shader->setVector3("spotLight.position", spotLight->getPosition());
				//shader->setVector3("spotLight.color", spotLight->getColor());
				//shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				//shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				//shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				//shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				//shader->setVector3("directionalLight.color", dirLight->getColor());
				//shader->setVector3("directionalLight.direction", dirLight->getDirection());
				//shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
				//shader->setFloat("directionalLight.intensity", dirLight->getIntensity());
				//	pointlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				//std::cout << pointLights.size()<<std::endl;
				//for (int i = 0; i < pointLights.size(); i++)
				//{
				//	auto& pointLight = pointLights[i];
				//	std::string baseName = "pointLights[";
				//	baseName.append(std::to_string(i));
				//	baseName.append("]");
				//	shader->setVector3(baseName + ".color", pointLight->getColor());
				//	shader->setVector3(baseName + ".position", pointLight->getPosition());
				//	shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
				//	shader->setFloat(baseName + ".k2", pointLight->getK2());
				//	shader->setFloat(baseName + ".k1", pointLight->getK1());
				//	shader->setFloat(baseName + ".k0", pointLight->getK0());
				//	shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

				//}

				//shader->setVector3("ambientColor", ambient->getColor());
				//shader->setFloat("time", glfwGetTime());
				//shader->setFloat("shiness", phongMat->mShiness);
				//shader->setFloat("speed", 0.5);

				//	锟斤拷锟斤拷锟较拷锟斤拷锟?
				//shader->setVector3("cameraPosition", camera->mPosition);
				//if (phongMat->mDiffuse == nullptr)
				//	std::cout << "null\n";
				std::shared_ptr<PhongShadowMaterial> phongMat = std::static_pointer_cast<PhongShadowMaterial>(material);

				setCommonMaterialUniforms(shader, material, camera);
				setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);
				setMVPMatrices(shader, mesh, camera);
				setNormalMatrix(shader, mesh);
				setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);

				shader->setFloat("shiness", phongMat->mShiness);
			}
			break;
		case MaterialType::WhiteMaterial:
			{
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				
			}
			break;
		case MaterialType::DepthMaterial:
			{
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				shader->setFloat("near", camera->mNear);
				shader->setFloat("far", camera->mFar);
				
			}
			break;
		case MaterialType::OpacityMaskMaterial:
			{
				std::shared_ptr<OpacityMaskMaterial> opacityMat = std::static_pointer_cast<OpacityMaskMaterial>(material);

				if (opacityMat->mDiffuse == nullptr)
					std::cout << "null\n";
				//锟斤拷锟斤拷锟斤拷锟斤拷默锟斤拷透锟斤拷锟斤拷--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------

				//	锟斤拷锟斤拷shader锟侥诧拷锟斤拷锟斤拷为0锟脚诧拷锟斤拷锟斤拷
				//	diffuse锟斤拷图
				GL_CALL(shader->setInt("samplerGrass", 0));

				//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟皆拷夜锟?
				opacityMat->mDiffuse->Bind();

				//	mask锟斤拷图
				GL_CALL(shader->setInt("opacityMaskSampler", 1));
				opacityMat->mOpacityrMask->Bind();

				//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷元锟斤拷锟叫挂癸拷
				//	mvp锟戒化锟斤拷锟斤拷
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//锟斤拷锟竭撅拷锟斤拷锟斤拷拢锟斤拷锟斤拷锟阶拷锟斤拷锟斤拷蟹锟斤拷叩谋浠拷锟斤拷锟?
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				//std::cout << pointLights.size()<<std::endl;
				for (int i = 0; i < pointLights.size(); i++)
				{
					auto& pointLight = pointLights[i];
					std::string baseName = "pointLights[";
					baseName.append(std::to_string(i));
					baseName.append("]");
					shader->setVector3(baseName + ".color", pointLight->getColor());
					shader->setVector3(baseName + ".position", pointLight->getPosition());
					shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
					shader->setFloat(baseName + ".k2", pointLight->getK2());
					shader->setFloat(baseName + ".k1", pointLight->getK1());
					shader->setFloat(baseName + ".k0", pointLight->getK0());
					shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", opacityMat->mShiness);
				shader->setFloat("speed", 0.5);

				//	锟斤拷锟斤拷锟较拷锟斤拷锟?
				shader->setVector3("cameraPosition", camera->mPosition);
				if (opacityMat->mDiffuse == nullptr)
					std::cout << "null\n";
			}
			break;
		case MaterialType::ScreenMaterial:
			{
				std::shared_ptr<ScreenMaterial> screenMaterial = std::static_pointer_cast<ScreenMaterial>(material);
				shader->setInt("screenTextureSampler", 0);
				shader->setInt("depthTextureSampler", 1);
				shader->setFloat("texWidth", 1200);
				shader->setFloat("texHeight", 900);
				shader->setFloat("exposure", screenMaterial->mExposure);
				screenMaterial->mScreenTexture->Bind();
			}
			break;
 		case MaterialType::CubeSphereMaterial:
			{
				std::shared_ptr<CubeSphereMaterial> cubeMat = std::static_pointer_cast<CubeSphereMaterial>(material);
				mesh->setPosition(camera->mPosition);
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				shader->setInt("cubeSampler", 0);
				cubeMat->mDiffuse->setUnit(0);
				cubeMat->mDiffuse->Bind();
				cubeMat->mDiffuse->setUnit(2);
			}
			break;
		case MaterialType::CubeMaterial:
			{
				std::shared_ptr<CubeMaterial> cubeMat = std::static_pointer_cast<CubeMaterial>(material);
				mesh->setPosition(camera->mPosition);
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				shader->setInt("cubeSampler", 0);
				cubeMat->mDiffuse->setUnit(0);
				cubeMat->mDiffuse->Bind();
				cubeMat->mDiffuse->setUnit(2);
			}
			break;
		case MaterialType::PhongEnvSphereMaterial:
		{
			std::shared_ptr<PhongEnvSphereMaterial> phongMat = std::static_pointer_cast<PhongEnvSphereMaterial>(material);

			if (phongMat->mDiffuse == nullptr)
				std::cout << "null diffuse\n";
			//锟斤拷锟斤拷锟斤拷锟斤拷默锟斤拷透锟斤拷锟斤拷--------
			GL_CALL(shader->setFloat("opacity", material->getOpacity()));
			//-----------------------

			//	锟斤拷锟斤拷shader锟侥诧拷锟斤拷锟斤拷为0锟脚诧拷锟斤拷锟斤拷
			//	diffuse锟斤拷图
			GL_CALL(shader->setInt("samplerGrass", 0));
			//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟皆拷夜锟?
			phongMat->mDiffuse->Bind();

			//	mask锟斤拷图
			GL_CALL(shader->setInt("MaskSampler", 1));
			phongMat->mSpecularMask->Bind();

			//	cube锟斤拷图
			GL_CALL(shader->setInt("envSampler", 2));
			phongMat->mEnv->Bind();

			//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷元锟斤拷锟叫挂癸拷
			//	mvp锟戒化锟斤拷锟斤拷
			shader->setMat4("modelMatrix", mesh->getModelMatrix());
			shader->setMat4("viewMatrix", camera->getViewMatrix());
			shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
			//锟斤拷锟竭撅拷锟斤拷锟斤拷拢锟斤拷锟斤拷锟阶拷锟斤拷锟斤拷蟹锟斤拷叩谋浠拷锟斤拷锟?
			shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
			//	spotlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
			shader->setVector3("spotLight.position", spotLight->getPosition());
			shader->setVector3("spotLight.color", spotLight->getColor());
			shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
			shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
			shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
			shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
			//	dirlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
			shader->setVector3("directionalLight.color", dirLight->getColor());
			shader->setVector3("directionalLight.direction", dirLight->getDirection());
			shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

			//	pointlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
			//std::cout << pointLights.size()<<std::endl;
			for (int i = 0; i < pointLights.size(); i++)
			{
				auto& pointLight = pointLights[i];
				std::string baseName = "pointLights[";
				baseName.append(std::to_string(i));
				baseName.append("]");
				shader->setVector3(baseName + ".color", pointLight->getColor());
				shader->setVector3(baseName + ".position", pointLight->getPosition());
				shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
				shader->setFloat(baseName + ".k2", pointLight->getK2());
				shader->setFloat(baseName + ".k1", pointLight->getK1());
				shader->setFloat(baseName + ".k0", pointLight->getK0());
				shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

			}

			shader->setVector3("ambientColor", ambient->getColor());
			shader->setFloat("time", glfwGetTime());
			shader->setFloat("shiness", phongMat->mShiness);
			shader->setFloat("speed", 0.5);

			//	锟斤拷锟斤拷锟较拷锟斤拷锟?
			shader->setVector3("cameraPosition", camera->mPosition);

		}
			break;
		case MaterialType::PhongEnvMaterial:
			{
				std::shared_ptr<PhongEnvMaterial> phongMat = std::static_pointer_cast<PhongEnvMaterial>(material);

				if (phongMat->mDiffuse == nullptr)
					std::cout << "null diffuse\n";
				//锟斤拷锟斤拷锟斤拷锟斤拷默锟斤拷透锟斤拷锟斤拷--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));
				//-----------------------

				//	锟斤拷锟斤拷shader锟侥诧拷锟斤拷锟斤拷为0锟脚诧拷锟斤拷锟斤拷
				//	diffuse锟斤拷图
				GL_CALL(shader->setInt("samplerGrass", 0));
				//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟皆拷夜锟?
				phongMat->mDiffuse->Bind();

				//	mask锟斤拷图
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();
		
				//	cube锟斤拷图
				GL_CALL(shader->setInt("envSampler", 2));
				phongMat->mEnv->Bind();

				//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷元锟斤拷锟叫挂癸拷
				//	mvp锟戒化锟斤拷锟斤拷
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//锟斤拷锟竭撅拷锟斤拷锟斤拷拢锟斤拷锟斤拷锟阶拷锟斤拷锟斤拷蟹锟斤拷叩谋浠拷锟斤拷锟?
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				//std::cout << pointLights.size()<<std::endl;
				for (int i = 0; i < pointLights.size(); i++)
				{
					auto& pointLight = pointLights[i];
					std::string baseName = "pointLights[";
					baseName.append(std::to_string(i));
					baseName.append("]");
					shader->setVector3(baseName + ".color", pointLight->getColor());
					shader->setVector3(baseName + ".position", pointLight->getPosition());
					shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
					shader->setFloat(baseName + ".k2", pointLight->getK2());
					shader->setFloat(baseName + ".k1", pointLight->getK1());
					shader->setFloat(baseName + ".k0", pointLight->getK0());
					shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setFloat("speed", 0.5);

				//	锟斤拷锟斤拷锟较拷锟斤拷锟?
				shader->setVector3("cameraPosition", camera->mPosition);

			}
			break;
		case MaterialType::PhongInstanceMaterial:
			{
				std::shared_ptr<PhongInstanceMaterial> phongMat = std::static_pointer_cast<PhongInstanceMaterial>(material);
				std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
				//锟斤拷锟斤拷锟斤拷锟斤拷默锟斤拷透锟斤拷锟斤拷--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------



				//	锟斤拷锟斤拷shader锟侥诧拷锟斤拷锟斤拷为0锟脚诧拷锟斤拷锟斤拷
				//	diffuse锟斤拷图
				GL_CALL(shader->setInt("samplerGrass", 0));

				//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟皆拷夜锟?
				phongMat->mDiffuse->Bind();

				//	mask锟斤拷图
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷元锟斤拷锟叫挂癸拷
				//	mvp鍙樻崲鐭╅樀
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//璁＄畻骞朵紶杈撴硶绾跨煩闃?瀹炰緥缁樺埗涓彧鑳藉湪GPU绔繘琛岃绠?
				//shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight鏁版嵁浼犲叆
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight鏁版嵁浼犲叆
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight鏁版嵁浼犲叆
				//std::cout << pointLights.size()<<std::endl;
				for (int i = 0; i < pointLights.size(); i++)
				{
					auto& pointLight = pointLights[i];
					std::string baseName = "pointLights[";
					baseName.append(std::to_string(i));
					baseName.append("]");
					shader->setVector3(baseName + ".color", pointLight->getColor());
					shader->setVector3(baseName + ".position", pointLight->getPosition());
					shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
					shader->setFloat(baseName + ".k2", pointLight->getK2());
					shader->setFloat(baseName + ".k1", pointLight->getK1());
					shader->setFloat(baseName + ".k0", pointLight->getK0());
					shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setFloat("speed", 0.5);

				//	锟斤拷锟斤拷锟较拷锟斤拷锟?
				shader->setVector3("cameraPosition", camera->mPosition);

				//锟斤拷锟斤拷uniform锟斤拷锟酵撅拷锟斤拷浠伙拷锟斤拷锟?
				if(im->getMatricesUpdateState())
				{
					shader->setMat4Array("matrices",im->mInstanceMatrices.data(),im->getInstanceCount());
					shader->setInt("matricesUpdateState", 1);
					//std::cout << "The matrix update as UNIFORM way" << std::endl;
				}else
				{
					shader->setInt("matricesUpdateState", 0);
					//std::cout << "The matrix update as ATTRIBUTE way" << std::endl;
				}
			}
			break;
		case MaterialType::GrassInstanceMaterial:
		{
				std::shared_ptr<GrassInstanceMaterial> instance_material = std::static_pointer_cast<GrassInstanceMaterial>(material);
				std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);

				//im->sortMatrix(camera->getViewMatrix());
				im->updateMatrices();

				//grass texture attribute
				shader->setFloat("uvScale", instance_material->getUVScale());
				shader->setFloat("brightness", instance_material->getBrightness());
				shader->setFloat("windScale", instance_material->getWindScale());
				shader->setFloat("phaseScale", instance_material->getPhaseScale());
				shader->setVector3("windDirection", instance_material->getWindDirection());
				//cloud texture attribute
				shader->setVector3("cloudWhiteColor", instance_material->getCloudWhiteColor());
				shader->setVector3("cloudBlackColor", instance_material->getCloudBlackColor());
				shader->setFloat("cloudUVScale", instance_material->getCloudUVScale());
				shader->setFloat("cloudSpeed", instance_material->getCloudSpeed());
				shader->setFloat("cloudLerp", instance_material->getCloudLerp());
				//璁剧疆閫忔槑搴?-------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));
				//-----------------------

				//	锟斤拷锟斤拷shader锟侥诧拷锟斤拷锟斤拷为0锟脚诧拷锟斤拷锟斤拷
				//	diffuse璐村浘缁戝畾1
				GL_CALL(shader->setInt("samplerGrass", 0));
				instance_material->mDiffuse->Bind();

				//	specularMask缁戝畾
				GL_CALL(shader->setInt("MaskSampler", 1));
				instance_material->mSpecularMask->Bind();

				//	opacityMask缁戝畾
				GL_CALL(shader->setInt("opacityMask", 2));
				instance_material->mOpacityMask->Bind();

				//	couldMask缁戝畾
				GL_CALL(shader->setInt("cloudMask", 3));
				instance_material->mCloudMask->Bind();
				//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷元锟斤拷锟叫挂癸拷
				//	mvp鐭╅樀鍙樻崲
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//锟斤拷锟竭撅拷锟斤拷锟斤拷拢锟斤拷锟斤拷锟阶拷锟斤拷锟斤拷蟹锟斤拷叩谋浠拷锟斤拷锟?
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
				


				//	pointlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				//std::cout << pointLights.size()<<std::endl;
				for (int i = 0; i < pointLights.size(); i++)
				{
					auto& pointLight = pointLights[i];
					std::string baseName = "pointLights[";
					baseName.append(std::to_string(i));
					baseName.append("]");
					shader->setVector3(baseName + ".color", pointLight->getColor());
					shader->setVector3(baseName + ".position", pointLight->getPosition());
					shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
					shader->setFloat(baseName + ".k2", pointLight->getK2());
					shader->setFloat(baseName + ".k1", pointLight->getK1());
					shader->setFloat(baseName + ".k0", pointLight->getK0());
					shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", instance_material->mShiness);
				shader->setFloat("speed", 0.5);

				//	锟斤拷锟斤拷锟较拷锟斤拷锟?
				shader->setVector3("cameraPosition", camera->mPosition);

				//	matrix update as uniform way : 1, attribute way : 0
				if (im->getMatricesUpdateState())
				{
					shader->setMat4Array("matrices", im->mInstanceMatrices.data(), im->getInstanceCount());
					shader->setInt("matricesUpdateState", 1);
					//std::cout << "The matrix update as UNIFORM way" << std::endl;
				}
				else
				{
					shader->setInt("matricesUpdateState", 0);
					//std::cout << "The matrix update as ATTRIBUTE way" << std::endl;
				}
		}
			break;
		case MaterialType::PhongNormalMaterial:
			{
				//std::shared_ptr<PhongNormalMaterial> phongMat = std::static_pointer_cast<PhongNormalMaterial>(material);

				////锟斤拷锟斤拷锟斤拷锟斤拷默锟斤拷透锟斤拷锟斤拷--------
				//GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				////-----------------------
				////	shader settings
				////	diffuse sampler set
				//GL_CALL(shader->setInt("samplerGrass", 0));
				//phongMat->mDiffuse->Bind();

				////	mask sampler set
				//GL_CALL(shader->setInt("MaskSampler", 1));
				//phongMat->mSpecularMask->Bind();

				////	normal sampler set
				//GL_CALL(shader->setInt("NormalMapSampler", 2));
				//phongMat->mNormal->Bind();

				////	mvp transform settings
				//shader->setMat4("modelMatrix", mesh->getModelMatrix());
				//shader->setMat4("viewMatrix", camera->getViewMatrix());
				//shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				////	spotlight settings
				//shader->setVector3("spotLight.position", spotLight->getPosition());
				//shader->setVector3("spotLight.color", spotLight->getColor());
				//shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				//shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				//shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				//shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				////	dirlight settings
				//shader->setVector3("directionalLight.color", dirLight->getColor());
				//shader->setVector3("directionalLight.direction", dirLight->getDirection());
				//shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
				//shader->setFloat("directionalLight.intensity", dirLight->getIntensity());
				////	pointlight settings
				////std::cout << pointLights.size()<<std::endl;
				//for (int i = 0; i < pointLights.size(); i++)
				//{
				//	auto& pointLight = pointLights[i];
				//	std::string baseName = "pointLights[";
				//	baseName.append(std::to_string(i));
				//	baseName.append("]");
				//	shader->setVector3(baseName + ".color", pointLight->getColor());
				//	shader->setVector3(baseName + ".position", pointLight->getPosition());
				//	shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
				//	shader->setFloat(baseName + ".k2", pointLight->getK2());
				//	shader->setFloat(baseName + ".k1", pointLight->getK1());
				//	shader->setFloat(baseName + ".k0", pointLight->getK0());
				//	shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

				//}

				//shader->setVector3("ambientColor", ambient->getColor());
				//shader->setFloat("time", glfwGetTime());
				//shader->setFloat("shiness", phongMat->mShiness);
				//shader->setFloat("speed", 0.5);

				//shader->setVector3("cameraPosition", camera->mPosition);
				//if (phongMat->mDiffuse == nullptr)
				//	std::cout << "mDiffuse null\n";
				//if (phongMat->mNormal == nullptr)
				//	std::cout << "mNormal null\n";
				std::shared_ptr<PhongNormalMaterial> phongMat = std::static_pointer_cast<PhongNormalMaterial>(material);

				setCommonMaterialUniforms(shader, material, camera);
				setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);

				shader->setInt("NormalMapSampler", 2);
				phongMat->mNormal->Bind();

				setMVPMatrices(shader, mesh, camera);
				setNormalMatrix(shader, mesh);
				setLightingUniforms(shader, dirLight, spotLight, pointLights, ambient);

				shader->setFloat("shiness", phongMat->mShiness);
		}
			break;
		case MaterialType::PhongParallaxMaterial:
			{
				std::shared_ptr<PhongParallaxMaterial> phongMat = std::static_pointer_cast<PhongParallaxMaterial>(material);

				//锟斤拷锟斤拷锟斤拷锟斤拷默锟斤拷透锟斤拷锟斤拷--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------
				//	shader settings
				//	diffuse sampler set
				GL_CALL(shader->setInt("samplerGrass", 0));
				phongMat->mDiffuse->Bind();

				//	mask sampler set
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				//	normal sampler set
				GL_CALL(shader->setInt("NormalMapSampler", 2));
				phongMat->mNormal->Bind();

				//	parallax sampler set
				GL_CALL(shader->setInt("ParallaxMapSampler", 3));
				phongMat->mParallaxMap->Bind();

				shader->setFloat("heightScale", phongMat->mHeightScale);
				shader->setInt("layerNum", phongMat->mLayerNum);
				//	mvp transform settings
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight settings
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight settings
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
				shader->setFloat("directionalLight.intensity", dirLight->getIntensity());
				//	pointlight settings
				//std::cout << pointLights.size()<<std::endl;
				for (int i = 0; i < pointLights.size(); i++)
				{
					auto& pointLight = pointLights[i];
					std::string baseName = "pointLights[";
					baseName.append(std::to_string(i));
					baseName.append("]");
					shader->setVector3(baseName + ".color", pointLight->getColor());
					shader->setVector3(baseName + ".position", pointLight->getPosition());
					shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
					shader->setFloat(baseName + ".k2", pointLight->getK2());
					shader->setFloat(baseName + ".k1", pointLight->getK1());
					shader->setFloat(baseName + ".k0", pointLight->getK0());
					shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setFloat("speed", 0.5);

				shader->setVector3("cameraPosition", camera->mPosition);
			}
			break;
		case MaterialType::PhongCSMShadowMaterial:
			{
				std::shared_ptr<PhongCSMShadowMaterial> phongMat = std::static_pointer_cast<PhongCSMShadowMaterial>(material);
				std::shared_ptr<DirectionalLightCSMShadow> dirCSMShadow = std::static_pointer_cast<DirectionalLightCSMShadow>(dirLight->getShadow());
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
				//锟斤拷锟斤拷锟斤拷锟斤拷默锟斤拷透锟斤拷锟斤拷--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------

				//	diffuse 
				GL_CALL(shader->setInt("samplerGrass", phongMat->mDiffuse->getUnit()));
				phongMat->mDiffuse->Bind();

				//	mask锟斤拷图
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				//	CSM
				shader->setInt("csmLayerCount", dirCSMShadow->getLayerCount());
				std::vector<float> layers;
				dirCSMShadow->generateCascadeLayers(layers, camera->mNear, camera->mFar);
				shader->setFloatArray("csmLayers", layers.data(), layers.size());

				
				GL_CALL(shader->setInt("shadowMapSampler", 2));
				dirCSMShadow->mRenderTarget->getDepthAttachment()->setUnit(2);
				dirCSMShadow->mRenderTarget->getDepthAttachment()->Bind();
				
				auto lightMatrices = (dirCSMShadow->getLightMatrix(camera, dirLight->getDirection(), layers));
				shader->setMat4Array("lightMatrices", lightMatrices.data(), lightMatrices.size());

				//	PCSS
				shader->setFloat("lightSize", dirCSMShadow->mLightSize);
				shader->setMat4("lightViewMatrix", glm::inverse(dirLight->getModelMatrix()));
				//	frustum & nearPlane
				//std::shared_ptr<OrthographicCamera> aCamera = std::static_pointer_cast<OrthographicCamera>(dirCSMShadow->mCamera);
				//shader->setFloat("frustum", aCamera->mR - aCamera->mL);
				//shader->setFloat("nearPlane", aCamera->mNear);

				//shader->setMat4("lightMatrix", dirShadow->getLightMatrix(dirLight->getModelMatrix()));
				shader->setFloat("bias", dirCSMShadow->mBias);
				shader->setFloat("diskTightness", dirCSMShadow->mDiskTightness);
				shader->setFloat("pcfRadius", dirCSMShadow->mPcfRadius);

				//	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷元锟斤拷锟叫挂癸拷
				//	mvp锟戒化锟斤拷锟斤拷
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//锟斤拷锟竭撅拷锟斤拷锟斤拷拢锟斤拷锟斤拷锟阶拷锟斤拷锟斤拷蟹锟斤拷叩谋浠拷锟斤拷锟?
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
				shader->setFloat("directionalLight.intensity", dirLight->getIntensity());
				//	pointlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				//std::cout << pointLights.size()<<std::endl;
				for (int i = 0; i < pointLights.size(); i++)
				{
					auto& pointLight = pointLights[i];
					std::string baseName = "pointLights[";
					baseName.append(std::to_string(i));
					baseName.append("]");
					shader->setVector3(baseName + ".color", pointLight->getColor());
					shader->setVector3(baseName + ".position", pointLight->getPosition());
					shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
					shader->setFloat(baseName + ".k2", pointLight->getK2());
					shader->setFloat(baseName + ".k1", pointLight->getK1());
					shader->setFloat(baseName + ".k0", pointLight->getK0());
					shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setFloat("speed", 0.5);

				//	锟斤拷锟斤拷锟较拷锟斤拷锟?
				shader->setVector3("cameraPosition", camera->mPosition);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";

			}
			break;
		case MaterialType::PhongPointShadowMaterial:
			{
				//std::shared_ptr<PhongPointShadowMaterial> phongMat = std::static_pointer_cast<PhongPointShadowMaterial>(material);
				//std::shared_ptr<DirectionalLightShadow> dirShadow = std::static_pointer_cast<DirectionalLightShadow>(dirLight->getShadow());

				//if (phongMat->mDiffuse == nullptr)
				//	std::cout << "null\n";
				////锟斤拷锟斤拷锟斤拷锟斤拷默锟斤拷透锟斤拷锟斤拷--------
				//GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				////-----------------------

				////	diffuse
				//GL_CALL(shader->setInt("samplerGrass", phongMat->mDiffuse->getUnit()));
				//phongMat->mDiffuse->Bind();

				////	mask锟斤拷图
				//GL_CALL(shader->setInt("MaskSampler", 1));
				//phongMat->mSpecularMask->Bind();
				//
				//GL_CALL(shader->setInt("pointShadowMaps", 2));  // 浣跨敤绾圭悊鍗曞厓2
				//PointLightShadow::getSharedDepthTexture()->setUnit(2);
				//PointLightShadow::getSharedDepthTexture()->Bind();

				//glm::mat4 directionalLightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f,80.0f);
				//glm::mat4 directionalLightView = glm::lookAt(dirLight->getPosition(), dirLight->getPosition() + glm::vec3(1.0f), glm::vec3(0.0, 1.0, 0.0));
				//glm::mat4 directionalLightSpaceMatrix = directionalLightProjection * directionalLightView;

				//shader->setMat4("directionalLightSpaceMatrix", directionalLightSpaceMatrix);
				//
				//
				////	PCSS
				//
				///*
				//shader->setFloat("lightSize", dirLight->getShadow()->mLightSize);
				//shader->setMat4("lightViewMatrix", glm::inverse(dirLight->getModelMatrix()));
				////	frustum & nearPlane
				//std::shared_ptr<OrthographicCamera> aCamera = std::static_pointer_cast<OrthographicCamera>(dirShadow->mCamera);
				//shader->setFloat("frustum", aCamera->mR - aCamera->mL);
				//shader->setFloat("nearPlane", aCamera->mNear);
				//
				//
				//shader->setMat4("lightMatrix", dirShadow->getLightMatrix(dirLight->getModelMatrix()));
				//*/
				//shader->setFloat("bias", dirShadow->mBias);
				//shader->setFloat("diskTightness", dirShadow->mDiskTightness);
				//shader->setFloat("pcfRadius", dirShadow->mPcfRadius);
				//
				////	锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷元锟斤拷锟叫挂癸拷
				////	mvp锟戒化锟斤拷锟斤拷
				//shader->setMat4("modelMatrix", mesh->getModelMatrix());
				//shader->setMat4("viewMatrix", camera->getViewMatrix());
				//shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				////锟斤拷锟竭撅拷锟斤拷锟斤拷拢锟斤拷锟斤拷锟阶拷锟斤拷锟斤拷蟹锟斤拷叩谋浠拷锟斤拷锟?
				//shader->setMat3("normalMatrix", glm::mat(glm::transpose(glm::inverse(mesh->getModelMatrix()))));
				////	spotlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				//shader->setVector3("spotLight.position", spotLight->getPosition());
				//shader->setVector3("spotLight.color", spotLight->getColor());
				//shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				//shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				//shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				//shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				////	dirlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				//shader->setVector3("directionalLight.color", dirLight->getColor());
				//shader->setVector3("directionalLight.direction", dirLight->getDirection());
				//shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
				//shader->setFloat("directionalLight.intensity", dirLight->getIntensity());
				////	pointlight锟斤拷源锟斤拷锟斤拷锟斤拷锟斤拷
				////std::cout << pointLights.size()<<std::endl;
				//for (int i = 0; i < pointLights.size(); i++)
				//{
				//	const auto& pointLight = pointLights[i];
				//	const auto& pointShadow = std::static_pointer_cast<PointLightShadow>(pointLight->getShadow());
				//	std::string baseName = "pointLights[";
				//	baseName.append(std::to_string(i));
				//	baseName.append("]");
				//	shader->setVector3(baseName + ".color", pointLight->getColor());
				//	shader->setVector3(baseName + ".position", pointLight->getPosition());
				//	shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
				//	shader->setFloat(baseName + ".k2", pointLight->getK2());
				//	shader->setFloat(baseName + ".k1", pointLight->getK1());
				//	shader->setFloat(baseName + ".k0", pointLight->getK0());
				//	shader->setFloat(baseName + ".far", pointShadow->mCamera->mFar);
				//	shader->setFloat(baseName + ".near", pointShadow->mCamera->mNear);

				//	shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

				//}
				//shader->setInt("debugShadowMap", 1);
				//shader->setInt("debugLightIndex", 0); // 瑕佽皟璇曠殑鐐瑰厜婧愮储寮?

				//shader->setVector3("ambientColor", ambient->getColor());
				//shader->setFloat("time", glfwGetTime());
				//shader->setFloat("shiness", phongMat->mShiness);
				//shader->setFloat("speed", 0.5);

				////	锟斤拷锟斤拷锟较拷锟斤拷锟?
				//shader->setVector3("cameraPosition", camera->mPosition);
				//if (phongMat->mDiffuse == nullptr)
				//	std::cout << "null\n";
				std::shared_ptr<PhongPointShadowMaterial> phongMat = std::static_pointer_cast<PhongPointShadowMaterial>(material);
				std::shared_ptr<DirectionalLightShadow> dirShadow = std::static_pointer_cast<DirectionalLightShadow>(dirLight->getShadow());

				setCommonMaterialUniforms(shader, material, camera);
				setPhongTextures(shader, phongMat->mDiffuse, phongMat->mSpecularMask);

				// Point shadow specific
				shader->setInt("pointShadowMaps", 2);
				PointLightShadow::getSharedDepthTexture()->setUnit(2);
				PointLightShadow::getSharedDepthTexture()->Bind();

				glm::mat4 directionalLightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 80.0f);
				glm::mat4 directionalLightView = glm::lookAt(dirLight->getPosition(),
					dirLight->getPosition() + glm::vec3(1.0f),
					glm::vec3(0.0, 1.0, 0.0));
				shader->setMat4("directionalLightSpaceMatrix", directionalLightProjection* directionalLightView);

				shader->setFloat("bias", dirShadow->mBias);
				shader->setFloat("diskTightness", dirShadow->mDiskTightness);
				shader->setFloat("pcfRadius", dirShadow->mPcfRadius);

				setMVPMatrices(shader, mesh, camera);
				setNormalMatrix(shader, mesh);

				// Use specialized point light shadow uniforms
				setPointLightShadowUniforms(shader, pointLights);

				// Set other lights (spotlight and directional)
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

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("shiness", phongMat->mShiness);

				shader->setInt("debugShadowMap", 1);
				shader->setInt("debugLightIndex", 0);

			}
			break;
		default:
			std::cout << "wrong\n";
			break;
		}
		//// 3. 锟斤拷vao
		//glBindVertexArray(geometry->getVao());
		//// 4. 閫夋嫨geometry鐨勭粯鐢绘柟寮忥紙鍗曚緥缁樼敾/瀹炲姏缁樼敾锛?
		////glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//if(object->getType()==ObjectType::Mesh)
		//	glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, static_cast<void*>(nullptr));
		//else
		//{
		//	std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
		//	glDrawElementsInstanced(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0, im->getInstanceCount());
		//}
		//GL_CALL(glBindVertexArray(0));
		drawMesh(mesh);
		shader->end();
	}
	
}


void Renderer::renderDirShadowMap(
	Camera* camera,
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	std::shared_ptr<DirectionalLight> dirLight
)
{
	//	1. make sure that the current draw is not a postProcessPass draw, if it is, then the render is not performed
	bool isPostProcessPass = true;
	for (auto& mesh : meshes)
	{
		if (mesh->getMaterial()->getMaterialType() != GLframework::MaterialType::ScreenMaterial)
		{
			isPostProcessPass = false;
			break;
		}
	}
	if (isPostProcessPass) return;

	//	2. save the original state, after drawing shadowmap, to restore the original state
	GLint preFbo;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &preFbo);

	GLint preViewPort[4];
	glGetIntegerv(GL_VIEWPORT, preViewPort);

	//	3. set the state required when ShadowPass is drawn
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);



	//	4. The loop renders a shadowMap for each child cone
	auto csmShadow = std::static_pointer_cast<DirectionalLightCSMShadow>(dirLight->getShadow());
	glBindFramebuffer(GL_FRAMEBUFFER, csmShadow->mRenderTarget->getFBO());

	std::vector<float> layers;
	csmShadow->generateCascadeLayers(layers, camera->mNear, camera->mFar);
	auto lightMatrices = csmShadow->getLightMatrix(camera, dirLight->getDirection(), layers);
	glViewport(0, 0, csmShadow->mRenderTarget->getWidth(), csmShadow->mRenderTarget->getHeight());

	for (int i = 0; i < csmShadow->getLayerCount(); ++i)
	{
		auto shadowShader = mShaderLibrary.getShadowShader();
		glFramebufferTextureLayer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			csmShadow->mRenderTarget->getDepthAttachment()->getTexture(),
			0,
			i
		);
		glClear(GL_DEPTH_BUFFER_BIT);
		shadowShader->begin();
		shadowShader->setMat4("lightMatrix", lightMatrices[i]);
		for (auto& mesh : meshes)
		{
			glBindVertexArray(mesh->getGeometry()->getVao());
			shadowShader->setMat4("modelMatrix", mesh->getModelMatrix());

			if (mesh->getType() == ObjectType::InstancedMesh)
			{
				std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
				glDrawElementsInstanced(GL_TRIANGLES, mesh->getGeometry()->getIndicesCount(), GL_UNSIGNED_INT, nullptr,
					im->getInstanceCount());
			}
			else
			{
				glDrawElements(GL_TRIANGLES, mesh->getGeometry()->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
			}
		}

		shadowShader->end();

	}

	glBindFramebuffer(GL_FRAMEBUFFER, preFbo);
	glViewport(preViewPort[0], preViewPort[1], preViewPort[2], preViewPort[3]);
}

void Renderer::renderPointShadowMap(
	Camera* camera,
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::vector<std::shared_ptr<PointLight>>& pointLights
)
{
	bool isPostProcessPass = true;
	for (auto& mesh : meshes)
	{
		if (mesh->getMaterial()->getMaterialType() != MaterialType::ScreenMaterial)
		{
			isPostProcessPass = false;
			break;
		}
	}
	if (isPostProcessPass) return;
	// store state
	GLint preFbo;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &preFbo);
	GLint preViewPort[4];
	glGetIntegerv(GL_VIEWPORT, preViewPort);

	// set render state
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	auto depthTexture = PointLightShadow::getSharedDepthTexture();
	
	int width = depthTexture->getWidth();
	int height = depthTexture->getHeight();

	// 鍒涘缓骞剁粦瀹氫复鏃禙BO
	GLuint tempFBO;
	glGenFramebuffers(1, &tempFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
	//std::cout << "temp " << tempFBO << std::endl;
	
	//glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture->getTexture(), 0, 0);
	glViewport(0, 0, width, height);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);


	for (size_t i = 0; i < pointLights.size(); ++i)
	{
		const auto& pointLight = pointLights[i];
		const auto& pointShadow = std::static_pointer_cast<PointLightShadow>(pointLight->getShadow());
		pointShadow->setShadowMapIndex(i);
		auto shadowDistanceShader = mShaderLibrary.getShadowDistanceShader();
		
		// 娓叉煋鍏釜闈㈢殑娣卞害璐村浘
		for (unsigned int face = 0; face < 6; ++face)
		{
			int layerIndex = pointShadow->getShadowMapIndex() * 6 + face;
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture->getTexture(), 0, layerIndex);
			// 妫€鏌?FBO 鐘舵€?
			//std::cerr << layerIndex << std::endl;


			glClear(GL_DEPTH_BUFFER_BIT);

			// 璁剧疆鍏夋簮瑙嗗浘鍜屾姇褰辩煩闃?
			glm::mat4 shadowProj = std::static_pointer_cast<PerspectiveCamera>(pointShadow->mCamera)->getProjectionMatrix();
			glm::mat4 shadowView = lookAt(pointLight->getPosition(),
			                              pointLight->getPosition() + Tools::getCubemapFaceDirection(face),
			                              Tools::getCubemapFaceUp(face));
			shadowDistanceShader->begin();
			shadowDistanceShader->setMat4("lightSpaceMatrix", shadowProj * shadowView);
			shadowDistanceShader->setVector3("lightPos", pointLight->getPosition());
			shadowDistanceShader->setFloat("far_plane", pointShadow->mCamera->mFar);

			//****text
			for (auto& mesh : meshes)
			{
				glBindVertexArray(mesh->getGeometry()->getVao());
				shadowDistanceShader->setMat4("modelMatrix", mesh->getModelMatrix());

				if (mesh->getType() == ObjectType::InstancedMesh)
				{
					std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
					glDrawElementsInstanced(GL_TRIANGLES, mesh->getGeometry()->getIndicesCount(), GL_UNSIGNED_INT,
					                        nullptr,
					                        im->getInstanceCount());
				}
				else
				{
					glDrawElements(GL_TRIANGLES, mesh->getGeometry()->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
				}
			}
		
			
			shadowDistanceShader->end();
	
		}
	}

	// delete tempFBO
	glDeleteFramebuffers(1, &tempFBO);

	// render back last state
	glBindFramebuffer(GL_FRAMEBUFFER, preFbo);
	glViewport(preViewPort[0], preViewPort[1], preViewPort[2], preViewPort[3]);
}

void Renderer::msaaResolve(std::shared_ptr<Framebuffer> src, std::shared_ptr<Framebuffer> dst)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER,src->getFBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->getFBO());
	glBlitFramebuffer(0, 0, src->getWidth(), src->getHeight(), 0, 0, dst->getWidth(), dst->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

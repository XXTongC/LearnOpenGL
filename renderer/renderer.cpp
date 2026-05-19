#include "renderer.h"
#include "MaterialBinder.h"
#include "RenderState.h"
#include <iostream>
#include <memory>
#include <string>
#include "opacityMaskMatetial.h"
#include "phongEnvMaterial.h"
#include "phongEnvSphereMaterial.h"
#include "phongInstanceMaterial.h"
#include "materials/grassInstanceMaterial/grassInstanceMaterial.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "../mesh/instancedMesh.h"
#include <algorithm>

using namespace GLframework;

Renderer::Renderer()
{
	mShaderLibrary.initialize();
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
	mShadowRenderer.render(camera, mOpacityObjects, dirLight, pointLights, mShaderLibrary);

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


		if (!MaterialBinder::bind(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient))
		{
			switch (material->getMaterialType())
			{
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
				shader->setFloat("time", static_cast<float>(glfwGetTime()));
				shader->setFloat("shiness", opacityMat->mShiness);
				shader->setFloat("speed", 0.5);

				//	锟斤拷锟斤拷锟较拷锟斤拷锟?
				shader->setVector3("cameraPosition", camera->mPosition);
				if (opacityMat->mDiffuse == nullptr)
					std::cout << "null\n";
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
			shader->setFloat("time", static_cast<float>(glfwGetTime()));
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
				shader->setFloat("time", static_cast<float>(glfwGetTime()));
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
				shader->setFloat("time", static_cast<float>(glfwGetTime()));
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
				shader->setFloat("time", static_cast<float>(glfwGetTime()));
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
		default:
			std::cout << "wrong\n";
			break;
			}
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


void Renderer::msaaResolve(std::shared_ptr<Framebuffer> src, std::shared_ptr<Framebuffer> dst)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER,src->getFBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->getFBO());
	glBlitFramebuffer(0, 0, src->getWidth(), src->getHeight(), 0, 0, dst->getWidth(), dst->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

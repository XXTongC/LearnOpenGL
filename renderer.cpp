#include "renderer.h"
#include <iostream>
#include <memory>
#include <string>
#include "phongMaterial.h"
#include "whiteMaterial.h"
#include "opacityMaskMatetial.h"
#include "phongEnvMaterial.h"
#include "phongEnvSphereMaterial.h"
#include "phongInstanceMaterial.h"
#include "cubeSphereMaterial.h"
#include "../mesh/instancedMesh.h"
#include "cubeMaterial.h"
#include "screenMaterial.h"
#include <algorithm>

using namespace GLframework;

void Renderer::setFaceCullingState(std::shared_ptr<GLframework::Material> material)
{
	if(material->getFaceCullingState())
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(material->getFrontFace());
		glCullFace(material->getCullFace());
	}else
	{
		glDisable(GL_CULL_FACE);
	}
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


void Renderer::setColorBlendState(std::shared_ptr<GLframework::Material> material)
{
	if(material->getColorBlendState())
	{
		glEnable(GL_BLEND);
		glBlendFunc(material->getSFactor(), material->getDFactor());

	}else
	{
		glDisable(GL_BLEND);
	}
}


void Renderer::setStencilState(std::shared_ptr<Material> material)
{
	if (material->getStencilState())
	{
		glEnable(GL_STENCIL_TEST);
		glStencilOp(material->getSFail(), material->getZFail(), material->getZPass());
		glStencilMask(material->getStencilMask());
		glStencilFunc(material->getStencilFunc(), material->getStencilRef(), material->getStencilFuncMask());
	}
	else
	{
		glDisable(GL_STENCIL_TEST);
	}
}


void Renderer::setDepthState(std::shared_ptr<Material> material)
{
	if (material->getDepthTest())
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(material->getDepthFunc());
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (material->getDepthWrite())
	{
		glDepthMask(GL_TRUE);
	}
	else
	{
		glDepthMask(GL_FALSE);
	}
}

void Renderer::setPolygonOffsetState(std::shared_ptr<Material> material)
{
	if (material->getPolygonOffsetState())
	{
		glEnable(material->getPolygonOffsetType());
		// Factor表示深度斜率的倍数，unit表示深度精度的最小细分值的倍数，目的在于解决zFighting现象
		glPolygonOffset(material->getFactor(), material->getUnit());
	}
	else
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
	}
}


void Renderer::setClearColor(glm::vec3 color)
{
	glClearColor(color.r, color.g, color.b, 1.0f);
}

std::shared_ptr<Shader> Renderer::getShader(MaterialType type)
{
	return pickShader(type);
}


std::shared_ptr<Shader> Renderer::pickShader(MaterialType type)
{
	std::shared_ptr<Shader> res{nullptr};
	switch (type)
	{
	case MaterialType::PhongMaterial:
		res = mPhongShader;
		break;
	case MaterialType::WhiteMaterial:
		res = mWhiteShader;
		break;
	case MaterialType::DepthMaterial:
		res = mDepthShader;
		break;
	case MaterialType::OpacityMaskMaterial:
		res = mOpacityMaskShader;
		break;
	case MaterialType::ScreenMaterial:
		res = mScreenShader;
		break;
	case MaterialType::CubeSphereMaterial:
		res = mCubeSphereShader;
		break;
	case MaterialType::CubeMaterial:
		res = mCubeShader;
		break;
	case MaterialType::PhongEnvSphereMaterial:
		res = mPhongEnvSphereShader;
		break;
	case MaterialType::PhongEnvMaterial:
		res = mPhongEnvShader;
		break;
	case MaterialType::PhongInstanceMaterial:
		res = mPhongInstanceShader;
		break;
	default:
		std::cerr << "Unknown material type to pick shader\n";
		break;
	}
	return res;
}


void Renderer::render(
	std::shared_ptr<Scene> scene,
	Camera* camera,
	std::shared_ptr<DirectionalLight> dirLight,
	std::shared_ptr<SpotLight> spotLight,
	std::vector<std::shared_ptr<PointLight>> pointLights,
	std::shared_ptr<AmbientLight> ambient,
	unsigned int fbo
)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// 1. 深度缓冲信息
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	// 开启测试、设置基本写入状态以及开启模板测试写入
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilMask(0xff); //保证模板缓冲可清理

	// 默认颜色混合为关闭状态
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// 2. 清理画布
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// 清空两个队列
	mOpacityObjects.clear();
	mTransparentObjects.clear();

	projectObject(scene);

	std::sort(mTransparentObjects.begin(), mTransparentObjects.end(), [camera](const std::shared_ptr<Mesh>& A,const std::shared_ptr<Mesh>& B)
		{
			//	1. 计算A的相机系的Z
			auto viewMatrix = camera->getViewMatrix();

			auto modelMatrixA = A->getModleMatrix();
			auto worldPositionA = modelMatrixA * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionA = viewMatrix * worldPositionA;

			auto modelMatrixB = B->getModleMatrix();
			auto worldPositionB = modelMatrixB * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionB = viewMatrix * worldPositionB;

			return cameraPositionA.z < cameraPositionB.z;
		});

	// 3. 渲染两个队列
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
	std::vector<std::shared_ptr<PointLight>> pointLights,
	std::shared_ptr<AmbientLight> ambient
)
{
	// 1. 判断mesh类型，object不需要渲染
	if (object->getType() == ObjectType::Mesh||object->getType() == ObjectType::InstancedMesh)
	{
		// 3. 遍历mesh进行绘制
		auto mesh = std::static_pointer_cast<Mesh>(object);
		std::shared_ptr<Geometry> geometry = mesh->getGeometry();

		std::shared_ptr<Material> material;

		if(mGlobalMaterial!=nullptr)
		{
			material = mGlobalMaterial;
		}else
		{
			material = mesh->getMaterial();
		}

		auto shader = pickShader(material->getMaterialType());

		// 设置渲染状态
		// 1. 检测深度状态
		setDepthState(material);
		// 2. 检测深度偏移状态
		setPolygonOffsetState(material);
		setStencilState(material);
		// 3. 检测颜色混合状态
		setColorBlendState(material);
		// 4. 检测面剔除状态
		setFaceCullingState(material);
		// 2. 更新shader的uniform
		shader->begin();


		switch (material->getMaterialType())
		{
		case MaterialType::PhongMaterial:
			{
				std::shared_ptr<PhongMaterial> phongMat = std::static_pointer_cast<PhongMaterial>(material);

				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
				//设置整体默认透明度--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------
				
				//	设置shader的采样器为0号采样器
				//	diffuse贴图
				GL_CALL(shader->setInt("samplerGrass", 0));

				//	将纹理与纹理单元挂钩
				phongMat->mDiffuse->Bind();

				//	mask贴图
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				//	将纹理采样器与纹理单元进行挂钩
				//	mvp变化矩阵
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//法线矩阵更新，在旋转过程中法线的变化矩阵
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	spotlight光源参数更新
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight光源参数更新
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight光源参数更新
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
				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setFloat("speed", 0.5);

				//	相机信息更新
				shader->setVector3("cameraPosition", camera->mPosition);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
			}
			break;
		case MaterialType::WhiteMaterial:
			{
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				
			}
			break;
		case MaterialType::DepthMaterial:
			{
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
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
				//设置整体默认透明度--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------

				//	设置shader的采样器为0号采样器
				//	diffuse贴图
				GL_CALL(shader->setInt("samplerGrass", 0));

				//	将纹理与纹理单元挂钩
				opacityMat->mDiffuse->Bind();

				//	mask贴图
				GL_CALL(shader->setInt("opacityMaskSampler", 1));
				opacityMat->mOpacityrMask->Bind();

				//	将纹理采样器与纹理单元进行挂钩
				//	mvp变化矩阵
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//法线矩阵更新，在旋转过程中法线的变化矩阵
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	spotlight光源参数更新
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight光源参数更新
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight光源参数更新
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
				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", opacityMat->mShiness);
				shader->setFloat("speed", 0.5);

				//	相机信息更新
				shader->setVector3("cameraPosition", camera->mPosition);
				if (opacityMat->mDiffuse == nullptr)
					std::cout << "null\n";
			}
			break;
		case MaterialType::ScreenMaterial:
			{
				std::shared_ptr<ScreenMaterial> screenMaterial = std::static_pointer_cast<ScreenMaterial>(material);
				shader->setInt("screenTextureSampler", 0);
				shader->setFloat("texWidth", 1200);
				shader->setFloat("texHeight", 900);

				screenMaterial->mScreenTexture->Bind();
			}
			break;
		case MaterialType::CubeSphereMaterial:
			{
				std::shared_ptr<CubeSphereMaterial> cubeMat = std::static_pointer_cast<CubeSphereMaterial>(material);
				mesh->setPosition(camera->mPosition);
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
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
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
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
			//设置整体默认透明度--------
			GL_CALL(shader->setFloat("opacity", material->getOpacity()));
			//-----------------------

			//	设置shader的采样器为0号采样器
			//	diffuse贴图
			GL_CALL(shader->setInt("samplerGrass", 0));
			//	将纹理与纹理单元挂钩
			phongMat->mDiffuse->Bind();

			//	mask贴图
			GL_CALL(shader->setInt("MaskSampler", 1));
			phongMat->mSpecularMask->Bind();

			//	cube贴图
			GL_CALL(shader->setInt("envSampler", 2));
			phongMat->mEnv->Bind();

			//	将纹理采样器与纹理单元进行挂钩
			//	mvp变化矩阵
			shader->setMat4("modelMatrix", mesh->getModleMatrix());
			shader->setMat4("viewMatrix", camera->getViewMatrix());
			shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
			//法线矩阵更新，在旋转过程中法线的变化矩阵
			shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
			//	spotlight光源参数更新
			shader->setVector3("spotLight.position", spotLight->getPosition());
			shader->setVector3("spotLight.color", spotLight->getColor());
			shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
			shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
			shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
			shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
			//	dirlight光源参数更新
			shader->setVector3("directionalLight.color", dirLight->getColor());
			shader->setVector3("directionalLight.direction", dirLight->getDirection());
			shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

			//	pointlight光源参数更新
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
			}

			shader->setVector3("ambientColor", ambient->getColor());
			shader->setFloat("time", glfwGetTime());
			shader->setFloat("shiness", phongMat->mShiness);
			shader->setFloat("speed", 0.5);

			//	相机信息更新
			shader->setVector3("cameraPosition", camera->mPosition);

		}
			break;
		case MaterialType::PhongEnvMaterial:
			{
				std::shared_ptr<PhongEnvMaterial> phongMat = std::static_pointer_cast<PhongEnvMaterial>(material);

				if (phongMat->mDiffuse == nullptr)
					std::cout << "null diffuse\n";
				//设置整体默认透明度--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));
				//-----------------------

				//	设置shader的采样器为0号采样器
				//	diffuse贴图
				GL_CALL(shader->setInt("samplerGrass", 0));
				//	将纹理与纹理单元挂钩
				phongMat->mDiffuse->Bind();

				//	mask贴图
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();
		
				//	cube贴图
				GL_CALL(shader->setInt("envSampler", 2));
				phongMat->mEnv->Bind();

				//	将纹理采样器与纹理单元进行挂钩
				//	mvp变化矩阵
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//法线矩阵更新，在旋转过程中法线的变化矩阵
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	spotlight光源参数更新
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight光源参数更新
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight光源参数更新
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
				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setFloat("speed", 0.5);

				//	相机信息更新
				shader->setVector3("cameraPosition", camera->mPosition);

			}
			break;
		case MaterialType::PhongInstanceMaterial:
			{
				std::shared_ptr<PhongInstanceMaterial> phongMat = std::static_pointer_cast<PhongInstanceMaterial>(material);
				std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
				//设置整体默认透明度--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------

				//	设置shader的采样器为0号采样器
				//	diffuse贴图
				GL_CALL(shader->setInt("samplerGrass", 0));

				//	将纹理与纹理单元挂钩
				phongMat->mDiffuse->Bind();

				//	mask贴图
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				//	将纹理采样器与纹理单元进行挂钩
				//	mvp变化矩阵
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//法线矩阵更新，在旋转过程中法线的变化矩阵
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	spotlight光源参数更新
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight光源参数更新
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight光源参数更新
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
				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setFloat("speed", 0.5);

				//	相机信息更新
				shader->setVector3("cameraPosition", camera->mPosition);

				//传输uniform类型矩阵变换数组
				if(im->getMatricesUpdateState())
				{
					shader->setMat4Array("matrices",im->mInstanceMatrices,im->getInstanceCount());
					shader->setInt("matricesUpdateState", 1);
					//std::cout << "The matrix update as UNIFORM way" << std::endl;
				}else
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
		// 3. 绑定vao
		glBindVertexArray(geometry->getVao());
		// 4. 执行绘制命令
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if(object->getType()==ObjectType::Mesh)
			glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, static_cast<void*>(nullptr));
		else
		{
			std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
			glDrawElementsInstanced(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, 0, im->getInstanceCount());
		}
		GL_CALL(glBindVertexArray(0));
		shader->end();
	}

}

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
#include "materials/grassInstanceMaterial/grassInstanceMaterial.h"
#include "materials/phongParallaxMaterial/phongParallaxMaterial.h"
#include "cubeSphereMaterial.h"
#include "materials/phongNormalMaterial/phongNormalMaterial.h"
#include "materials/phongShadowMaterial/phongShadowMaterial.h"
#include "light/shadow/directionalLightShadow/directionalLightShadow.h"
#include "orthographiccamera.h"
#include "../mesh/instancedMesh.h"
#include "cubeMaterial.h"
#include "screenMaterial.h"
#include <algorithm>

using namespace GLframework;


void Renderer::renderShadowMap(const std::vector<std::shared_ptr<Mesh>>& meshes, std::shared_ptr<DirectionalLight> dirLight, std::shared_ptr<Framebuffer> fbo)
{
	//	1. make sure that the current draw is not a postProcessPass draw, if it is, then the render is not performed
	bool isPostProcessPass = true;
	for(auto& mesh : meshes)
	{
		if(mesh->getMaterial()->getMaterialType()!=GLframework::MaterialType::ScreenMaterial)
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

	glBindFramebuffer(GL_FRAMEBUFFER, fbo->getFBO());
	glViewport(0, 0, fbo->getWidth(), fbo->getHeight());


	//	4. start drawing
	glClear(GL_DEPTH_BUFFER_BIT);
	auto lightMatrix = std::static_pointer_cast<DirectionalLightShadow>( dirLight->getShadow())->getLightMatrix(dirLight->getModelMatrix());
	mShadowShader->begin();
	mShadowShader->setMat4("lightMatrix", lightMatrix);
	for(auto& mesh : meshes)
	{
		glBindVertexArray(mesh->getGeometry()->getVao());
		mShadowShader->setMat4("modelMatrix", mesh->getModelMatrix());

		if (mesh->getType() == ObjectType::InstancedMesh)
		{
			std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
			glDrawElementsInstanced(GL_TRIANGLES, mesh->getGeometry()->getIndicesCount(), GL_UNSIGNED_INT, 0, im->getInstanceCount());
		}
		else
		{
			glDrawElements(GL_TRIANGLES, mesh->getGeometry()->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
		}
	}
	mShadowShader->end();

	glBindFramebuffer(GL_FRAMEBUFFER, preFbo);
	glViewport(preViewPort[0], preViewPort[1], preViewPort[2], preViewPort[3]);
}

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
		// Factor��ʾ���б�ʵı�����unit��ʾ��Ⱦ��ȵ���Сϸ��ֵ�ı�����Ŀ�����ڽ��zFighting����
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
	case MaterialType::GrassInstanceMaterial:
		res = mGrassInstanceShader;
		break;
	case MaterialType::PhongNormalMaterial:
		res = mPhongNormalShader;
		break;
	case MaterialType::PhongParallaxMaterial:
		res = mPhongParallaxShader;
		break;
	case MaterialType::PhongShadowMaterial:
		res = mPhongShadowShader;
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

	// 1. 设置当前帧绘制的时候，opengl的必要状态机参数
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	// 开启测试、设置基本写入状态，打开模板测试写入
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilMask(0xff); //保证了模板缓冲可以被清理

	// 默认颜色混合
	glDisable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// 2. 清理画布 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// 清空两个队列
	mOpacityObjects.clear();
	mTransparentObjects.clear();

	projectObject(scene);

	std::sort(mTransparentObjects.begin(), mTransparentObjects.end(), [camera](const std::shared_ptr<Mesh>& A,const std::shared_ptr<Mesh>& B)
		{
			//	1. 计算a的相机系的Z
			auto viewMatrix = camera->getViewMatrix();

			auto modelMatrixA = A->getModelMatrix();
			auto worldPositionA = modelMatrixA * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionA = viewMatrix * worldPositionA;

			//2 计算b的相机系的Z
			auto modelMatrixB = B->getModelMatrix();
			auto worldPositionB = modelMatrixB * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionB = viewMatrix * worldPositionB;

			return cameraPositionA.z < cameraPositionB.z;
		});

	//	render shadowmap
	renderShadowMap(mOpacityObjects, dirLight,dirLight->getShadow()->mRenderTarget);

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
	//判断是Mesh还是Object，如果是Mesh需要渲染
	if (object->getType() == ObjectType::Mesh||object->getType() == ObjectType::InstancedMesh)
	{
		auto mesh = std::static_pointer_cast<Mesh>(object);
		std::shared_ptr<Geometry> geometry = mesh->getGeometry();

		std::shared_ptr<Material> material = nullptr;
		//考察是否拥有全局材质
		if(mGlobalMaterial!=nullptr)
		{
			material = mGlobalMaterial;
		}else
		{
			material = mesh->getMaterial();
		}

		

		//设置渲染状态
		setDepthState(material);
		setPolygonOffsetState(material);
		setStencilState(material);
		setColorBlendState(material);
		setFaceCullingState(material);
		auto shader = pickShader(material->getMaterialType());
		shader->begin();


		switch (material->getMaterialType())
		{
		case MaterialType::PhongMaterial:
			{
				std::shared_ptr<PhongMaterial> phongMat = std::static_pointer_cast<PhongMaterial>(material);

				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
				//��������Ĭ��͸����--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------
				
				//	����shader�Ĳ�����Ϊ0�Ų�����
				//	diffuse��ͼ
				
				GL_CALL(shader->setInt("samplerGrass", phongMat->mDiffuse->getUnit()));

				//	�������������Ԫ�ҹ�
				phongMat->mDiffuse->Bind();
				
				//	mask��ͼ
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				//	������������������Ԫ���йҹ�
				//	mvp�仯����
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight��Դ��������
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight��Դ��������
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
				shader->setFloat("directionalLight.intensity", dirLight->getIntensity());
				//	pointlight��Դ��������
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

				//	�����Ϣ����
				shader->setVector3("cameraPosition", camera->mPosition);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
			}
			break;
		case MaterialType::PhongShadowMaterial:
			{
				std::shared_ptr<PhongShadowMaterial> phongMat = std::static_pointer_cast<PhongShadowMaterial>(material);
				std::shared_ptr<DirectionalLightShadow> dirShadow = std::static_pointer_cast<DirectionalLightShadow>(dirLight->getShadow());
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
				//��������Ĭ��͸����--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------

		
				//	diffuse
				GL_CALL(shader->setInt("samplerGrass", phongMat->mDiffuse->getUnit()));
				phongMat->mDiffuse->Bind();

				//	mask��ͼ
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				GL_CALL(shader->setInt("shadowMapSampler", 2));
				dirShadow-> mRenderTarget->getDepthAttachment()->setUnit(2);
				dirShadow->mRenderTarget->getDepthAttachment()->Bind();

				shader->setMat4("lightMatrix", dirShadow->getLightMatrix(dirLight->getModelMatrix()));
				shader->setFloat("bias", dirShadow->mBias);
				shader->setFloat("diskTightness", dirShadow->mDiskTightness);
				shader->setFloat("pcfRadius", dirShadow->mPcfRadius);

				//	������������������Ԫ���йҹ�
				//	mvp�仯����
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight��Դ��������
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight��Դ��������
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
				shader->setFloat("directionalLight.intensity", dirLight->getIntensity());
				//	pointlight��Դ��������
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

				//	�����Ϣ����
				shader->setVector3("cameraPosition", camera->mPosition);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";

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
				//��������Ĭ��͸����--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------

				//	����shader�Ĳ�����Ϊ0�Ų�����
				//	diffuse��ͼ
				GL_CALL(shader->setInt("samplerGrass", 0));

				//	�������������Ԫ�ҹ�
				opacityMat->mDiffuse->Bind();

				//	mask��ͼ
				GL_CALL(shader->setInt("opacityMaskSampler", 1));
				opacityMat->mOpacityrMask->Bind();

				//	������������������Ԫ���йҹ�
				//	mvp�仯����
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight��Դ��������
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight��Դ��������
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight��Դ��������
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

				//	�����Ϣ����
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
			//��������Ĭ��͸����--------
			GL_CALL(shader->setFloat("opacity", material->getOpacity()));
			//-----------------------

			//	����shader�Ĳ�����Ϊ0�Ų�����
			//	diffuse��ͼ
			GL_CALL(shader->setInt("samplerGrass", 0));
			//	�������������Ԫ�ҹ�
			phongMat->mDiffuse->Bind();

			//	mask��ͼ
			GL_CALL(shader->setInt("MaskSampler", 1));
			phongMat->mSpecularMask->Bind();

			//	cube��ͼ
			GL_CALL(shader->setInt("envSampler", 2));
			phongMat->mEnv->Bind();

			//	������������������Ԫ���йҹ�
			//	mvp�仯����
			shader->setMat4("modelMatrix", mesh->getModelMatrix());
			shader->setMat4("viewMatrix", camera->getViewMatrix());
			shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
			//���߾�����£�����ת�����з��ߵı仯����
			shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
			//	spotlight��Դ��������
			shader->setVector3("spotLight.position", spotLight->getPosition());
			shader->setVector3("spotLight.color", spotLight->getColor());
			shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
			shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
			shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
			shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
			//	dirlight��Դ��������
			shader->setVector3("directionalLight.color", dirLight->getColor());
			shader->setVector3("directionalLight.direction", dirLight->getDirection());
			shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

			//	pointlight��Դ��������
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

			//	�����Ϣ����
			shader->setVector3("cameraPosition", camera->mPosition);

		}
			break;
		case MaterialType::PhongEnvMaterial:
			{
				std::shared_ptr<PhongEnvMaterial> phongMat = std::static_pointer_cast<PhongEnvMaterial>(material);

				if (phongMat->mDiffuse == nullptr)
					std::cout << "null diffuse\n";
				//��������Ĭ��͸����--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));
				//-----------------------

				//	����shader�Ĳ�����Ϊ0�Ų�����
				//	diffuse��ͼ
				GL_CALL(shader->setInt("samplerGrass", 0));
				//	�������������Ԫ�ҹ�
				phongMat->mDiffuse->Bind();

				//	mask��ͼ
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();
		
				//	cube��ͼ
				GL_CALL(shader->setInt("envSampler", 2));
				phongMat->mEnv->Bind();

				//	������������������Ԫ���йҹ�
				//	mvp�仯����
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight��Դ��������
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight��Դ��������
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight��Դ��������
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

				//	�����Ϣ����
				shader->setVector3("cameraPosition", camera->mPosition);

			}
			break;
		case MaterialType::PhongInstanceMaterial:
			{
				std::shared_ptr<PhongInstanceMaterial> phongMat = std::static_pointer_cast<PhongInstanceMaterial>(material);
				std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
				//��������Ĭ��͸����--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));


				//-----------------------



				//	����shader�Ĳ�����Ϊ0�Ų�����
				//	diffuse��ͼ
				GL_CALL(shader->setInt("samplerGrass", 0));

				//	�������������Ԫ�ҹ�
				phongMat->mDiffuse->Bind();

				//	mask��ͼ
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				//	������������������Ԫ���йҹ�
				//	mvp变换矩阵
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//计算并传输法线矩阵 实例绘制中只能在GPU端进行计算
				//shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight数据传入
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight数据传入
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight数据传入
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

				//	�����Ϣ����
				shader->setVector3("cameraPosition", camera->mPosition);

				//����uniform���;���任����
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
				//设置透明度--------
				GL_CALL(shader->setFloat("opacity", material->getOpacity()));
				//-----------------------

				//	����shader�Ĳ�����Ϊ0�Ų�����
				//	diffuse贴图绑定1
				GL_CALL(shader->setInt("samplerGrass", 0));
				instance_material->mDiffuse->Bind();

				//	specularMask绑定
				GL_CALL(shader->setInt("MaskSampler", 1));
				instance_material->mSpecularMask->Bind();

				//	opacityMask绑定
				GL_CALL(shader->setInt("opacityMask", 2));
				instance_material->mOpacityMask->Bind();

				//	couldMask绑定
				GL_CALL(shader->setInt("cloudMask", 3));
				instance_material->mCloudMask->Bind();
				//	������������������Ԫ���йҹ�
				//	mvp矩阵变换
				shader->setMat4("modelMatrix", mesh->getModelMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModelMatrix()))));
				//	spotlight��Դ��������
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight��Դ��������
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
				


				//	pointlight��Դ��������
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
				shader->setFloat("shiness", instance_material->mShiness);
				shader->setFloat("speed", 0.5);

				//	�����Ϣ����
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
				std::shared_ptr<PhongNormalMaterial> phongMat = std::static_pointer_cast<PhongNormalMaterial>(material);

				//��������Ĭ��͸����--------
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
				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setFloat("speed", 0.5);

				shader->setVector3("cameraPosition", camera->mPosition);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "mDiffuse null\n";
				if (phongMat->mNormal == nullptr)
					std::cout << "mNormal null\n";
		}
			break;
		case MaterialType::PhongParallaxMaterial:
			{
				std::shared_ptr<PhongParallaxMaterial> phongMat = std::static_pointer_cast<PhongParallaxMaterial>(material);

				//��������Ĭ��͸����--------
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
				}

				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setFloat("speed", 0.5);

				shader->setVector3("cameraPosition", camera->mPosition);
			}
			break;
		default:
			std::cout << "wrong\n";
			break;
		}
		// 3. ��vao
		glBindVertexArray(geometry->getVao());
		// 4. 选择geometry的绘画方式（单例绘画/实力绘画）
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

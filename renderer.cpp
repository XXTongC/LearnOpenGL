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
#include "grassInstanceMaterial/grassInstanceMaterial.h"
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

	// 1. ��Ȼ�����Ϣ
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	// ������ԡ����û���д��״̬�Լ�����ģ�����д��
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilMask(0xff); //��֤ģ�建�������

	// Ĭ����ɫ���Ϊ�ر�״̬
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// 2. �������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// �����������
	mOpacityObjects.clear();
	mTransparentObjects.clear();

	projectObject(scene);

	std::sort(mTransparentObjects.begin(), mTransparentObjects.end(), [camera](const std::shared_ptr<Mesh>& A,const std::shared_ptr<Mesh>& B)
		{
			//	1. ����A�����ϵ��Z
			auto viewMatrix = camera->getViewMatrix();

			auto modelMatrixA = A->getModleMatrix();
			auto worldPositionA = modelMatrixA * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionA = viewMatrix * worldPositionA;

			auto modelMatrixB = B->getModleMatrix();
			auto worldPositionB = modelMatrixB * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			auto cameraPositionB = viewMatrix * worldPositionB;

			return cameraPositionA.z < cameraPositionB.z;
		});

	// 3. ��Ⱦ��������
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
	// 1. �ж�mesh���ͣ�object����Ҫ��Ⱦ
	if (object->getType() == ObjectType::Mesh||object->getType() == ObjectType::InstancedMesh)
	{
		// 3. ����mesh���л���
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

		// ������Ⱦ״̬
		// 1. ������״̬
		setDepthState(material);
		// 2. ������ƫ��״̬
		setPolygonOffsetState(material);
		setStencilState(material);
		// 3. �����ɫ���״̬
		setColorBlendState(material);
		// 4. ������޳�״̬
		setFaceCullingState(material);
		// 2. ����shader��uniform
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
				GL_CALL(shader->setInt("samplerGrass", 0));

				//	�������������Ԫ�ҹ�
				phongMat->mDiffuse->Bind();

				//	mask��ͼ
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				//	������������������Ԫ���йҹ�
				//	mvp�仯����
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	spotlight��Դ��������
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
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
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	spotlight��Դ��������
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
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
			shader->setMat4("modelMatrix", mesh->getModleMatrix());
			shader->setMat4("viewMatrix", camera->getViewMatrix());
			shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
			//���߾�����£�����ת�����з��ߵı仯����
			shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
			//	spotlight��Դ��������
			shader->setVector3("spotLight.position", spotLight->getPosition());
			shader->setVector3("spotLight.color", spotLight->getColor());
			shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
			shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
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
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	spotlight��Դ��������
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
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
				//	mvp�仯����
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	spotlight��Դ��������
				shader->setVector3("spotLight.position", spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
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

				//����uniform���;���任����
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
		case MaterialType::GrassInstanceMaterial:
		{
			std::shared_ptr<GrassInstanceMaterial> grassMat = std::static_pointer_cast<GrassInstanceMaterial>(material);
			std::shared_ptr<InstancedMesh> im = std::static_pointer_cast<InstancedMesh>(mesh);
			if (grassMat->mDiffuse == nullptr)
				std::cout << "null\n";
			//��������Ĭ��͸����--------
			GL_CALL(shader->setFloat("opacity", material->getOpacity()));


			//-----------------------

			//	����shader�Ĳ�����Ϊ0�Ų�����
			//	diffuse��ͼ
			GL_CALL(shader->setInt("samplerGrass", 0));

			//	�������������Ԫ�ҹ�
			grassMat->mDiffuse->Bind();

			//	mask��ͼ
			GL_CALL(shader->setInt("MaskSampler", 1));
			grassMat->mSpecularMask->Bind();

			//	������������������Ԫ���йҹ�
			//	mvp�仯����
			shader->setMat4("modelMatrix", mesh->getModleMatrix());
			shader->setMat4("viewMatrix", camera->getViewMatrix());
			shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
			//���߾�����£�����ת�����з��ߵı仯����
			shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
			//	spotlight��Դ��������
			shader->setVector3("spotLight.position", spotLight->getPosition());
			shader->setVector3("spotLight.color", spotLight->getColor());
			shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
			shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
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
			shader->setFloat("shiness", grassMat->mShiness);
			shader->setFloat("speed", 0.5);

			//	�����Ϣ����
			shader->setVector3("cameraPosition", camera->mPosition);

			//����uniform���;���任����
			if (im->getMatricesUpdateState())
			{
				shader->setMat4Array("matrices", im->mInstanceMatrices, im->getInstanceCount());
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
		// 3. ��vao
		glBindVertexArray(geometry->getVao());
		// 4. ִ�л�������
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

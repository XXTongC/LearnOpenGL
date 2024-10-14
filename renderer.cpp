#include "renderer.h"
#include <iostream>
#include <memory>
#include <string>
#include "phongMaterial.h"
#include "whiteMaterial.h"


using namespace GLframework;

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
	default:
		std::cerr << "Unknown material type to pick shader\n";
		break;
	}
	return res;
}


void  Renderer::render(
	std::shared_ptr < GLframework::Scene> scene,
	Camera* camera,
	std::shared_ptr < GLframework::DirectionalLight> dirLight,
	std::shared_ptr < GLframework::SpotLight> spotLight,
	std::vector<std::shared_ptr<GLframework::PointLight>> pointLights,
	std::shared_ptr <GLframework::AmbientLight> ambient
)
{
	// 1. ��Ȼ�����Ϣ
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	// 2. ������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// 3. ��sceneΪ���ڵ㿪ʼ�ݹ���Ⱦ
	renderObject(scene, camera, dirLight, spotLight, pointLights, ambient);
}


void Renderer::renderObject(
	std::shared_ptr<Object> object,
	Camera* camera,
	std::shared_ptr < GLframework::DirectionalLight> dirLight,
	std::shared_ptr < GLframework::SpotLight> spotLight,
	std::vector<std::shared_ptr<GLframework::PointLight>> pointLights,
	std::shared_ptr <GLframework::AmbientLight> ambient
)
{
	// 1. �ж�mesh���ͣ�object����Ҫ��Ⱦ
	if(object->getType()==ObjectType::Mesh)
	{
		// 3. ����mesh���л���
		auto mesh = std::static_pointer_cast<Mesh>(object);
			std::shared_ptr<Geometry> geometry = mesh->getGeometry();

			std::shared_ptr<Material> material = mesh->getMaterial();

			auto shader = pickShader(material->getMaterialType());

			// ������Ⱦ״̬
			// 1. ������״̬
			if(material->getDepthTest())
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(material->getDepthFunc());
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
			}

			if(material->getDepthWrite())
			{
				glDepthMask(GL_TRUE);
			}else
			{
				glDepthMask(GL_FALSE);
			}
			// 2. ������ƫ��״̬
			if(material->getPolygonOffsetState())
			{
				glEnable(material->getPolygonOffsetType());
				// Factor��ʾ���б�ʵı�����unit��ʾ��Ⱦ��ȵ���Сϸ��ֵ�ı�����Ŀ�����ڽ��zFighting����
				glPolygonOffset(material->getFactor(), material->getUnit());
			}else
			{
				glDisable(GL_POLYGON_OFFSET_FILL);
				glDisable(GL_POLYGON_OFFSET_LINE);
			}
			
			
			// 2. ����shader��uniform
			shader->begin();


			switch (material->getMaterialType())
			{
			case MaterialType::PhongMaterial:
			{
				std::shared_ptr<PhongMaterial> phongMat = std::static_pointer_cast<PhongMaterial>(material);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";

				//std::cout << phongMat->mDiffuse << "\n";
			//	����shader�Ĳ�����Ϊ0�Ų�����
			//	diffuse��ͼ
				GL_CALL(shader->setInt("samplerGrass", 0));

				//	������������Ԫ�ҹ�
				phongMat->mDiffuse->Bind();

				//	mask��ͼ
				GL_CALL(shader->setInt("MaskSampler", 1));
				phongMat->mSpecularMask->Bind();

				//	�����������������Ԫ���йҹ�
				//	mvp�仯����
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", glm::transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
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
				break;
			}
			case MaterialType::DepthMaterial:
			{
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				shader->setFloat("near", camera->mNear);
				shader->setFloat("far", camera->mFar);
				break;
			}
			default:
				std::cout << "wrong\n";
				break;
			}
			// 3. ��vao
			glBindVertexArray(geometry->getVao());
			// 4. ִ�л�������
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, static_cast<void*>(0));

			//GL_CALL(glBindVertexArray(0));
			//shader->end();
		
	}

	// 2. ����object���ӽڵ㣬��ÿ���ӽڵ����renderObject����
	
	for(auto& t : object->getChildren())
	{
		renderObject(t, camera, dirLight,spotLight,pointLights, ambient);
	}

}
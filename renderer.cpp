#include "renderer.h"
#include <iostream>
#include <memory>
#include "phongMaterial.h"
using namespace GLframework;

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
	default:
		std::cerr << "Unknown material type to pick shader\n";
		break;
	}
	return res;
}


void Renderer::render(
	const std::vector<std::shared_ptr < GLframework::Mesh>>& meshes,
	Camera* camera,
	std::shared_ptr < GLframework::DirectionalLight> dirLight,
	std::shared_ptr <GLframework::AmbientLight> ambient
)
{
	// 1. ���û���֡opengl�ı�Ҫ״̬������
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// 2. ������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// 3. ����mesh���л���
	for (auto& mesh : meshes)
	{
		
		std::shared_ptr<Geometry> geometry = mesh->getGeometry();

		std::shared_ptr<Material> material = mesh->getMaterial();

		auto shader = pickShader(material->getMaterialType());

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
				shader->setInt("samplerLand", 1);
				shader->setInt("samplerNoise", 2);
				//	������������Ԫ�ҹ�
				phongMat->mDiffuse->Bind();
				
				//	�����������������Ԫ���йҹ�
				//	mvp�仯����
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//���߾�����£�����ת�����з��ߵı仯����
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	��Դ��������
				shader->setVector3("lightDirection", dirLight->getDirection());
				shader->setVector3("lightColor", dirLight->getColor());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("specularIntensity", dirLight->getSpecularIntensity());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("speed", 0.5);
				//	�����Ϣ����
				shader->setVector3("cameraPosition", camera->mPosition);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
			}
			break;
		default:
			std::cout << "wrong\n";
			continue;
		}


		// 3. ��vao
		glBindVertexArray(geometry->getVao());
		// 4. ִ�л�������
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, static_cast<void*>(0));

		//GL_CALL(glBindVertexArray(0));
		//shader->end();
	
	}

}

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
	// 1. 设置绘制帧opengl的必要状态机参数
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// 2. 清理画布
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// 3. 遍历mesh进行绘制
	for (auto& mesh : meshes)
	{
		
		std::shared_ptr<Geometry> geometry = mesh->getGeometry();

		std::shared_ptr<Material> material = mesh->getMaterial();

		auto shader = pickShader(material->getMaterialType());

		// 2. 更新shader的uniform
		shader->begin();
	

		switch (material->getMaterialType())
		{
		case MaterialType::PhongMaterial:
			{
				std::shared_ptr<PhongMaterial> phongMat = std::static_pointer_cast<PhongMaterial>(material);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
			
					//std::cout << phongMat->mDiffuse << "\n";
				//	设置shader的采样器为0号采样器
				//	diffuse贴图
				GL_CALL(shader->setInt("samplerGrass", 0));
				shader->setInt("samplerLand", 1);
				shader->setInt("samplerNoise", 2);
				//	将纹理与纹理单元挂钩
				phongMat->mDiffuse->Bind();
				
				//	将纹理采样器与纹理单元进行挂钩
				//	mvp变化矩阵
				shader->setMat4("modelMatrix", mesh->getModleMatrix());
				shader->setMat4("viewMatrix", camera->getViewMatrix());
				shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
				//法线矩阵更新，在旋转过程中法线的变化矩阵
				shader->setMat3("normalMatrix", transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
				//	光源参数更新
				shader->setVector3("lightDirection", dirLight->getDirection());
				shader->setVector3("lightColor", dirLight->getColor());
				shader->setFloat("shiness", phongMat->mShiness);
				shader->setVector3("ambientColor", ambient->getColor());
				shader->setFloat("specularIntensity", dirLight->getSpecularIntensity());
				shader->setFloat("time", glfwGetTime());
				shader->setFloat("speed", 0.5);
				//	相机信息更新
				shader->setVector3("cameraPosition", camera->mPosition);
				if (phongMat->mDiffuse == nullptr)
					std::cout << "null\n";
			}
			break;
		default:
			std::cout << "wrong\n";
			continue;
		}


		// 3. 绑定vao
		glBindVertexArray(geometry->getVao());
		// 4. 执行绘制命令
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, static_cast<void*>(0));

		//GL_CALL(glBindVertexArray(0));
		//shader->end();
	
	}

}

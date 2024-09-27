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
	default:
		std::cerr << "Unknown material type to pick shader\n";
		break;
	}
	return res;
}


void  Renderer::render(
	const std::vector<std::shared_ptr < GLframework::Mesh>>& meshes,
	Camera* camera,
	std::shared_ptr < GLframework::DirectionalLight> dirLight,
	std::shared_ptr < GLframework::SpotLight> spotLight,
	std::vector<std::shared_ptr<GLframework::PointLight>> pointLights,
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
				shader->setVector3("spotLight.position",spotLight->getPosition());
				shader->setVector3("spotLight.color", spotLight->getColor());
				shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
				shader->setVector3("spotLight.targetDirection", spotLight->getTargetDirection());
				shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
				shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));
				//	dirlight光源参数更新
				shader->setVector3("directionalLight.color", dirLight->getColor());
				shader->setVector3("directionalLight.direction", dirLight->getDirection());
				shader->setFloat(  "directionalLight.specularIntensity", dirLight->getSpecularIntensity());

				//	pointlight光源参数更新
				//std::cout << pointLights.size()<<std::endl;
				for(int  i = 0;i<pointLights.size();i++)
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
				break;
			}
		default:
			std::cout << "wrong\n";
			continue;
		}


		// 3. 绑定vao
		glBindVertexArray(geometry->getVao());
		// 4. 执行绘制命令
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, static_cast<void*>(0));

		GL_CALL(glBindVertexArray(0));
		shader->end();
	
	}

}

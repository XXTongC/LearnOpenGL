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
	// 1. 深度缓冲信息
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	// 2. 清理画布
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// 3. 以scene为根节点开始递归渲染
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
	// 1. 判断mesh类型，object不需要渲染
	if(object->getType()==ObjectType::Mesh)
	{
		// 3. 遍历mesh进行绘制
		auto mesh = std::static_pointer_cast<Mesh>(object);
			std::shared_ptr<Geometry> geometry = mesh->getGeometry();

			std::shared_ptr<Material> material = mesh->getMaterial();

			auto shader = pickShader(material->getMaterialType());

			// 设置渲染状态
			// 1. 检测深度状态
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
			// 2. 检测深度偏移状态
			if(material->getPolygonOffsetState())
			{
				glEnable(material->getPolygonOffsetType());
				// Factor表示深度斜率的倍数，unit表示深度精度的最小细分值的倍数，目的在于解决zFighting现象
				glPolygonOffset(material->getFactor(), material->getUnit());
			}else
			{
				glDisable(GL_POLYGON_OFFSET_FILL);
				glDisable(GL_POLYGON_OFFSET_LINE);
			}
			
			
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
				shader->setMat3("normalMatrix", glm::transpose(inverse(glm::mat3(mesh->getModleMatrix()))));
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
			// 3. 绑定vao
			glBindVertexArray(geometry->getVao());
			// 4. 执行绘制命令
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, static_cast<void*>(0));

			//GL_CALL(glBindVertexArray(0));
			//shader->end();
		
	}

	// 2. 遍历object的子节点，对每个子节点调用renderObject函数
	
	for(auto& t : object->getChildren())
	{
		renderObject(t, camera, dirLight,spotLight,pointLights, ambient);
	}

}
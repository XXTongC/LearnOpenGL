#pragma once
#include "core.h"
#include <vector>
#include "mesh.h"
#include "camera.h"
#include "shader.h"
#include "directionalLight.h"
#include "ambientLight.h"
#include "pointLight.h"
#include "spotLight.h"
#include "scene.h"
namespace GLframework
{
	class Renderer
	{
	public:
		Renderer()
		{
			mPhongShader = { std::make_shared<Shader>("phongPointLight.vert","phong.frag")};
			mWhiteShader = { std::make_shared<Shader>("white.vert","white.frag") };
		}
		~Renderer(){}
		std::shared_ptr<Shader> getShader(MaterialType type);
		//��Ⱦ���ܺ���
		//1. ÿ�ε�����Ⱦһ֡
		
		void render(
			std::shared_ptr < GLframework::Scene> scene,
			Camera* camera,
			std::shared_ptr < GLframework::DirectionalLight> dirLight,
			std::shared_ptr < GLframework::SpotLight> spotLight,
			std::vector<std::shared_ptr<GLframework::PointLight>> pointLights,
			std::shared_ptr <GLframework::AmbientLight> ambient
		);
		void setClearColor(glm::vec3 color);
		
	private:
		//����Material���Ͳ�ͬ����ѡ��ͬ��shader
		std::shared_ptr<Shader> pickShader(MaterialType type);
		void renderObject(
			std::shared_ptr<Object> object,
			Camera* camera,
			std::shared_ptr < GLframework::DirectionalLight> dirLight,
			std::shared_ptr < GLframework::SpotLight> spotLight,
			std::vector<std::shared_ptr<GLframework::PointLight>> pointLights,
			std::shared_ptr <GLframework::AmbientLight> ambient
		);
	private:
		//���ɶ��ֲ�ͬ��shader����
		//���ݲ������͵Ĳ�ͬ����ѡѡ���ĸ�shader����
		std::shared_ptr<Shader> mPhongShader{ nullptr };
		std::shared_ptr<Shader> mWhiteShader{ nullptr };
	};
}

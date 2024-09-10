#pragma once
#include "core.h"
#include <vector>
#include "mesh.h"
#include "camera.h"
#include "shader.h"
#include "directionalLight.h"
#include "ambientLight.h"
namespace GLframework
{
	class Renderer
	{
	public:
		Renderer()
		{
			mPhongShader = { std::make_shared<Shader>("phong.vert","phong.frag")};
		}
		~Renderer(){}
		std::shared_ptr<Shader> getShader(MaterialType type);
		//��Ⱦ���ܺ���
		//1. ÿ�ε�����Ⱦһ֡
		void render(
			const std::vector<std::shared_ptr < GLframework::Mesh>>& meshes,
			Camera* camera,
			std::shared_ptr < GLframework::DirectionalLight> dirLight,
			std::shared_ptr <GLframework::AmbientLight> ambient
		);
	private:
		//����Material���Ͳ�ͬ����ѡ��ͬ��shader
		std::shared_ptr<Shader> pickShader(MaterialType type);

	private:
		//���ɶ��ֲ�ͬ��shader����
		//���ݲ������͵Ĳ�ͬ����ѡѡ���ĸ�shader����
		std::shared_ptr<Shader> mPhongShader{ nullptr };
	};
}

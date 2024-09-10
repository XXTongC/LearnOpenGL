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
		//渲染功能函数
		//1. 每次调用渲染一帧
		void render(
			const std::vector<std::shared_ptr < GLframework::Mesh>>& meshes,
			Camera* camera,
			std::shared_ptr < GLframework::DirectionalLight> dirLight,
			std::shared_ptr <GLframework::AmbientLight> ambient
		);
	private:
		//根据Material类型不同，挑选不同的shader
		std::shared_ptr<Shader> pickShader(MaterialType type);

	private:
		//生成多种不同的shader对象
		//根据材质类型的不同，挑选选择哪个shader对象
		std::shared_ptr<Shader> mPhongShader{ nullptr };
	};
}

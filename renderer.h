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
			mPhongShader =  std::make_shared<Shader>("phongPointLight.vert","phong.frag");
			mWhiteShader =  std::make_shared<Shader>("white.vert","white.frag");
			mDepthShader = std::make_shared<Shader>("depth.vert", "depth.frag");
			mOpacityMaskShader = std::make_shared<Shader>("phongOpacityMask.vert", "phongOpacityMask.frag");
		}
		~Renderer(){}
		std::shared_ptr<Shader> getShader(MaterialType type);
		//渲染功能函数
		//1. 每次调用渲染一帧
		
		void render(
			std::shared_ptr < GLframework::Scene> scene,
			Camera* camera,
			std::shared_ptr < GLframework::DirectionalLight> dirLight,
			std::shared_ptr < GLframework::SpotLight> spotLight,
			std::vector<std::shared_ptr<GLframework::PointLight>> pointLights,
			std::shared_ptr <GLframework::AmbientLight> ambient
		);
		void setClearColor(glm::vec3 color);

	public:
		std::shared_ptr<Material> mGlobalMaterial{nullptr};
	private:
		//根据Material类型不同，挑选不同的shader
		std::shared_ptr<Shader> pickShader(MaterialType type);
		void renderObject(
			std::shared_ptr<Object> object,
			Camera* camera,
			std::shared_ptr < GLframework::DirectionalLight> dirLight,
			std::shared_ptr < GLframework::SpotLight> spotLight,
			std::vector<std::shared_ptr<GLframework::PointLight>> pointLights,
			std::shared_ptr <GLframework::AmbientLight> ambient
		);
		void setDepthState(std::shared_ptr<GLframework::Material> material);
		void setPolygonOffsetState(std::shared_ptr<GLframework::Material> material);
		void setStencilState(std::shared_ptr<GLframework::Material> material);
		void setColorBlendState(std::shared_ptr<GLframework::Material> material);
		void projectObject(std::shared_ptr<Object> obj);
	private:
		//生成多种不同的shader对象
		//根据材质类型的不同，挑选选择哪个shader对象
		std::shared_ptr<Shader> mPhongShader{ nullptr };
		std::shared_ptr<Shader> mWhiteShader{ nullptr };
		std::shared_ptr<Shader> mDepthShader{ nullptr };
		std::shared_ptr<Shader> mOpacityMaskShader{ nullptr };

		//不透明队列与透明队列
		//ops: 每一帧绘制前需要清空两个队列
		std::vector<std::shared_ptr<Mesh>> mOpacityObjects;
		std::vector<std::shared_ptr<Mesh>> mTransparentObjects;

	};
}

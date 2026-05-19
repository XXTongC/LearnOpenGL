#pragma once
#include "core.h"
#include <vector>
#include "mesh/mesh.h"
#include "framebuffer.h"
#include "camera.h"
#include "shader.h"
#include "ShadowRenderer.h"
#include "ShaderLibrary.h"
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
		Renderer();
		~Renderer() = default;
		std::shared_ptr<Shader> getShader(MaterialType type);
		//渲染功能函数
		//1. 每次调用渲染一帧
		
		void render(
			std::shared_ptr < GLframework::Scene> scene,
			Camera* camera,
			std::shared_ptr < GLframework::DirectionalLight> dirLight,
			std::shared_ptr < GLframework::SpotLight> spotLight,
			const std::vector<std::shared_ptr<GLframework::PointLight>>& pointLights,
			std::shared_ptr <GLframework::AmbientLight> ambient,
			unsigned int fbo = 0
		);
		void setClearColor(glm::vec3 color);
		void msaaResolve(std::shared_ptr<Framebuffer> src, std::shared_ptr<Framebuffer> dst);
	public:
		std::shared_ptr<Material> mGlobalMaterial{nullptr};
private:
		void renderObject(
			std::shared_ptr<Object> object,
			Camera* camera,
			std::shared_ptr < GLframework::DirectionalLight> dirLight,
			std::shared_ptr < GLframework::SpotLight> spotLight,
			const std::vector<std::shared_ptr<GLframework::PointLight>>& pointLights,
			std::shared_ptr <GLframework::AmbientLight> ambient
		);
		void projectObject(std::shared_ptr<Object> obj);
private:
		void drawMesh(std::shared_ptr<Mesh> mesh);

		ShaderLibrary mShaderLibrary{};
		ShadowRenderer mShadowRenderer{};
		
		//不透明队列与透明队列
		//ops: 每一帧绘制前需要清空两个队列
		std::vector<std::shared_ptr<Mesh>> mOpacityObjects;
		std::vector<std::shared_ptr<Mesh>> mTransparentObjects;

	};
}

#pragma once
#include "core.h"
#include <vector>
#include "mesh/mesh.h"
#include "framebuffer.h"
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
			mPhongShader =  std::make_shared<Shader>("shaders/phong/phong_V2.vert","shaders/phong/phong_V2.frag");
			mWhiteShader =  std::make_shared<Shader>("shaders/white/white.vert","shaders/white/white.frag");
			mDepthShader = std::make_shared<Shader>("shaders/depth/depth.vert", "shaders/depth/depth.frag");
			mOpacityMaskShader = std::make_shared<Shader>("shaders/opacityMask/phongOpacityMask.vert", "shaders/opacityMask/phongOpacityMask.frag");
			mScreenShader = std::make_shared<Shader>("shaders/screen/screen.vert", "shaders/screen/screen.frag");
			mCubeShader = std::make_shared<Shader>("shaders/cube/cube.vert", "shaders/cube/cube.frag");
			mPhongEnvShader = std::make_shared<Shader>("shaders/phongEnv/phongEnv.vert", "shaders/phongEnv/phongEnv_V2.frag");
			mCubeSphereShader = std::make_shared<Shader>("shaders/cube/cube.vert", "shaders/cube/cubeSphere.frag");
			mPhongEnvSphereShader = std::make_shared<Shader>("shaders/phongEnv/phongEnv.vert", "shaders/phongEnv/phongEnvSphere.frag");
			mPhongInstanceShader = std::make_shared<Shader>("shaders/phongInstance/phongInstance.vert", "shaders/phongInstance/phongInstance.frag");
			mGrassInstanceShader = std::make_shared<Shader>("shaders/grassInstance/grassInstance.vert", "shaders/grassInstance/grassInstance.frag");
			mPhongNormalShader = std::make_shared<Shader>("shaders/phongNormal/phongNormal_V2.vert", "shaders/phongNormal/phongNormal_V2.frag");
			mPhongParallaxShader = std::make_shared<Shader>("shaders/phongParallax/phongParallax_V2.vert", "shaders/phongParallax/phongParallax_V2.frag");
			mShadowShader = std::make_shared<Shader>("shaders/shadow/shadow.vert", "shaders/shadow/shadow.frag");
			mPhongShadowShader = std::make_shared<Shader>("shaders/phong/phongShadow.vert", "shaders/phong/phongShadow.frag");

			mShadowFBO = Framebuffer::createShadowFBO(2048,2048); 
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
			std::shared_ptr <GLframework::AmbientLight> ambient,
			unsigned int fbo = 0
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
		void renderShadowMap(
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			std::shared_ptr<DirectionalLight> dirLight,
			std::shared_ptr<Framebuffer> fbo
		);
		void setDepthState(std::shared_ptr<GLframework::Material> material);
		void setPolygonOffsetState(std::shared_ptr<GLframework::Material> material);
		void setStencilState(std::shared_ptr<GLframework::Material> material);
		void setColorBlendState(std::shared_ptr<GLframework::Material> material);
		void projectObject(std::shared_ptr<Object> obj);
		void setFaceCullingState(std::shared_ptr<GLframework::Material> material);
		glm::mat4 getLightMatrix(std::shared_ptr<DirectionalLight> dirLight);
	private:
		//生成多种不同的shader对象
		//根据材质类型的不同，挑选选择哪个shader对象
		std::shared_ptr<Shader> mPhongShader{ nullptr };
		std::shared_ptr<Shader> mWhiteShader{ nullptr };
		std::shared_ptr<Shader> mDepthShader{ nullptr };
		std::shared_ptr<Shader> mOpacityMaskShader{ nullptr };
		std::shared_ptr<Shader> mScreenShader{ nullptr };
		std::shared_ptr<Shader> mCubeShader{ nullptr };
		std::shared_ptr<Shader> mPhongEnvShader{ nullptr };
		std::shared_ptr<Shader> mCubeSphereShader{ nullptr };
		std::shared_ptr<Shader> mPhongEnvSphereShader{ nullptr };
		std::shared_ptr<Shader> mPhongInstanceShader{ nullptr };
		std::shared_ptr<Shader> mGrassInstanceShader{ nullptr };
		std::shared_ptr<Shader> mPhongNormalShader{ nullptr };
		std::shared_ptr<Shader> mPhongParallaxShader{ nullptr };
		std::shared_ptr<Shader> mShadowShader{ nullptr };
		std::shared_ptr<Shader> mPhongShadowShader{ nullptr };
		//不透明队列与透明队列
		//ops: 每一帧绘制前需要清空两个队列
		std::vector<std::shared_ptr<Mesh>> mOpacityObjects;
		std::vector<std::shared_ptr<Mesh>> mTransparentObjects;
		
	public:
		std::shared_ptr<Framebuffer> mShadowFBO{ nullptr };
	};
}

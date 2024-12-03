#pragma once
#include "core.h"
#include <vector>
#include "mesh/mesh.h"
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
			mPhongShader =  std::make_shared<Shader>("shaders/phong/phongPointLight.vert","shaders/phong/phong.frag");
			mWhiteShader =  std::make_shared<Shader>("shaders/white/white.vert","shaders/white/white.frag");
			mDepthShader = std::make_shared<Shader>("shaders/depth/depth.vert", "shaders/depth/depth.frag");
			mOpacityMaskShader = std::make_shared<Shader>("shaders/opacityMask/phongOpacityMask.vert", "shaders/opacityMask/phongOpacityMask.frag");
			mScreenShader = std::make_shared<Shader>("shaders/screen/screen.vert", "shaders/screen/screen.frag");
			mCubeShader = std::make_shared<Shader>("shaders/cube/cube.vert", "shaders/cube/cube.frag");
			mPhongEnvShader = std::make_shared<Shader>("shaders/phongEnv/phongEnv.vert", "shaders/phongEnv/phongEnv.frag");
			mCubeSphereShader = std::make_shared<Shader>("shaders/cube/cube.vert", "shaders/cube/cubeSphere.frag");
			mPhongEnvSphereShader = std::make_shared<Shader>("shaders/phongEnv/phongEnv.vert", "shaders/phongEnv/phongEnvSphere.frag");
			mPhongInstanceShader = std::make_shared<Shader>("shaders/phongInstance/phongInstance.vert", "shaders/phongInstance/phongInstance.frag");
			mGrassInstanceShader = std::make_shared<Shader>("shaders/grassInstance/grassInstance.vert", "shaders/grassInstance/grassInstance.frag");
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
			std::shared_ptr <GLframework::AmbientLight> ambient,
			unsigned int fbo = 0
		);
		void setClearColor(glm::vec3 color);

	public:
		std::shared_ptr<Material> mGlobalMaterial{nullptr};
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
		void setDepthState(std::shared_ptr<GLframework::Material> material);
		void setPolygonOffsetState(std::shared_ptr<GLframework::Material> material);
		void setStencilState(std::shared_ptr<GLframework::Material> material);
		void setColorBlendState(std::shared_ptr<GLframework::Material> material);
		void projectObject(std::shared_ptr<Object> obj);
		void setFaceCullingState(std::shared_ptr<GLframework::Material> material);
	private:
		//���ɶ��ֲ�ͬ��shader����
		//���ݲ������͵Ĳ�ͬ����ѡѡ���ĸ�shader����
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
		//��͸��������͸������
		//ops: ÿһ֡����ǰ��Ҫ�����������
		std::vector<std::shared_ptr<Mesh>> mOpacityObjects;
		std::vector<std::shared_ptr<Mesh>> mTransparentObjects;
	};
}

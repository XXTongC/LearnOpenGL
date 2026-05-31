#pragma once
#include <memory>
#include <vector>

#include "third_party/glm/glm.hpp"

class Camera;

namespace GLframework
{
	class AmbientLight;
	class DirectionalLight;
	class EnvironmentRenderTargets;
	struct EnvironmentProfile;
	class IBLPrecomputePass;
	class Material;
	enum class MaterialType;
	class Mesh;
	class PointLight;
	struct RendererFramePassProfile;
	struct RendererFrameStats;
	class Scene;
	class Shader;
	class SpotLight;
	class Texture;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		std::shared_ptr<Shader> getShader(MaterialType type);
		std::shared_ptr<Shader> getIBLCaptureShader() const;
		std::shared_ptr<Shader> getIBLBrdfLutShader() const;
		const EnvironmentRenderTargets& getEnvironmentRenderTargets() const;
		EnvironmentRenderTargets& getEnvironmentRenderTargets();
		const IBLPrecomputePass& getIBLPrecomputePass() const;
		IBLPrecomputePass& getIBLPrecomputePass();
		const RendererFrameStats& getLastFrameStats() const;
		const RendererFramePassProfile& getFramePassProfile() const;
		RendererFramePassProfile& getFramePassProfile();
		bool precomputeEnvironment(
			const std::shared_ptr<Texture>& equirectangularMap,
			const std::shared_ptr<Mesh>& captureCube,
			const std::shared_ptr<Mesh>& brdfQuad
		);
		bool precomputeEnvironment(const EnvironmentProfile& profile);
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

	public:
		std::shared_ptr<Material> mGlobalMaterial{nullptr};

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}

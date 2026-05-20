#pragma once
#include "core.h"
#include <vector>
#include "mesh/mesh.h"
#include "framebuffer.h"
#include "camera.h"
#include "shader.h"
#include "EnvironmentRenderTargets.h"
#include "FrameRenderState.h"
#include "IBLPrecomputePass.h"
#include "IBLDebugPass.h"
#include "PBRDepthPrepass.h"
#include "PBRSceneRenderPass.h"
#include "RenderQueue.h"
#include "RendererFrameStats.h"
#include "RendererFramePassProfile.h"
#include "SceneRenderPass.h"
#include "ShadowRenderer.h"
#include "ShaderLibrary.h"
#include "directionalLight.h"
#include "ambientLight.h"
#include "pointLight.h"
#include "spotLight.h"
#include "scene.h"
namespace GLframework
{
	struct EnvironmentProfile;

	class Renderer
	{
	public:
		Renderer();
		~Renderer() = default;
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
		ShaderLibrary mShaderLibrary{};
		FrameRenderState mFrameRenderState{};
		RenderQueue mRenderQueue{};
		ShadowRenderer mShadowRenderer{};
		SceneRenderPass mSceneRenderPass{};
		PBRDepthPrepass mPbrDepthPrepass{};
		PBRSceneRenderPass mPbrSceneRenderPass{};
		IBLDebugPass mIblDebugPass{};
		EnvironmentRenderTargets mEnvironmentRenderTargets{};
		IBLPrecomputePass mIblPrecomputePass{};
		RendererFrameStats mLastFrameStats{};
		RendererFramePassProfile mFramePassProfile{};

	};
}

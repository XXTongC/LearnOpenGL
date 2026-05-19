#include "renderer.h"

using namespace GLframework;

Renderer::Renderer()
{
	mShaderLibrary.initialize();
	mEnvironmentRenderTargets.initialize();
}

void Renderer::setClearColor(glm::vec3 color)
{
	glClearColor(color.r, color.g, color.b, 1.0f);
}

std::shared_ptr<Shader> Renderer::getShader(MaterialType type)
{
	return mShaderLibrary.get(type);
}

const EnvironmentRenderTargets& Renderer::getEnvironmentRenderTargets() const
{
	return mEnvironmentRenderTargets;
}

EnvironmentRenderTargets& Renderer::getEnvironmentRenderTargets()
{
	return mEnvironmentRenderTargets;
}

const IBLPrecomputePass& Renderer::getIBLPrecomputePass() const
{
	return mIblPrecomputePass;
}

IBLPrecomputePass& Renderer::getIBLPrecomputePass()
{
	return mIblPrecomputePass;
}

void Renderer::render(
	std::shared_ptr<Scene> scene,
	Camera* camera,
	std::shared_ptr<DirectionalLight> dirLight,
	std::shared_ptr<SpotLight> spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	std::shared_ptr<AmbientLight> ambient,
	unsigned int fbo
)
{
	mFrameRenderState.begin(fbo);
	mRenderQueue.build(scene, camera);

	//	render shadowmap
	mShadowRenderer.render(camera, mRenderQueue.getOpacityObjects(), dirLight, pointLights, mShaderLibrary);

	mSceneRenderPass.render(
		mRenderQueue.getOpacityObjects(),
		mRenderQueue.getTransparentObjects(),
		camera,
		dirLight,
		spotLight,
		pointLights,
		ambient,
		mGlobalMaterial,
		mShaderLibrary
	);
}

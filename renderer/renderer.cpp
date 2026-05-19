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

std::shared_ptr<Shader> Renderer::getIBLCaptureShader() const
{
	return mShaderLibrary.getEquirectangularToCubemapShader();
}

std::shared_ptr<Shader> Renderer::getIBLBrdfLutShader() const
{
	return mShaderLibrary.getBrdfLutShader();
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

bool Renderer::precomputeEnvironment(
	const std::shared_ptr<Texture>& equirectangularMap,
	const std::shared_ptr<Mesh>& captureCube,
	const std::shared_ptr<Mesh>& brdfQuad
)
{
	if (!mIblPrecomputePass.captureEnvironmentMap(equirectangularMap, mEnvironmentRenderTargets, captureCube, mShaderLibrary))
	{
		return false;
	}

	if (!mIblPrecomputePass.computeIrradianceMap(mEnvironmentRenderTargets, captureCube, mShaderLibrary))
	{
		return false;
	}

	if (!mIblPrecomputePass.computePrefilterMap(mEnvironmentRenderTargets, captureCube, mShaderLibrary))
	{
		return false;
	}

	return mIblPrecomputePass.computeBrdfLut(mEnvironmentRenderTargets, brdfQuad, mShaderLibrary);
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

#include "renderer.h"

#include "renderer/EnvironmentProfile.h"
#include "renderer/RendererFrameContext.h"
#include "renderer/RendererFramePassRegistry.h"

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

const RendererFrameStats& Renderer::getLastFrameStats() const
{
	return mLastFrameStats;
}

bool Renderer::precomputeEnvironment(
	const std::shared_ptr<Texture>& equirectangularMap,
	const std::shared_ptr<Mesh>& captureCube,
	const std::shared_ptr<Mesh>& brdfQuad
)
{
	mEnvironmentRenderTargets.setPrecomputedEnvironment(false);

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

	if (!mIblPrecomputePass.computeBrdfLut(mEnvironmentRenderTargets, brdfQuad, mShaderLibrary))
	{
		return false;
	}

	mEnvironmentRenderTargets.setPrecomputedEnvironment(true);
	return true;
}

bool Renderer::precomputeEnvironment(const EnvironmentProfile& profile)
{
	if (!profile.hasEnvironmentSource())
	{
		return false;
	}

	auto environment = EnvironmentTextureLoader::loadEquirectangular(profile);
	if (!environment)
	{
		return false;
	}

	auto captureCubeGeometry = Geometry::createBox(getIBLCaptureShader(), 2.0f, 2.0f, 2.0f);
	auto captureCube = std::make_shared<Mesh>(captureCubeGeometry, nullptr);
	captureCube->setName("IBL Capture Cube");

	auto brdfQuadGeometry = Geometry::createScreenPlane(getIBLBrdfLutShader());
	auto brdfQuad = std::make_shared<Mesh>(brdfQuadGeometry, nullptr);
	brdfQuad->setName("IBL BRDF LUT Quad");

	return precomputeEnvironment(environment, captureCube, brdfQuad);
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
	mLastFrameStats = {};
	RendererFrameContext frameContext{
		scene,
		camera,
		dirLight,
		spotLight,
		&pointLights,
		ambient,
		mGlobalMaterial,
		fbo,
		&mFrameRenderState,
		&mRenderQueue,
		&mShadowRenderer,
		&mSceneRenderPass,
		&mPbrDepthPrepass,
		&mPbrSceneRenderPass,
		&mShaderLibrary,
		&mEnvironmentRenderTargets,
		&mLastFrameStats
	};

	const auto& passPlan = mGlobalMaterial
		? RendererFramePassRegistry::globalMaterialOverridePasses()
		: RendererFramePassRegistry::defaultPasses();
	for (const auto& pass : passPlan)
	{
		RendererFramePassRegistry::executePass(pass, frameContext);
	}
}

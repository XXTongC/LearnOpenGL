#include "renderer.h"

#include "core.h"
#include "framework/geometry.h"
#include "framework/scene.h"
#include "framework/shader.h"
#include "framework/texture.h"
#include "mesh/mesh.h"
#include "renderer/EnvironmentRenderTargets.h"
#include "renderer/EnvironmentProfile.h"
#include "renderer/FrameRenderState.h"
#include "renderer/IBLDebugPass.h"
#include "renderer/IBLPrecomputePass.h"
#include "renderer/PBRDepthPrepass.h"
#include "renderer/PBRDeferredClusteredLightDebugPass.h"
#include "renderer/PBRDeferredLightingPass.h"
#include "renderer/PBRDeferredTiledLightDebugPass.h"
#include "renderer/PBRGBufferDebugPass.h"
#include "renderer/PBRGBufferPass.h"
#include "renderer/PBRGBufferRenderTargets.h"
#include "renderer/PBRSceneRenderPass.h"
#include "renderer/PBRShadowAtlasRenderPass.h"
#include "renderer/PBRShadowAtlasRenderTargets.h"
#include "renderer/RenderQueue.h"
#include "renderer/RendererFrameContext.h"
#include "renderer/RendererFramePassRegistry.h"
#include "renderer/RendererFramePassProfile.h"
#include "renderer/RendererFrameStats.h"
#include "renderer/RendererGpuTimerQueryPool.h"
#include "renderer/SceneRenderPass.h"
#include "renderer/ShaderLibrary.h"
#include "renderer/ShadowRenderer.h"

using namespace GLframework;

struct Renderer::Impl
{
	Impl()
	{
		shaderLibrary.initialize();
		environmentRenderTargets.initialize();
	}

	ShaderLibrary shaderLibrary{};
	FrameRenderState frameRenderState{};
	RenderQueue renderQueue{};
	ShadowRenderer shadowRenderer{};
	SceneRenderPass sceneRenderPass{};
	PBRDepthPrepass pbrDepthPrepass{};
	PBRGBufferPass pbrGBufferPass{};
	PBRDeferredLightingPass pbrDeferredLightingPass{};
	PBRDeferredTiledLightDebugPass pbrDeferredTiledLightDebugPass{};
	PBRDeferredClusteredLightDebugPass pbrDeferredClusteredLightDebugPass{};
	PBRGBufferDebugPass pbrGBufferDebugPass{};
	PBRSceneRenderPass pbrSceneRenderPass{};
	IBLDebugPass iblDebugPass{};
	PBRShadowAtlasRenderPass pbrShadowAtlasPass{};
	PBRShadowAtlasRenderTargets pbrShadowAtlasTargets{};
	PBRGBufferRenderTargets pbrGBufferTargets{};
	EnvironmentRenderTargets environmentRenderTargets{};
	IBLPrecomputePass iblPrecomputePass{};
	RendererGpuTimerQueryPool gpuTimerQueries{};
	RendererFrameStats lastFrameStats{};
	RendererFramePassProfile framePassProfile{};
};

Renderer::Renderer()
	: mImpl(std::make_unique<Impl>())
{
}

Renderer::~Renderer() = default;

void Renderer::setClearColor(glm::vec3 color)
{
	glClearColor(color.r, color.g, color.b, 1.0f);
}

std::shared_ptr<Shader> Renderer::getShader(MaterialType type)
{
	return mImpl->shaderLibrary.get(type);
}

std::shared_ptr<Shader> Renderer::getIBLCaptureShader() const
{
	return mImpl->shaderLibrary.getEquirectangularToCubemapShader();
}

std::shared_ptr<Shader> Renderer::getIBLBrdfLutShader() const
{
	return mImpl->shaderLibrary.getBrdfLutShader();
}

const EnvironmentRenderTargets& Renderer::getEnvironmentRenderTargets() const
{
	return mImpl->environmentRenderTargets;
}

EnvironmentRenderTargets& Renderer::getEnvironmentRenderTargets()
{
	return mImpl->environmentRenderTargets;
}

const IBLPrecomputePass& Renderer::getIBLPrecomputePass() const
{
	return mImpl->iblPrecomputePass;
}

IBLPrecomputePass& Renderer::getIBLPrecomputePass()
{
	return mImpl->iblPrecomputePass;
}

const RendererFrameStats& Renderer::getLastFrameStats() const
{
	return mImpl->lastFrameStats;
}

const RendererFramePassProfile& Renderer::getFramePassProfile() const
{
	return mImpl->framePassProfile;
}

RendererFramePassProfile& Renderer::getFramePassProfile()
{
	return mImpl->framePassProfile;
}

bool Renderer::precomputeEnvironment(
	const std::shared_ptr<Texture>& equirectangularMap,
	const std::shared_ptr<Mesh>& captureCube,
	const std::shared_ptr<Mesh>& brdfQuad
)
{
	mImpl->environmentRenderTargets.setPrecomputedEnvironment(false);

	if (!mImpl->iblPrecomputePass.captureEnvironmentMap(equirectangularMap, mImpl->environmentRenderTargets, captureCube, mImpl->shaderLibrary))
	{
		return false;
	}

	if (!mImpl->iblPrecomputePass.computeIrradianceMap(mImpl->environmentRenderTargets, captureCube, mImpl->shaderLibrary))
	{
		return false;
	}

	if (!mImpl->iblPrecomputePass.computePrefilterMap(mImpl->environmentRenderTargets, captureCube, mImpl->shaderLibrary))
	{
		return false;
	}

	if (!mImpl->iblPrecomputePass.computeBrdfLut(mImpl->environmentRenderTargets, brdfQuad, mImpl->shaderLibrary))
	{
		return false;
	}

	mImpl->environmentRenderTargets.setPrecomputedEnvironment(true);
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
	mImpl->lastFrameStats = {};
	mImpl->pbrShadowAtlasTargets.resetFrameStats();
	mImpl->gpuTimerQueries.beginFrame(mImpl->framePassProfile.rendererGpuTimingEnabled, mImpl->lastFrameStats);
	RendererFrameContext frameContext{
		scene,
		camera,
		dirLight,
		spotLight,
		&pointLights,
		ambient,
		mGlobalMaterial,
		fbo,
		&mImpl->frameRenderState,
		&mImpl->renderQueue,
		&mImpl->shadowRenderer,
		&mImpl->sceneRenderPass,
		&mImpl->pbrDepthPrepass,
		&mImpl->pbrGBufferPass,
		&mImpl->pbrDeferredLightingPass,
		&mImpl->pbrDeferredTiledLightDebugPass,
		&mImpl->pbrDeferredClusteredLightDebugPass,
		&mImpl->pbrGBufferDebugPass,
		&mImpl->pbrSceneRenderPass,
		&mImpl->iblDebugPass,
		&mImpl->pbrShadowAtlasPass,
		&mImpl->pbrShadowAtlasTargets,
		&mImpl->pbrGBufferTargets,
		&mImpl->shaderLibrary,
		&mImpl->environmentRenderTargets,
		&mImpl->framePassProfile,
		&mImpl->gpuTimerQueries,
		&mImpl->lastFrameStats
	};

	const auto passPlan = RendererFramePassRegistry::buildPassPlan(
		mGlobalMaterial
			? mImpl->framePassProfile.globalMaterialOverridePassOrder
			: mImpl->framePassProfile.defaultPassOrder
	);
	for (const auto* pass : passPlan)
	{
		if (pass)
		{
			RendererFramePassRegistry::executePass(*pass, frameContext);
		}
	}
	mImpl->gpuTimerQueries.endFrame(mImpl->lastFrameStats);
}

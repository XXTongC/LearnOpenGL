#include "RuntimeScenePreparer.h"

#include "../tools/Logger/LogManager.h"

namespace GL_RUNTIME
{
	GL_SCENE::SetupContext RuntimeScenePreparer::makeSceneSetupContext(
		GLframework::AppRuntimeContext& context,
		const RuntimeScenePrepareConfig& config
	)
	{
		return {
			context.renderer,
			context.sceneOffScreen,
			context.sceneInScreen,
			context.frameRenderTargets,
			context.bloom,
			context.screenQuad,
			context.skyBoxMesh,
			context.textD,
			context.screenMaterial,
			context.ambientLight,
			context.dirLight,
			context.spotLight,
			context.pointLights,
			config.width,
			config.height,
			config.texturePath,
			context.environmentProfile,
			context.pbrPreviewProfile,
			context.pbrLightRigProfile
		};
	}

	GL_EXPERIMENTS::RuntimeContext RuntimeScenePreparer::makeLegacyExperimentContext(
		GLframework::AppRuntimeContext& context
	)
	{
		return {
			context.renderer,
			context.sceneOffScreen,
			context.grassMaterial,
			context.skyBoxMesh,
			context.movePlane,
			context.csmShadowMaterial,
			context.dirLight,
			context.pointLights
		};
	}

	void RuntimeScenePreparer::prepare(
		GLframework::AppRuntimeContext& context,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		const RuntimeScenePrepareConfig& config
	)
	{
		auto sceneSetupContext = makeSceneSetupContext(context, config);
		GL_SCENE::prepareDefaultScene(sceneSetupContext);
		prepareLegacyExperiments(context, legacyExperiments, config);

		LogInfo(":\n Renderer Prepared\n SceneInScreen Prepared\n SceneOffScreen Prepared \n FramebufferMultisample Prepared\n FramebufferResolve Prepared\n PointLightShadow initialized\n Lights Ready \n Objects Ready");
	}

	void RuntimeScenePreparer::prepareLegacyExperiments(
		GLframework::AppRuntimeContext& context,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		const RuntimeScenePrepareConfig& config
	)
	{
		auto legacyContext = makeLegacyExperimentContext(context);
		(void)legacyContext;
		(void)config;

		// Re-enable legacy experiments here with a few focused calls.
		// legacyExperiments.enableSolarSystem(legacyContext);
		// legacyExperiments.enableGrassField(legacyContext, config.legacyGrassRows, config.legacyGrassColumns);
		// legacyExperiments.enableEnvironmentSphere(legacyContext);
		// legacyExperiments.enableCsmPlane(legacyContext);
		// legacyExperiments.enableBackpackModel(legacyContext);
		// legacyExperiments.enableShadowPreview(legacyContext);
		// legacyExperiments.enableOrbitingPointLight(0, 3.0f, 3.0f);
	}

	void RuntimeScenePreparer::updateLegacyExperiments(
		GLframework::AppRuntimeContext& context,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments
	)
	{
		auto legacyContext = makeLegacyExperimentContext(context);
		legacyExperiments.update(legacyContext);
	}
}

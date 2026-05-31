#include "RuntimeLegacyExperimentLifecycle.h"

#include "AppRuntimeContext.h"
#include "RuntimeScenePrepareConfig.h"
#include "../tools/legacyExperiments/LegacyExperimentRunner.h"

namespace GL_RUNTIME
{
	GL_EXPERIMENTS::RuntimeContext RuntimeLegacyExperimentLifecycle::makeContext(
		GLframework::AppRuntimeContext& context
	)
	{
		return {
			context.renderResources.renderer,
			context.renderResources.sceneOffScreen,
			context.renderResources.grassMaterial,
			context.renderResources.skyBoxMesh,
			context.renderResources.movePlane,
			context.renderResources.csmShadowMaterial,
			context.cameraLights.dirLight,
			context.cameraLights.pointLights
		};
	}

	void RuntimeLegacyExperimentLifecycle::prepare(
		GLframework::AppRuntimeContext& context,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		const RuntimeScenePrepareConfig& config
	)
	{
		auto legacyContext = makeContext(context);
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

	void RuntimeLegacyExperimentLifecycle::update(
		GLframework::AppRuntimeContext& context,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments
	)
	{
		auto legacyContext = makeContext(context);
		legacyExperiments.update(legacyContext);
	}
}

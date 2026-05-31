#include "RuntimeContentSceneLifecycle.h"

#include "RuntimeLegacyExperimentLifecycle.h"
#include "RuntimeScenePrepareConfig.h"
#include "RuntimeSceneSetupPipelineLifecycle.h"
#include "RuntimeSceneSetupReport.h"

namespace GL_RUNTIME
{
	void RuntimeContentSceneLifecycle::prepareScene(
		GLframework::AppRuntimeContext& context,
		GLengine::Engine& engine,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		const RuntimeScenePrepareConfig& config
	)
	{
		RuntimeSceneSetupPipelineLifecycle::prepare(context, engine, config);
		RuntimeLegacyExperimentLifecycle::prepare(context, legacyExperiments, config);

		RuntimeSceneSetupReport::reportRendererPrepared();
	}
}

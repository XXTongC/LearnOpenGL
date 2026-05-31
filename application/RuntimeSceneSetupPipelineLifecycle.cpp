#include "RuntimeSceneSetupPipelineLifecycle.h"

#include "RuntimeScenePrepareConfig.h"
#include "RuntimeSceneSetupContextFactory.h"
#include "RuntimeSceneSetupReport.h"
#include "../tools/sceneSetup/SceneSetupPipeline.h"

namespace GL_RUNTIME
{
	void RuntimeSceneSetupPipelineLifecycle::prepare(
		GLframework::AppRuntimeContext& context,
		GLengine::Engine& engine,
		const RuntimeScenePrepareConfig& config
	)
	{
		auto sceneSetupContext = RuntimeSceneSetupContextFactory::make(context, engine, config);
		const GL_SCENE::SceneSetupPipelineResult sceneSetupResult =
			GL_SCENE::prepareScene(sceneSetupContext, config.sceneSetupPipeline);
		RuntimeSceneSetupReport::reportPreparedScene(sceneSetupResult);
	}
}

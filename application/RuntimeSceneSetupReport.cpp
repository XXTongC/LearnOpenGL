#include "RuntimeSceneSetupReport.h"

#include <iostream>
#include <string>

#include "../tools/Logger/LogManager.h"
#include "../tools/sceneSetup/SceneSetupPipeline.h"

namespace GL_RUNTIME
{
	namespace
	{
		void reportLine(const std::string& message)
		{
			std::cout << message << std::endl;
			LogInfo(message);
		}
	}

	void RuntimeSceneSetupReport::reportPreparedScene(const GL_SCENE::SceneSetupPipelineResult& result)
	{
		if (result.legacyWorldMirrorPrepared)
		{
			reportLine(GL_SCENE::formatLegacyWorldMirrorStats(result.legacyWorldMirrorStats));
		}
		if (result.worldDrivenMinimalScenePrepared)
		{
			reportLine(GL_SCENE::formatEngineWorldMinimalSceneStats(result.worldDrivenMinimalSceneStats));
		}
		if (result.worldDrivenProbeRequested)
		{
			reportLine(GL_SCENE::formatEngineWorldSceneProbeStats(result.worldDrivenProbeStats));
		}
	}

	void RuntimeSceneSetupReport::reportRendererPrepared()
	{
		LogInfo(":\n Renderer Prepared\n SceneInScreen Prepared\n SceneOffScreen Prepared \n FramebufferMultisample Prepared\n FramebufferResolve Prepared\n PointLightShadow initialized\n Lights Ready \n Objects Ready");
	}
}

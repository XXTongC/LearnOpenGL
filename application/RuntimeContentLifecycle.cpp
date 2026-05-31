#include "RuntimeContentLifecycle.h"

#include "RuntimeContentCameraLifecycle.h"
#include "RuntimeContentLifecycleConfig.h"
#include "RuntimeContentRendererBackendLifecycle.h"
#include "RuntimeContentSceneLifecycle.h"
#include "RuntimeContentVerificationLifecycle.h"

namespace GL_RUNTIME
{
	bool RuntimeContentLifecycle::prepare(
		GLframework::AppRuntimeContext& context,
		GLengine::Engine& engine,
		RuntimeEngineLifecycleState& engineLifecycle,
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
		const RuntimeContentLifecycleConfig& config
	)
	{
		RuntimeContentCameraLifecycle::initializeCamera(context, config.camera);
		RuntimeContentVerificationLifecycle::loadStartupProfiles(context, config.verification);
		RuntimeContentSceneLifecycle::prepareScene(context, engine, legacyExperiments, config.scene);
		if (!RuntimeContentRendererBackendLifecycle::attachAfterScenePreparation(
			context,
			engineLifecycle,
			config.rendererBackendKey
		))
		{
			return false;
		}

		RuntimeContentVerificationLifecycle::reportPreparedScene(context, config.verification);
		return true;
	}
}

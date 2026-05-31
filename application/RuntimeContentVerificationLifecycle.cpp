#include "RuntimeContentVerificationLifecycle.h"

#include "RuntimeProfileLoader.h"
#include "RuntimeVerificationPreparedSceneLifecycle.h"
#include "RuntimeVerificationStartupProfileLifecycle.h"

namespace GL_RUNTIME
{
	void RuntimeContentVerificationLifecycle::loadStartupProfiles(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& config
	)
	{
		RuntimeProfileLoader::loadAll(context);
		RuntimeVerificationStartupProfileLifecycle::applyStartupProfile(context, config);
	}

	void RuntimeContentVerificationLifecycle::reportPreparedScene(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& config
	)
	{
		RuntimeVerificationPreparedSceneLifecycle::reportPreparedScene(context, config);
	}
}

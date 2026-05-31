#include "RuntimeVerificationPreparedSceneLifecycle.h"

#include "RuntimeEngineWorldVerification.h"
#include "RuntimeImportedAssetVerification.h"
#include "RuntimePBRPassProfileVerification.h"
#include "RuntimePBRPreparedSceneStatsVerification.h"
#include "RuntimePBRSceneProbeVerification.h"
#include "RuntimeVerificationConfig.h"

namespace GL_RUNTIME
{
	void RuntimeVerificationPreparedSceneLifecycle::reportPreparedScene(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& config
	)
	{
		if (!config.enabled)
		{
			return;
		}

		RuntimeEngineWorldVerification::addVerificationSceneProbes(context, config);
		RuntimePBRSceneProbeVerification::addVerificationSceneProbes(context, config);
		RuntimeImportedAssetVerification::addVerificationSceneProbes(context, config);
		RuntimePBRPassProfileVerification::applyRendererPassProfile(context, config);
		RuntimePBRPreparedSceneStatsVerification::reportPreparedScene(context, config);
		RuntimeEngineWorldVerification::reportPreparedScene(context, config);
	}
}

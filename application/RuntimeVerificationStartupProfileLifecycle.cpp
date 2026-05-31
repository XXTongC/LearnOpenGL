#include "RuntimeVerificationStartupProfileLifecycle.h"

#include "RuntimePBRProfileVerification.h"
#include "RuntimeVerificationConfig.h"

namespace GL_RUNTIME
{
	void RuntimeVerificationStartupProfileLifecycle::applyStartupProfile(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& config
	)
	{
		if (!config.enabled)
		{
			return;
		}

		RuntimePBRProfileVerification::applyProfile(context, config);
	}
}

#include "RuntimePBRProfileVerification.h"

#include "RuntimePBRLightCameraRigVerification.h"
#include "RuntimePBRPassProfileVerification.h"
#include "RuntimePBRProfileLineVerification.h"
#include "RuntimePBRPreviewProfileVerification.h"
#include "RuntimePBRStartupProfileVerification.h"

namespace GL_RUNTIME
{
	void RuntimePBRProfileVerification::applyProfile(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& verification
	)
	{
		RuntimePBRStartupProfileVerification::applyStartupProfile(context);
		RuntimePBRPassProfileVerification::applyRendererPassProfile(context, verification);
		RuntimePBRPreviewProfileVerification::applyPreviewProfile(context, verification);
		RuntimePBRLightCameraRigVerification::applyLightCameraRig(context, verification);
		RuntimePBRProfileLineVerification::reportAppliedProfile(verification);
	}

}

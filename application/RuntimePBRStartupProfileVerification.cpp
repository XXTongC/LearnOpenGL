#include "RuntimePBRStartupProfileVerification.h"

#include "AppRuntimeContext.h"

namespace GL_RUNTIME
{
	void RuntimePBRStartupProfileVerification::applyStartupProfile(GLframework::AppRuntimeContext& context)
	{
		context.profiles.environmentProfile.precomputeOnPrepare = true;
		context.profiles.environmentProfile.useProceduralEnvironment = true;
		context.profiles.environmentProfile.proceduralWidth = 128;
		context.profiles.environmentProfile.proceduralHeight = 64;
		context.profiles.environmentProfile.proceduralSkyIntensity = 1.5f;
		context.profiles.environmentProfile.proceduralGroundIntensity = 0.15f;
		context.profiles.environmentProfile.proceduralSunIntensity = 4.0f;

		context.profiles.postProcessSettings.exposure = 1.0f;
		context.profiles.postProcessSettings.bloomEnabled = true;
		context.profiles.postProcessSettings.bloomThreshold = 1.0f;
		context.profiles.postProcessSettings.bloomIntensity = 0.04f;
		context.profiles.postProcessSettings.bloomIterations = 6;

		context.profiles.framePipelineProfile.sceneColorPassEnabled = true;
		context.profiles.framePipelineProfile.sceneResolvePassEnabled = true;
		context.profiles.framePipelineProfile.bloomPassEnabled = true;
		context.profiles.framePipelineProfile.screenCompositePassEnabled = true;
		context.profiles.framePipelineProfile.passOrder = "SceneColor,SceneResolve,Bloom,ScreenComposite";
	}
}

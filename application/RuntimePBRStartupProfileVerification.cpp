#include "RuntimePBRStartupProfileVerification.h"

#include "AppRuntimeContext.h"
#include "../renderer/EnvironmentProfile.h"
#include "../renderer/PostProcessSettings.h"

namespace GL_RUNTIME
{
	void RuntimePBRStartupProfileVerification::applyStartupProfile(GLframework::AppRuntimeContext& context)
	{
		auto& environmentProfile = context.profiles.environmentProfile();
		environmentProfile.precomputeOnPrepare = true;
		environmentProfile.useProceduralEnvironment = true;
		environmentProfile.proceduralWidth = 128;
		environmentProfile.proceduralHeight = 64;
		environmentProfile.proceduralSkyIntensity = 1.5f;
		environmentProfile.proceduralGroundIntensity = 0.15f;
		environmentProfile.proceduralSunIntensity = 4.0f;

		auto& postProcessSettings = context.profiles.postProcessSettings();
		postProcessSettings.exposure = 1.0f;
		postProcessSettings.bloomEnabled = true;
		postProcessSettings.bloomThreshold = 1.0f;
		postProcessSettings.bloomIntensity = 0.04f;
		postProcessSettings.bloomIterations = 6;

		context.profiles.framePipelineProfile.sceneColorPassEnabled = true;
		context.profiles.framePipelineProfile.sceneResolvePassEnabled = true;
		context.profiles.framePipelineProfile.bloomPassEnabled = true;
		context.profiles.framePipelineProfile.screenCompositePassEnabled = true;
		context.profiles.framePipelineProfile.passOrder = "SceneColor,SceneResolve,Bloom,ScreenComposite";
	}
}

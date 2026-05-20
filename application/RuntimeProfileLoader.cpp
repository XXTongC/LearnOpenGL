#include "RuntimeProfileLoader.h"

#include "../tools/Logger/LogManager.h"

namespace
{
	void loadEnvironmentProfile(GLframework::AppRuntimeContext& context)
	{
		if (GLframework::EnvironmentProfileStorage::loadFromFile(context.environmentProfilePath, context.environmentProfile))
		{
			LogInfo("Environment profile loaded from " + context.environmentProfilePath);
			return;
		}

		LogInfo("Environment profile config not found, using defaults: " + context.environmentProfilePath);
	}

	void loadPostProcessSettings(GLframework::AppRuntimeContext& context)
	{
		if (GLframework::PostProcessSettingsStorage::loadFromFile(context.postProcessSettingsPath, context.postProcessSettings))
		{
			LogInfo("Postprocess settings loaded from " + context.postProcessSettingsPath);
			return;
		}

		LogInfo("Postprocess settings config not found, using defaults: " + context.postProcessSettingsPath);
	}

	void loadPBRPreviewProfile(GLframework::AppRuntimeContext& context)
	{
		if (GL_SCENE::PBRPreviewProfileStorage::loadFromFile(context.pbrPreviewProfilePath, context.pbrPreviewProfile))
		{
			LogInfo("PBR preview profile loaded from " + context.pbrPreviewProfilePath);
			return;
		}

		LogInfo("PBR preview profile config not found, using defaults: " + context.pbrPreviewProfilePath);
	}

	void loadPBRExperimentProfile(GLframework::AppRuntimeContext& context)
	{
		if (GL_SCENE::PBRExperimentProfileStorage::loadFromFile(
			context.pbrExperimentProfilePath,
			context.environmentProfile,
			context.postProcessSettings,
			context.pbrPreviewProfile,
			context.pbrLightRigProfile,
			context.pbrCameraRigProfile
		))
		{
			context.pbrCameraRigProfile.applyTo(context.camera);
			LogInfo("PBR experiment profile loaded from " + context.pbrExperimentProfilePath);
			return;
		}

		LogInfo("PBR experiment profile config not found, using layered defaults: " + context.pbrExperimentProfilePath);
	}
}

void GL_RUNTIME::RuntimeProfileLoader::loadAll(GLframework::AppRuntimeContext& context)
{
	loadEnvironmentProfile(context);
	loadPostProcessSettings(context);
	loadPBRPreviewProfile(context);
	loadPBRExperimentProfile(context);
}

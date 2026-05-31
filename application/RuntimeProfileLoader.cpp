#include "RuntimeProfileLoader.h"

#include "AppRuntimeContext.h"
#include "../renderer/EnvironmentProfile.h"
#include "../renderer/PostProcessSettings.h"
#include "../renderer/RendererFramePassProfile.h"
#include "../renderer/renderer.h"
#include "../tools/Logger/LogManager.h"
#include "../tools/sceneSetup/PBRCameraRigProfile.h"
#include "../tools/sceneSetup/PBRExperimentProfile.h"

namespace
{
	void loadEnvironmentProfile(GLframework::AppRuntimeContext& context)
	{
		if (GLframework::EnvironmentProfileStorage::loadFromFile(
			context.profiles.environmentProfilePath,
			context.profiles.environmentProfile()
		))
		{
			LogInfo("Environment profile loaded from " + context.profiles.environmentProfilePath);
			return;
		}

		LogInfo("Environment profile config not found, using defaults: " + context.profiles.environmentProfilePath);
	}

	void loadPostProcessSettings(GLframework::AppRuntimeContext& context)
	{
		if (GLframework::PostProcessSettingsStorage::loadFromFile(
			context.profiles.postProcessSettingsPath,
			context.profiles.postProcessSettings()
		))
		{
			LogInfo("Postprocess settings loaded from " + context.profiles.postProcessSettingsPath);
			return;
		}

		LogInfo("Postprocess settings config not found, using defaults: " + context.profiles.postProcessSettingsPath);
	}

	void loadFramePipelineProfile(GLframework::AppRuntimeContext& context)
	{
		if (GL_RUNTIME::RuntimeFramePipelineProfileStorage::loadFromFile(
			context.profiles.framePipelineProfilePath,
			context.profiles.framePipelineProfile
		))
		{
			LogInfo("Runtime frame pipeline profile loaded from " + context.profiles.framePipelineProfilePath);
			return;
		}

		LogInfo("Runtime frame pipeline profile config not found, using defaults: " + context.profiles.framePipelineProfilePath);
	}

	void loadRendererFramePassProfile(GLframework::AppRuntimeContext& context)
	{
		if (!context.renderResources.renderer)
		{
			return;
		}

		if (GLframework::RendererFramePassProfileStorage::loadFromFile(
			context.profiles.rendererFramePassProfilePath,
			context.renderResources.renderer->getFramePassProfile()
		))
		{
			LogInfo("Renderer frame pass profile loaded from " + context.profiles.rendererFramePassProfilePath);
			return;
		}

		LogInfo("Renderer frame pass profile config not found, using defaults: " + context.profiles.rendererFramePassProfilePath);
	}

	void loadPBRPreviewProfile(GLframework::AppRuntimeContext& context)
	{
		if (GL_SCENE::PBRPreviewProfileStorage::loadFromFile(
			context.profiles.pbrPreviewProfilePath,
			context.profiles.pbrPreviewProfile
		))
		{
			LogInfo("PBR preview profile loaded from " + context.profiles.pbrPreviewProfilePath);
			return;
		}

		LogInfo("PBR preview profile config not found, using defaults: " + context.profiles.pbrPreviewProfilePath);
	}

	void loadPBRExperimentProfile(GLframework::AppRuntimeContext& context)
	{
		if (GL_SCENE::PBRExperimentProfileStorage::loadFromFile(
			context.profiles.pbrExperimentProfilePath,
			context.profiles.environmentProfile(),
			context.profiles.postProcessSettings(),
			context.profiles.pbrPreviewProfile,
			context.profiles.pbrLightRigProfile(),
			context.profiles.pbrCameraRigProfile()
		))
		{
			context.profiles.pbrCameraRigProfile().applyTo(context.cameraLights.camera);
			LogInfo("PBR experiment profile loaded from " + context.profiles.pbrExperimentProfilePath);
			return;
		}

		LogInfo("PBR experiment profile config not found, using layered defaults: " + context.profiles.pbrExperimentProfilePath);
	}
}

void GL_RUNTIME::RuntimeProfileLoader::loadAll(GLframework::AppRuntimeContext& context)
{
	loadEnvironmentProfile(context);
	loadPostProcessSettings(context);
	loadFramePipelineProfile(context);
	loadRendererFramePassProfile(context);
	loadPBRPreviewProfile(context);
	loadPBRExperimentProfile(context);
}

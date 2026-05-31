#pragma once

#include <string>

#include "RuntimeFramePipelineProfile.h"
#include "../renderer/EnvironmentProfile.h"
#include "../renderer/PostProcessSettings.h"
#include "../renderer/RendererFramePassProfile.h"
#include "../tools/sceneSetup/PBRCameraRigProfile.h"
#include "../tools/sceneSetup/PBRExperimentProfile.h"
#include "../tools/sceneSetup/PBRLightRigProfile.h"
#include "../tools/sceneSetup/PBRPreviewProfile.h"

namespace GL_RUNTIME
{
	struct RuntimeProfileState
	{
		RuntimeFramePipelineProfile framePipelineProfile{};
		std::string framePipelineProfilePath{ RuntimeFramePipelineProfileStorage::defaultPath() };
		std::string rendererFramePassProfilePath{ GLframework::RendererFramePassProfileStorage::defaultPath() };
		GLframework::PostProcessSettings postProcessSettings{};
		std::string postProcessSettingsPath{ GLframework::PostProcessSettingsStorage::defaultPath() };
		GLframework::EnvironmentProfile environmentProfile{};
		std::string environmentProfilePath{ GLframework::EnvironmentProfileStorage::defaultPath() };
		GL_SCENE::PBRPreviewProfile pbrPreviewProfile{};
		GL_SCENE::PBRLightRigProfile pbrLightRigProfile{};
		GL_SCENE::PBRCameraRigProfile pbrCameraRigProfile{};
		std::string pbrPreviewProfilePath{ GL_SCENE::PBRPreviewProfileStorage::defaultPath() };
		std::string pbrExperimentProfilePath{ GL_SCENE::PBRExperimentProfileStorage::defaultPath() };
	};
}

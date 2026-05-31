#pragma once

#include <string>

#include "RuntimeFramePipelineProfile.h"
#include "../renderer/EnvironmentProfile.h"
#include "../renderer/PostProcessSettings.h"
#include "../tools/sceneSetup/PBRCameraRigProfile.h"
#include "../tools/sceneSetup/PBRLightRigProfile.h"
#include "../tools/sceneSetup/PBRPreviewProfile.h"

namespace GL_RUNTIME
{
	struct RuntimeProfileState
	{
		RuntimeProfileState();

		RuntimeFramePipelineProfile framePipelineProfile{};
		std::string framePipelineProfilePath{};
		std::string rendererFramePassProfilePath{};
		GLframework::PostProcessSettings postProcessSettings{};
		std::string postProcessSettingsPath{};
		GLframework::EnvironmentProfile environmentProfile{};
		std::string environmentProfilePath{};
		GL_SCENE::PBRPreviewProfile pbrPreviewProfile{};
		GL_SCENE::PBRLightRigProfile pbrLightRigProfile{};
		GL_SCENE::PBRCameraRigProfile pbrCameraRigProfile{};
		std::string pbrPreviewProfilePath{};
		std::string pbrExperimentProfilePath{};
	};
}

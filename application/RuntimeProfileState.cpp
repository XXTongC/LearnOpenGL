#include "RuntimeProfileState.h"

#include "../renderer/RendererFramePassProfile.h"
#include "../tools/sceneSetup/PBRExperimentProfile.h"

GL_RUNTIME::RuntimeProfileState::RuntimeProfileState()
	: framePipelineProfilePath(RuntimeFramePipelineProfileStorage::defaultPath())
	, rendererFramePassProfilePath(GLframework::RendererFramePassProfileStorage::defaultPath())
	, postProcessSettingsPath(GLframework::PostProcessSettingsStorage::defaultPath())
	, environmentProfilePath(GLframework::EnvironmentProfileStorage::defaultPath())
	, pbrPreviewProfilePath(GL_SCENE::PBRPreviewProfileStorage::defaultPath())
	, pbrExperimentProfilePath(GL_SCENE::PBRExperimentProfileStorage::defaultPath())
{
}

#include "RuntimeProfileState.h"

#include "../renderer/RendererFramePassProfile.h"
#include "../tools/sceneSetup/PBRCameraRigProfile.h"
#include "../tools/sceneSetup/PBRExperimentProfile.h"

GL_RUNTIME::RuntimeProfileState::RuntimeProfileState()
	: framePipelineProfilePath(RuntimeFramePipelineProfileStorage::defaultPath())
	, rendererFramePassProfilePath(GLframework::RendererFramePassProfileStorage::defaultPath())
	, postProcessSettingsPath(GLframework::PostProcessSettingsStorage::defaultPath())
	, environmentProfilePath(GLframework::EnvironmentProfileStorage::defaultPath())
	, pbrPreviewProfilePath(GL_SCENE::PBRPreviewProfileStorage::defaultPath())
	, pbrExperimentProfilePath(GL_SCENE::PBRExperimentProfileStorage::defaultPath())
	, mPBRCameraRigProfile(std::make_unique<GL_SCENE::PBRCameraRigProfile>())
{
}

GL_RUNTIME::RuntimeProfileState::~RuntimeProfileState() = default;

GL_RUNTIME::RuntimeProfileState::RuntimeProfileState(RuntimeProfileState&&) noexcept = default;

GL_RUNTIME::RuntimeProfileState& GL_RUNTIME::RuntimeProfileState::operator=(RuntimeProfileState&&) noexcept = default;

GL_SCENE::PBRCameraRigProfile& GL_RUNTIME::RuntimeProfileState::pbrCameraRigProfile()
{
	return *mPBRCameraRigProfile;
}

const GL_SCENE::PBRCameraRigProfile& GL_RUNTIME::RuntimeProfileState::pbrCameraRigProfile() const
{
	return *mPBRCameraRigProfile;
}

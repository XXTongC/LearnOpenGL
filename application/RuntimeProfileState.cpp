#include "RuntimeProfileState.h"

#include "../renderer/EnvironmentProfile.h"
#include "../renderer/PostProcessSettings.h"
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
	, mPostProcessSettings(std::make_unique<GLframework::PostProcessSettings>())
	, mEnvironmentProfile(std::make_unique<GLframework::EnvironmentProfile>())
	, mPBRCameraRigProfile(std::make_unique<GL_SCENE::PBRCameraRigProfile>())
{
}

GL_RUNTIME::RuntimeProfileState::~RuntimeProfileState() = default;

GL_RUNTIME::RuntimeProfileState::RuntimeProfileState(RuntimeProfileState&&) noexcept = default;

GL_RUNTIME::RuntimeProfileState& GL_RUNTIME::RuntimeProfileState::operator=(RuntimeProfileState&&) noexcept = default;

GLframework::PostProcessSettings& GL_RUNTIME::RuntimeProfileState::postProcessSettings()
{
	return *mPostProcessSettings;
}

const GLframework::PostProcessSettings& GL_RUNTIME::RuntimeProfileState::postProcessSettings() const
{
	return *mPostProcessSettings;
}

GLframework::EnvironmentProfile& GL_RUNTIME::RuntimeProfileState::environmentProfile()
{
	return *mEnvironmentProfile;
}

const GLframework::EnvironmentProfile& GL_RUNTIME::RuntimeProfileState::environmentProfile() const
{
	return *mEnvironmentProfile;
}

GL_SCENE::PBRCameraRigProfile& GL_RUNTIME::RuntimeProfileState::pbrCameraRigProfile()
{
	return *mPBRCameraRigProfile;
}

const GL_SCENE::PBRCameraRigProfile& GL_RUNTIME::RuntimeProfileState::pbrCameraRigProfile() const
{
	return *mPBRCameraRigProfile;
}

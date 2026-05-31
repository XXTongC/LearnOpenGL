#pragma once

#include <memory>
#include <string>

#include "RuntimeFramePipelineProfile.h"
#include "../tools/sceneSetup/PBRLightRigProfile.h"
#include "../tools/sceneSetup/PBRPreviewProfile.h"

namespace GLframework
{
	struct EnvironmentProfile;
	struct PostProcessSettings;
}

namespace GL_SCENE
{
	struct PBRCameraRigProfile;
}

namespace GL_RUNTIME
{
	struct RuntimeProfileState
	{
		RuntimeProfileState();
		~RuntimeProfileState();

		RuntimeProfileState(const RuntimeProfileState&) = delete;
		RuntimeProfileState& operator=(const RuntimeProfileState&) = delete;
		RuntimeProfileState(RuntimeProfileState&&) noexcept;
		RuntimeProfileState& operator=(RuntimeProfileState&&) noexcept;

		RuntimeFramePipelineProfile framePipelineProfile{};
		std::string framePipelineProfilePath{};
		std::string rendererFramePassProfilePath{};
		std::string postProcessSettingsPath{};
		std::string environmentProfilePath{};
		GL_SCENE::PBRPreviewProfile pbrPreviewProfile{};
		GL_SCENE::PBRLightRigProfile pbrLightRigProfile{};
		std::string pbrPreviewProfilePath{};
		std::string pbrExperimentProfilePath{};

		GLframework::PostProcessSettings& postProcessSettings();
		const GLframework::PostProcessSettings& postProcessSettings() const;
		GLframework::EnvironmentProfile& environmentProfile();
		const GLframework::EnvironmentProfile& environmentProfile() const;
		GL_SCENE::PBRCameraRigProfile& pbrCameraRigProfile();
		const GL_SCENE::PBRCameraRigProfile& pbrCameraRigProfile() const;

	private:
		std::unique_ptr<GLframework::PostProcessSettings> mPostProcessSettings{};
		std::unique_ptr<GLframework::EnvironmentProfile> mEnvironmentProfile{};
		std::unique_ptr<GL_SCENE::PBRCameraRigProfile> mPBRCameraRigProfile{};
	};
}

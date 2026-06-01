#pragma once

#include <memory>
#include <string>

namespace GLframework
{
	struct EnvironmentProfile;
	struct PostProcessSettings;
}

namespace GL_SCENE
{
	struct PBRCameraRigProfile;
	struct PBRLightRigProfile;
	struct PBRPreviewProfile;
}

namespace GL_EDITOR
{
	struct EditorUiModuleProfile;
}

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineProfile;

	struct RuntimeProfileState
	{
		RuntimeProfileState();
		~RuntimeProfileState();

		RuntimeProfileState(const RuntimeProfileState&) = delete;
		RuntimeProfileState& operator=(const RuntimeProfileState&) = delete;
		RuntimeProfileState(RuntimeProfileState&&) noexcept;
		RuntimeProfileState& operator=(RuntimeProfileState&&) noexcept;

		std::string framePipelineProfilePath{};
		std::string rendererFramePassProfilePath{};
		std::string postProcessSettingsPath{};
		std::string environmentProfilePath{};
		std::string pbrPreviewProfilePath{};
		std::string pbrExperimentProfilePath{};
		std::string editorUiModuleProfilePath{};

		RuntimeFramePipelineProfile& framePipelineProfile();
		const RuntimeFramePipelineProfile& framePipelineProfile() const;
		GLframework::PostProcessSettings& postProcessSettings();
		const GLframework::PostProcessSettings& postProcessSettings() const;
		GLframework::EnvironmentProfile& environmentProfile();
		const GLframework::EnvironmentProfile& environmentProfile() const;
		GL_SCENE::PBRLightRigProfile& pbrLightRigProfile();
		const GL_SCENE::PBRLightRigProfile& pbrLightRigProfile() const;
		GL_SCENE::PBRPreviewProfile& pbrPreviewProfile();
		const GL_SCENE::PBRPreviewProfile& pbrPreviewProfile() const;
		GL_SCENE::PBRCameraRigProfile& pbrCameraRigProfile();
		const GL_SCENE::PBRCameraRigProfile& pbrCameraRigProfile() const;
		GL_EDITOR::EditorUiModuleProfile& editorUiModuleProfile();
		const GL_EDITOR::EditorUiModuleProfile& editorUiModuleProfile() const;

	private:
		std::unique_ptr<RuntimeFramePipelineProfile> mFramePipelineProfile{};
		std::unique_ptr<GLframework::PostProcessSettings> mPostProcessSettings{};
		std::unique_ptr<GLframework::EnvironmentProfile> mEnvironmentProfile{};
		std::unique_ptr<GL_SCENE::PBRLightRigProfile> mPBRLightRigProfile{};
		std::unique_ptr<GL_SCENE::PBRPreviewProfile> mPBRPreviewProfile{};
		std::unique_ptr<GL_SCENE::PBRCameraRigProfile> mPBRCameraRigProfile{};
		std::unique_ptr<GL_EDITOR::EditorUiModuleProfile> mEditorUiModuleProfile{};
	};
}

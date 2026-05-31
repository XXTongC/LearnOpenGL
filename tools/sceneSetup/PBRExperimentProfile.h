#pragma once

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

	class PBRExperimentProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(
			const std::string& path,
			GLframework::EnvironmentProfile& environmentProfile,
			GLframework::PostProcessSettings& postProcessSettings,
			PBRPreviewProfile& pbrPreviewProfile,
			PBRLightRigProfile& lightRigProfile,
			PBRCameraRigProfile& cameraRigProfile
		);
		static bool saveToFile(
			const std::string& path,
			const GLframework::EnvironmentProfile& environmentProfile,
			const GLframework::PostProcessSettings& postProcessSettings,
			const PBRPreviewProfile& pbrPreviewProfile,
			const PBRLightRigProfile& lightRigProfile,
			const PBRCameraRigProfile& cameraRigProfile
		);
	};
}

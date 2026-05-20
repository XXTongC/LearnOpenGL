#pragma once

#include <memory>
#include <string>
#include <vector>

class Camera;

namespace GLframework
{
	class DirectionalLight;
	class AmbientLight;
	struct EnvironmentProfile;
	class Mesh;
	class PointLight;
	class SpotLight;
	struct PostProcessSettings;
	class Renderer;
}

namespace GL_SCENE
{
	struct PBRCameraRigProfile;
	struct PBRLightRigProfile;
	struct PBRPreviewProfile;
}

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineProfile;
}

namespace GL_EDITOR
{
	struct DebugControllerContext
	{
		std::shared_ptr<GLframework::DirectionalLight>* directionalLight{ nullptr };
		std::shared_ptr<GLframework::AmbientLight>* ambientLight{ nullptr };
		std::shared_ptr<GLframework::SpotLight>* spotLight{ nullptr };
		std::vector<std::shared_ptr<GLframework::PointLight>>* pointLights{ nullptr };
		std::shared_ptr<GLframework::Mesh> textObject{ nullptr };
		GL_RUNTIME::RuntimeFramePipelineProfile* framePipelineProfile{ nullptr };
		const std::string* framePipelineProfilePath{ nullptr };
		GLframework::PostProcessSettings* postProcessSettings{ nullptr };
		const std::string* postProcessSettingsPath{ nullptr };
		std::shared_ptr<GLframework::Renderer> renderer{ nullptr };
		GLframework::EnvironmentProfile* environmentProfile{ nullptr };
		const std::string* environmentProfilePath{ nullptr };
		GL_SCENE::PBRPreviewProfile* pbrPreviewProfile{ nullptr };
		const std::string* pbrPreviewProfilePath{ nullptr };
		const std::string* pbrExperimentProfilePath{ nullptr };
		GL_SCENE::PBRLightRigProfile* lightRigProfile{ nullptr };
		GL_SCENE::PBRCameraRigProfile* cameraRigProfile{ nullptr };
		Camera* mainCamera{ nullptr };
		float* orbitAngle{ nullptr };
	};

	void drawDebugControllerPanel(const DebugControllerContext& context);
}

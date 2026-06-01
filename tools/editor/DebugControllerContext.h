#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

class Camera;

namespace GLframework
{
	class AmbientLight;
	class DirectionalLight;
	struct EnvironmentProfile;
	class Mesh;
	class PointLight;
	struct PostProcessSettings;
	class Renderer;
	class SpotLight;
}

namespace GLengine
{
	class AssetSubsystem;
	class Engine;
	class RendererSubsystem;
	class World;
}

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineProfile;
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
	struct EditorUiModuleRegistries;

	using EditorUiModuleProfileApplyRequest = std::function<bool(const EditorUiModuleProfile&)>;

	struct DebugControllerContext
	{
		std::shared_ptr<GLframework::DirectionalLight>* directionalLight{ nullptr };
		std::shared_ptr<GLframework::AmbientLight>* ambientLight{ nullptr };
		std::shared_ptr<GLframework::SpotLight>* spotLight{ nullptr };
		std::vector<std::shared_ptr<GLframework::PointLight>>* pointLights{ nullptr };
		std::shared_ptr<GLframework::Mesh> textObject{ nullptr };
		GL_RUNTIME::RuntimeFramePipelineProfile* framePipelineProfile{ nullptr };
		const std::string* framePipelineProfilePath{ nullptr };
		const std::string* rendererFramePassProfilePath{ nullptr };
		GLframework::PostProcessSettings* postProcessSettings{ nullptr };
		const std::string* postProcessSettingsPath{ nullptr };
		std::shared_ptr<GLframework::Renderer> renderer{ nullptr };
		const GLengine::RendererSubsystem* rendererSubsystem{ nullptr };
		GLframework::EnvironmentProfile* environmentProfile{ nullptr };
		const std::string* environmentProfilePath{ nullptr };
		GL_SCENE::PBRPreviewProfile* pbrPreviewProfile{ nullptr };
		const std::string* pbrPreviewProfilePath{ nullptr };
		const std::string* pbrExperimentProfilePath{ nullptr };
		GL_SCENE::PBRLightRigProfile* lightRigProfile{ nullptr };
		GL_SCENE::PBRCameraRigProfile* cameraRigProfile{ nullptr };
		Camera* mainCamera{ nullptr };
		float* orbitAngle{ nullptr };
		const GLengine::Engine* engine{ nullptr };
		const GLengine::World* engineWorld{ nullptr };
		const GLengine::AssetSubsystem* assetSubsystem{ nullptr };
		EditorUiModuleProfile* editorUiModuleProfile{ nullptr };
		const std::string* editorUiModuleProfilePath{ nullptr };
		EditorUiModuleProfileApplyRequest requestEditorUiModuleProfileApply{};
		const EditorUiModuleRegistries* editorUiModules{ nullptr };
	};
}

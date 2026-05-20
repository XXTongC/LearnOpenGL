#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../renderer/EnvironmentProfile.h"
#include "../renderer/FrameRenderTargets.h"
#include "../renderer/PostProcessPass.h"
#include "../renderer/PostProcessSettings.h"
#include "../third_party/glm/glm.hpp"
#include "../tools/sceneSetup/PBRCameraRigProfile.h"
#include "../tools/sceneSetup/PBRExperimentProfile.h"
#include "../tools/sceneSetup/PBRLightRigProfile.h"
#include "../tools/sceneSetup/PBRPreviewProfile.h"

class Camera;
class CameraControl;

namespace GLframework
{
	class AmbientLight;
	class Bloom;
	class DirectionalLight;
	class GrassInstanceMaterial;
	class Mesh;
	class PhongCSMShadowMaterial;
	class PointLight;
	class Renderer;
	class Scene;
	class ScreenMaterial;
	class SpotLight;

	struct AppRuntimeContext
	{
		std::shared_ptr<Renderer> renderer{ nullptr };
		std::shared_ptr<Scene> sceneOffScreen{ nullptr };
		std::shared_ptr<Scene> sceneInScreen{ nullptr };
		std::shared_ptr<Mesh> meshPointLight{ nullptr };
		std::shared_ptr<Mesh> screenQuad{ nullptr };
		std::shared_ptr<AmbientLight> ambientLight{ nullptr };
		FrameRenderTargets frameRenderTargets{};
		std::shared_ptr<Bloom> bloom{ nullptr };
		std::shared_ptr<GrassInstanceMaterial> grassMaterial{ nullptr };
		std::shared_ptr<Mesh> skyBoxMesh{ nullptr };
		std::shared_ptr<Mesh> movePlane{ nullptr };
		std::shared_ptr<Mesh> textD{ nullptr };
		std::shared_ptr<ScreenMaterial> screenMaterial{ nullptr };
		std::shared_ptr<PhongCSMShadowMaterial> csmShadowMaterial{ nullptr };
		PostProcessPass postProcessPass{};
		PostProcessSettings postProcessSettings{};
		std::string postProcessSettingsPath{ PostProcessSettingsStorage::defaultPath() };
		EnvironmentProfile environmentProfile{};
		std::string environmentProfilePath{ EnvironmentProfileStorage::defaultPath() };
		GL_SCENE::PBRPreviewProfile pbrPreviewProfile{};
		GL_SCENE::PBRLightRigProfile pbrLightRigProfile{};
		GL_SCENE::PBRCameraRigProfile pbrCameraRigProfile{};
		std::string pbrPreviewProfilePath{ GL_SCENE::PBRPreviewProfileStorage::defaultPath() };
		std::string pbrExperimentProfilePath{ GL_SCENE::PBRExperimentProfileStorage::defaultPath() };
		Camera* camera{ nullptr };
		CameraControl* cameracontrol{ nullptr };
		glm::vec3 clearColor{};
		std::shared_ptr<DirectionalLight> dirLight{ nullptr };
		std::shared_ptr<SpotLight> spotLight{ nullptr };
		std::vector<std::shared_ptr<PointLight>> pointLights{};
	};
}

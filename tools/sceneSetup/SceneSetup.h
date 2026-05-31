#pragma once

#include <memory>
#include <string>
#include <vector>

namespace GLengine
{
	class Engine;
	class World;
}

namespace GLframework
{
	class AmbientLight;
	class Bloom;
	class DirectionalLight;
	struct EnvironmentProfile;
	class FrameRenderTargets;
	class Mesh;
	class PointLight;
	class Renderer;
	class Scene;
	class ScreenMaterial;
	class SpotLight;
}

namespace GL_SCENE
{
	struct PBRLightRigProfile;
	struct PBRPreviewProfile;

	struct SetupContext
	{
		std::shared_ptr<GLframework::Renderer>& renderer;
		std::shared_ptr<GLframework::Scene>& sceneOffScreen;
		std::shared_ptr<GLframework::Scene>& sceneInScreen;
		GLframework::FrameRenderTargets& frameRenderTargets;
		std::shared_ptr<GLframework::Bloom>& bloom;
		std::shared_ptr<GLframework::Mesh>& screenQuad;
		std::shared_ptr<GLframework::Mesh>& skyBoxMesh;
		std::shared_ptr<GLframework::Mesh>& textD;
		std::shared_ptr<GLframework::ScreenMaterial>& screenMaterial;
		std::shared_ptr<GLframework::AmbientLight>& ambientLight;
		std::shared_ptr<GLframework::DirectionalLight>& dirLight;
		std::shared_ptr<GLframework::SpotLight>& spotLight;
		std::vector<std::shared_ptr<GLframework::PointLight>>& pointLights;
		int width{ 0 };
		int height{ 0 };
		const std::string& texturePath;
		GLframework::EnvironmentProfile& environmentProfile;
		PBRPreviewProfile& pbrPreviewProfile;
		PBRLightRigProfile& lightRigProfile;
		GLengine::Engine* engine{ nullptr };
		GLengine::World*& engineWorld;
		bool& engineWorldEditable;
	};

	void prepareDefaultScene(SetupContext& context);
	void prepareSceneInfrastructure(SetupContext& context);
	void prepareLegacyDefaultSceneContent(SetupContext& context);
	void prepareConfiguredPBRPreview(SetupContext& context);
}

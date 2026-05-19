#pragma once

#include "../../framework/scene.h"
#include "../../light/ambientLight.h"
#include "../../light/directionalLight.h"
#include "../../light/pointLight.h"
#include "../../light/spotLight.h"
#include "../../materials/screenMaterial.h"
#include "../../mesh/mesh.h"
#include "../../renderer/FrameRenderTargets.h"
#include "../../renderer/renderer.h"

namespace GL_SCENE
{
	struct SetupContext
	{
		std::shared_ptr<GLframework::Renderer>& renderer;
		std::shared_ptr<GLframework::Scene>& sceneOffScreen;
		std::shared_ptr<GLframework::Scene>& sceneInScreen;
		GLframework::FrameRenderTargets& frameRenderTargets;
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
	};

	void prepareDefaultScene(SetupContext& context);
}

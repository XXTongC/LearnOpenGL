#pragma once

#include <memory>
#include <vector>

#include "RendererFrameStats.h"

class Camera;

namespace GLframework
{
	class AmbientLight;
	class DirectionalLight;
	class EnvironmentRenderTargets;
	class FrameRenderState;
	class Material;
	class PBRDepthPrepass;
	class PBRSceneRenderPass;
	class PointLight;
	class RenderQueue;
	class Scene;
	class SceneRenderPass;
	class ShaderLibrary;
	class ShadowRenderer;
	class SpotLight;

	struct RendererFrameContext
	{
		std::shared_ptr<Scene> scene{ nullptr };
		Camera* camera{ nullptr };
		std::shared_ptr<DirectionalLight> dirLight{ nullptr };
		std::shared_ptr<SpotLight> spotLight{ nullptr };
		const std::vector<std::shared_ptr<PointLight>>* pointLights{ nullptr };
		std::shared_ptr<AmbientLight> ambient{ nullptr };
		std::shared_ptr<Material> globalMaterial{ nullptr };
		unsigned int fbo{ 0 };

		FrameRenderState* frameRenderState{ nullptr };
		RenderQueue* renderQueue{ nullptr };
		ShadowRenderer* shadowRenderer{ nullptr };
		SceneRenderPass* sceneRenderPass{ nullptr };
		PBRDepthPrepass* pbrDepthPrepass{ nullptr };
		PBRSceneRenderPass* pbrSceneRenderPass{ nullptr };
		ShaderLibrary* shaderLibrary{ nullptr };
		EnvironmentRenderTargets* environmentTargets{ nullptr };
		RendererFrameStats* stats{ nullptr };
	};
}

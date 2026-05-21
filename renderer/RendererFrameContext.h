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
	class IBLDebugPass;
	class Material;
	class PBRDepthPrepass;
	class PBRDeferredClusteredLightDebugPass;
	class PBRDeferredLightingPass;
	class PBRDeferredTiledLightDebugPass;
	class PBRGBufferPass;
	class PBRGBufferDebugPass;
	class PBRGBufferRenderTargets;
	class PBRSceneRenderPass;
	class PBRShadowAtlasRenderPass;
	class PBRShadowAtlasRenderTargets;
	class PointLight;
	class RenderQueue;
	class RendererGpuTimerQueryPool;
	class Scene;
	class SceneRenderPass;
	class ShaderLibrary;
	class ShadowRenderer;
	class SpotLight;
	struct RendererFramePassProfile;

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
		PBRGBufferPass* pbrGBufferPass{ nullptr };
		PBRDeferredLightingPass* pbrDeferredLightingPass{ nullptr };
		PBRDeferredTiledLightDebugPass* pbrDeferredTiledLightDebugPass{ nullptr };
		PBRDeferredClusteredLightDebugPass* pbrDeferredClusteredLightDebugPass{ nullptr };
		PBRGBufferDebugPass* pbrGBufferDebugPass{ nullptr };
		PBRSceneRenderPass* pbrSceneRenderPass{ nullptr };
		IBLDebugPass* iblDebugPass{ nullptr };
		PBRShadowAtlasRenderPass* pbrShadowAtlasPass{ nullptr };
		PBRShadowAtlasRenderTargets* pbrShadowAtlasTargets{ nullptr };
		PBRGBufferRenderTargets* pbrGBufferTargets{ nullptr };
		ShaderLibrary* shaderLibrary{ nullptr };
		EnvironmentRenderTargets* environmentTargets{ nullptr };
		RendererFramePassProfile* framePassProfile{ nullptr };
		RendererGpuTimerQueryPool* gpuTimerQueries{ nullptr };
		RendererFrameStats* stats{ nullptr };
	};
}

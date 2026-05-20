#pragma once

namespace GLframework
{
	struct RendererFrameStats
	{
		int rendererPassCount{ 0 };
		int shadowCasterCount{ 0 };
		int directionalShadowLayerCount{ 0 };
		int directionalShadowDrawCalls{ 0 };
		int pointShadowLightCount{ 0 };
		int pointShadowFaceCount{ 0 };
		int pointShadowDrawCalls{ 0 };
		bool pbrShadowAtlasReady{ false };
		int pbrShadowAtlasDirectionalLayers{ 0 };
		int pbrShadowAtlasPointLights{ 0 };
		int pbrShadowAtlasPointFaces{ 0 };
		int pbrShadowAtlasPointFacesRendered{ 0 };
		int pbrShadowAtlasDirectionalDrawCalls{ 0 };
		int pbrShadowAtlasPointDrawCalls{ 0 };
		int pbrShadowAtlasDirectionalResolution{ 0 };
		int pbrShadowAtlasPointResolution{ 0 };
		int pbrDepthPrepassDrawCalls{ 0 };
		int pbrGBufferDrawCalls{ 0 };
		int pbrGBufferWidth{ 0 };
		int pbrGBufferHeight{ 0 };
		bool pbrGBufferReady{ false };
		int pbrDeferredLightingDrawCalls{ 0 };
		bool pbrDeferredCsmShadowBound{ false };
		int pbrDeferredCsmShadowLayers{ 0 };
		bool pbrDeferredCsmShadowAtlasBound{ false };
		bool pbrDeferredLightBufferBound{ false };
		int pbrDeferredLightBufferPointLights{ 0 };
		int pbrDeferredLightBufferMaxPointLights{ 0 };
		int pbrGBufferDebugDrawCalls{ 0 };
		int legacySceneDrawCalls{ 0 };
		int pbrSceneDrawCalls{ 0 };
		int iblDebugDrawCalls{ 0 };
	};
}

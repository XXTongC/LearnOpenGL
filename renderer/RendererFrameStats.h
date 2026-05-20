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
		int pbrDepthPrepassDrawCalls{ 0 };
		int legacySceneDrawCalls{ 0 };
		int pbrSceneDrawCalls{ 0 };
	};
}

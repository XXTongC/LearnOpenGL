#pragma once

namespace GLframework
{
	struct ShadowRenderStats
	{
		int directionalLayerCount{ 0 };
		int directionalDrawCalls{ 0 };
		int pointLightCount{ 0 };
		int pointFaceCount{ 0 };
		int pointDrawCalls{ 0 };

		int totalDrawCalls() const
		{
			return directionalDrawCalls + pointDrawCalls;
		}
	};
}

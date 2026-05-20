#pragma once

#include <memory>
#include <vector>

#include "core.h"

namespace GLframework
{
	class DirectionalLight;
	class PointLight;

	struct PBRShadowAtlasStats
	{
		bool ready{ false };
		bool directionalReady{ false };
		bool pointReady{ false };
		int directionalLayerCount{ 0 };
		int pointLightCount{ 0 };
		int pointFaceCount{ 0 };
		int pointFacesRendered{ 0 };
		int directionalDrawCalls{ 0 };
		int pointDrawCalls{ 0 };
		int directionalResolution{ 0 };
		int pointResolution{ 0 };
		unsigned int framebuffer{ 0 };
		unsigned int directionalDepthTexture{ 0 };
		unsigned int pointDepthTexture{ 0 };
	};

	class PBRShadowAtlasRenderTargets
	{
	public:
		~PBRShadowAtlasRenderTargets();

		PBRShadowAtlasStats prepare(
			const std::shared_ptr<DirectionalLight>& dirLight,
			const std::vector<std::shared_ptr<PointLight>>& pointLights
		);
		void resetFrameStats();

		const PBRShadowAtlasStats& getLastStats() const;
		unsigned int getFramebuffer() const;
		unsigned int getDirectionalDepthTexture() const;
		unsigned int getPointDepthTexture() const;

		static constexpr int maxDirectionalLayers()
		{
			return 20;
		}

		static constexpr int maxPointLights()
		{
			return 16;
		}

	private:
		void ensureFramebuffer();
		void ensureDirectionalAtlas(int layerCount);
		void ensurePointAtlas(int pointLightCount);
		void releaseTexture(unsigned int& texture, int& layerCapacity);
		unsigned int createDepthArrayTexture(int resolution, int layers) const;

		unsigned int mFramebuffer{ 0 };
		unsigned int mDirectionalDepthTexture{ 0 };
		unsigned int mPointDepthTexture{ 0 };
		int mDirectionalLayerCapacity{ 0 };
		int mPointFaceLayerCapacity{ 0 };
		PBRShadowAtlasStats mLastStats{};
	};
}

#pragma once

#include <memory>

#include "MaterialBindingContext.h"
#include "mesh/mesh.h"
#include "renderer/PBRDeferredLightBuffer.h"
#include "renderer/PBRDeferredTiledLightGrid.h"

namespace GLframework
{
	class PBRGBufferRenderTargets;
	struct RendererFramePassProfile;
	class Shader;
	class ShaderLibrary;

	struct PBRDeferredLightingPassStats
	{
		int drawCalls{ 0 };
		bool csmShadowBound{ false };
		int csmLayerCount{ 0 };
		bool csmShadowAtlasBound{ false };
		bool pointShadowAtlasBound{ false };
		int pointShadowAtlasLightCount{ 0 };
		bool lightBufferBound{ false };
		int lightBufferPointLightCount{ 0 };
		int lightBufferMaxPointLightCount{ 0 };
		bool tiledLightGridBound{ false };
		bool tiledLightGridEnabled{ false };
		int tiledLightGridTileSize{ 0 };
		int tiledLightGridColumns{ 0 };
		int tiledLightGridRows{ 0 };
		int tiledLightGridTileCount{ 0 };
		int tiledLightGridIndexCount{ 0 };
		int tiledLightGridOccupiedTileCount{ 0 };
		int tiledLightGridEmptyTileCount{ 0 };
		int tiledLightGridMaxTileLightCount{ 0 };
	};

	class PBRDeferredLightingPass
	{
	public:
		PBRDeferredLightingPassStats render(
			const PBRGBufferRenderTargets& targets,
			const MaterialBindingContext& context,
			const RendererFramePassProfile& profile,
			ShaderLibrary& shaderLibrary
		);

	private:
		PBRDeferredLightingPassStats bindFrameUniforms(
			const std::shared_ptr<Shader>& shader,
			const MaterialBindingContext& context,
			const RendererFramePassProfile& profile,
			unsigned int targetWidth,
			unsigned int targetHeight
		);
		void bindGBufferTextures(
			const std::shared_ptr<Shader>& shader,
			const PBRGBufferRenderTargets& targets
		) const;
		void ensureLightingQuad(const std::shared_ptr<Shader>& shader);

		std::shared_ptr<Mesh> mLightingQuad{ nullptr };
		PBRDeferredLightBuffer mLightBuffer{};
		PBRDeferredTiledLightGrid mTiledLightGrid{};
	};
}

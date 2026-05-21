#pragma once

#include <vector>

#include "core.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/PBRDeferredLightCullingConfig.h"

namespace GLframework
{
	struct PBRDeferredTiledLightGridStats
	{
		bool bound{ false };
		bool enabled{ false };
		int tileSize{ 0 };
		float lightCutoff{ 0.0f };
		int tileColumns{ 0 };
		int tileRows{ 0 };
		int tileCount{ 0 };
		int pointLightCount{ 0 };
		int fullLightIndexCount{ 0 };
		int lightIndexCount{ 0 };
		int culledLightIndexCount{ 0 };
		int occupiedTileCount{ 0 };
		int emptyTileCount{ 0 };
		int maxTileLightCount{ 0 };
		unsigned int tileBufferBindingPoint{ 0 };
		unsigned int indexBufferBindingPoint{ 0 };
	};

	class PBRDeferredTiledLightGrid
	{
	public:
		~PBRDeferredTiledLightGrid();

		PBRDeferredTiledLightGridStats bind(
			const MaterialBindingContext& context,
			unsigned int targetWidth,
			unsigned int targetHeight,
			const PBRDeferredLightCullingConfig& config
		);

		static constexpr unsigned int tileBufferBindingPoint()
		{
			return 4;
		}

		static constexpr unsigned int indexBufferBindingPoint()
		{
			return 5;
		}

	private:
		struct TileLightEntry
		{
			int tileIndex{ 0 };
			int lightIndex{ 0 };
		};

		void ensureBuffers();

		unsigned int mTileBuffer{ 0 };
		unsigned int mIndexBuffer{ 0 };
		std::vector<int> mTileLightCounts{};
		std::vector<TileLightEntry> mTileLightEntries{};
		std::vector<glm::ivec4> mTileOffsetCount{};
		std::vector<int> mLightIndices{};
		std::vector<int> mTileWriteOffsets{};
	};
}

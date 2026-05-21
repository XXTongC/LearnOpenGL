#pragma once

#include "renderer/MaterialBindingContext.h"

namespace GLframework
{
	struct PBRDeferredTiledLightGridStats
	{
		bool bound{ false };
		bool enabled{ false };
		int tileSize{ 0 };
		int tileColumns{ 0 };
		int tileRows{ 0 };
		int tileCount{ 0 };
		int lightIndexCount{ 0 };
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
			int tileSize
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
		void ensureBuffers();

		unsigned int mTileBuffer{ 0 };
		unsigned int mIndexBuffer{ 0 };
	};
}

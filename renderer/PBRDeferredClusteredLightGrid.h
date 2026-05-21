#pragma once

#include <vector>

#include "core.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/PBRDeferredLightCullingConfig.h"

namespace GLframework
{
	class PBRDeferredClusteredLightGrid
	{
	public:
		~PBRDeferredClusteredLightGrid();

		PBRDeferredClusteredLightGridStats bind(
			const MaterialBindingContext& context,
			unsigned int targetWidth,
			unsigned int targetHeight,
			const PBRDeferredLightCullingConfig& config
		);

		static constexpr unsigned int clusterBufferBindingPoint()
		{
			return 6;
		}

		static constexpr unsigned int indexBufferBindingPoint()
		{
			return 7;
		}

	private:
		struct ClusterLightEntry
		{
			int clusterIndex{ 0 };
			int lightIndex{ 0 };
		};

		void ensureBuffers();

		unsigned int mClusterBuffer{ 0 };
		unsigned int mIndexBuffer{ 0 };
		std::vector<int> mClusterLightCounts{};
		std::vector<ClusterLightEntry> mClusterLightEntries{};
		std::vector<glm::ivec4> mClusterOffsetCount{};
		std::vector<int> mLightIndices{};
		std::vector<int> mClusterWriteOffsets{};
	};
}

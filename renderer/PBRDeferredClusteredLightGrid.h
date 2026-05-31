#pragma once

#include <memory>
#include <vector>

#include "renderer/PBRDeferredLightCullingConfig.h"
#include "third_party/glm/glm.hpp"

namespace GLframework
{
	struct MaterialBindingContext;
	class Shader;

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

		PBRDeferredClusteredLightGridStats bindCompute(
			const MaterialBindingContext& context,
			unsigned int targetWidth,
			unsigned int targetHeight,
			const PBRDeferredLightCullingConfig& config,
			const std::shared_ptr<Shader>& computeShader
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

#include "PBRDeferredLightCullingConfig.h"

#include <algorithm>

#include "RendererFramePassProfile.h"

using namespace GLframework;

PBRDeferredLightCullingConfig GLframework::makePbrDeferredLightCullingConfig(
	const RendererFramePassProfile& profile,
	PBRDeferredLightCullingMode mode
)
{
	return PBRDeferredLightCullingConfig{
		mode,
		std::max(profile.pbrDeferredTileSize, 1),
		std::max(profile.pbrDeferredTiledLightCutoff, 0.001f),
		std::max(profile.pbrDeferredClusteredDepthSlices, 1),
		std::max(profile.pbrDeferredClusteredMaxLightsPerCluster, 1),
		profile.pbrDeferredClusteredStatsReadbackEnabled
	};
}

PBRDeferredClusteredLightGridLayout GLframework::makePbrDeferredClusteredLightGridLayout(
	unsigned int targetWidth,
	unsigned int targetHeight,
	const PBRDeferredLightCullingConfig& config
)
{
	const int safeTileSize = std::max(config.tileSize, 1);
	const int columns = static_cast<int>((targetWidth + static_cast<unsigned int>(safeTileSize) - 1) / static_cast<unsigned int>(safeTileSize));
	const int rows = static_cast<int>((targetHeight + static_cast<unsigned int>(safeTileSize) - 1) / static_cast<unsigned int>(safeTileSize));
	const int depthSlices = std::max(config.clusterDepthSlices, 1);
	const int clusterCount = std::max(columns * rows * depthSlices, 0);
	const int maxLightsPerCluster = std::max(config.maxLightsPerCluster, 1);
	return PBRDeferredClusteredLightGridLayout{
		safeTileSize,
		columns,
		rows,
		depthSlices,
		clusterCount,
		maxLightsPerCluster,
		clusterCount * maxLightsPerCluster,
		6,
		7
	};
}

bool GLframework::usesPbrDeferredCpuTiledLightGrid(const PBRDeferredLightCullingConfig& config)
{
	return config.mode == PBRDeferredLightCullingMode::CpuTiled;
}

bool GLframework::usesPbrDeferredGpuClusteredLightGrid(const PBRDeferredLightCullingConfig& config)
{
	return config.mode == PBRDeferredLightCullingMode::GpuClustered;
}

#pragma once

namespace GLframework
{
	struct RendererFramePassProfile;

	enum class PBRDeferredLightCullingMode
	{
		Disabled,
		CpuTiled,
		GpuClustered
	};

	struct PBRDeferredLightCullingConfig
	{
		PBRDeferredLightCullingMode mode{ PBRDeferredLightCullingMode::Disabled };
		int tileSize{ 16 };
		float lightCutoff{ 0.01f };
		int clusterDepthSlices{ 24 };
		int maxLightsPerCluster{ 64 };
		bool clusteredStatsReadbackEnabled{ false };
	};

	struct PBRDeferredClusteredLightGridLayout
	{
		int tileSize{ 16 };
		int clusterColumns{ 0 };
		int clusterRows{ 0 };
		int clusterDepthSlices{ 0 };
		int clusterCount{ 0 };
		int maxLightsPerCluster{ 0 };
		int maxLightIndexCount{ 0 };
		unsigned int clusterBufferBindingPoint{ 6 };
		unsigned int indexBufferBindingPoint{ 7 };
	};

	struct PBRDeferredClusteredLightGridStats
	{
		bool enabled{ false };
		bool bound{ false };
		bool computeDispatched{ false };
		bool statsReadbackEnabled{ false };
		bool lightIndexStatsAvailable{ false };
		PBRDeferredClusteredLightGridLayout layout{};
		int pointLightCount{ 0 };
		int lightIndexCount{ 0 };
		int culledLightIndexCount{ 0 };
	};

	PBRDeferredLightCullingConfig makePbrDeferredLightCullingConfig(
		const RendererFramePassProfile& profile,
		PBRDeferredLightCullingMode mode
	);

	PBRDeferredClusteredLightGridLayout makePbrDeferredClusteredLightGridLayout(
		unsigned int targetWidth,
		unsigned int targetHeight,
		const PBRDeferredLightCullingConfig& config
	);

	bool usesPbrDeferredCpuTiledLightGrid(const PBRDeferredLightCullingConfig& config);
	bool usesPbrDeferredGpuClusteredLightGrid(const PBRDeferredLightCullingConfig& config);
}

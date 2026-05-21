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
	};

	PBRDeferredLightCullingConfig makePbrDeferredLightCullingConfig(
		const RendererFramePassProfile& profile,
		PBRDeferredLightCullingMode mode
	);

	bool usesPbrDeferredCpuTiledLightGrid(const PBRDeferredLightCullingConfig& config);
}

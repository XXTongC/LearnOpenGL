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
		std::max(profile.pbrDeferredTiledLightCutoff, 0.001f)
	};
}

bool GLframework::usesPbrDeferredCpuTiledLightGrid(const PBRDeferredLightCullingConfig& config)
{
	return config.mode == PBRDeferredLightCullingMode::CpuTiled;
}

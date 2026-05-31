#include "PBRDeferredClusteredLightDebugPass.h"

#include <algorithm>

#include "core.h"
#include "framework/geometry.h"
#include "mesh/mesh.h"
#include "renderer/MeshDraw.h"
#include "renderer/PBRDeferredLightCullingConfig.h"
#include "renderer/RendererFramePassProfile.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

PBRDeferredClusteredLightDebugPassStats PBRDeferredClusteredLightDebugPass::render(
	const MaterialBindingContext& context,
	const RendererFramePassProfile& profile,
	ShaderLibrary& shaderLibrary,
	unsigned int targetWidth,
	unsigned int targetHeight
)
{
	PBRDeferredClusteredLightDebugPassStats stats{};
	const auto shader = shaderLibrary.getPbrDeferredClusteredLightDebugShader();
	if (!shader || targetWidth == 0 || targetHeight == 0)
	{
		return stats;
	}

	ensureDebugQuad(shader);
	if (!mDebugQuad)
	{
		return stats;
	}

	PBRDeferredLightCullingConfig clusteredGridConfig = makePbrDeferredLightCullingConfig(
		profile,
		PBRDeferredLightCullingMode::GpuClustered
	);
	clusteredGridConfig.clusteredStatsReadbackEnabled = false;
	stats.gridStats = mClusteredLightGrid.bindCompute(
		context,
		targetWidth,
		targetHeight,
		clusteredGridConfig,
		shaderLibrary.getPbrDeferredClusteredLightGridComputeShader()
	);
	if (!stats.gridStats.bound)
	{
		return stats;
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	shader->begin();
	shader->setInt("clusteredLightTileSize", stats.gridStats.layout.tileSize);
	shader->setInt("clusteredLightGridColumns", stats.gridStats.layout.clusterColumns);
	shader->setInt("clusteredLightGridRows", stats.gridStats.layout.clusterRows);
	shader->setInt("clusteredLightDepthSlices", stats.gridStats.layout.clusterDepthSlices);
	shader->setInt("clusteredLightDebugDepthSlice", profile.pbrDeferredClusteredLightDebugDepthSlice);
	shader->setInt("clusteredLightDebugMaxLights", std::max(profile.pbrDeferredClusteredLightDebugMaxLights, 1));
	shader->setFloat("clusteredLightDebugIntensity", profile.pbrDeferredClusteredLightDebugIntensity);
	const bool drawn = MeshDraw::drawIndexed(mDebugQuad);
	shader->end();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	stats.drawCalls = drawn ? 1 : 0;
	return stats;
}

void PBRDeferredClusteredLightDebugPass::ensureDebugQuad(const std::shared_ptr<Shader>& shader)
{
	if (mDebugQuad || !shader)
	{
		return;
	}

	auto geometry = Geometry::createScreenPlane(shader);
	mDebugQuad = std::make_shared<Mesh>(geometry, nullptr);
	mDebugQuad->setName("PBR Deferred Clustered Light Debug Quad");
}

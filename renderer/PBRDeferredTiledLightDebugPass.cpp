#include "PBRDeferredTiledLightDebugPass.h"

#include <algorithm>

#include "core.h"
#include "framework/geometry.h"
#include "renderer/MeshDraw.h"
#include "renderer/RendererFramePassProfile.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

PBRDeferredTiledLightDebugPassStats PBRDeferredTiledLightDebugPass::render(
	const MaterialBindingContext& context,
	const RendererFramePassProfile& profile,
	ShaderLibrary& shaderLibrary,
	unsigned int targetWidth,
	unsigned int targetHeight
)
{
	PBRDeferredTiledLightDebugPassStats stats{};
	const auto shader = shaderLibrary.getPbrDeferredTiledLightDebugShader();
	if (!shader || targetWidth == 0 || targetHeight == 0)
	{
		return stats;
	}

	ensureDebugQuad(shader);
	if (!mDebugQuad)
	{
		return stats;
	}

	const PBRDeferredTiledLightGridConfig tiledGridConfig{
		profile.pbrDeferredTileSize,
		profile.pbrDeferredTiledLightCutoff
	};
	stats.gridStats = mTiledLightGrid.bind(
		context,
		targetWidth,
		targetHeight,
		tiledGridConfig
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
	shader->setInt("tiledLightTileSize", stats.gridStats.tileSize);
	shader->setInt("tiledLightGridColumns", stats.gridStats.tileColumns);
	shader->setInt("tiledLightGridRows", stats.gridStats.tileRows);
	shader->setInt("tiledLightDebugMaxLights", std::max(profile.pbrDeferredTiledLightDebugMaxLights, 1));
	shader->setFloat("tiledLightDebugIntensity", profile.pbrDeferredTiledLightDebugIntensity);
	const bool drawn = MeshDraw::drawIndexed(mDebugQuad);
	shader->end();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	stats.drawCalls = drawn ? 1 : 0;
	return stats;
}

void PBRDeferredTiledLightDebugPass::ensureDebugQuad(const std::shared_ptr<Shader>& shader)
{
	if (mDebugQuad || !shader)
	{
		return;
	}

	auto geometry = Geometry::createScreenPlane(shader);
	mDebugQuad = std::make_shared<Mesh>(geometry, nullptr);
	mDebugQuad->setName("PBR Deferred Tiled Light Debug Quad");
}

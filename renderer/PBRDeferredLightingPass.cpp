#include "PBRDeferredLightingPass.h"

#include <algorithm>

#include "camera/camera.h"
#include "core.h"
#include "framework/geometry.h"
#include "renderer/EnvironmentRenderTargets.h"
#include "renderer/MeshDraw.h"
#include "renderer/PBRDeferredLightCullingConfig.h"
#include "renderer/PBRGBufferRenderTargets.h"
#include "renderer/PBRShadowResourceBinder.h"
#include "renderer/RendererFramePassProfile.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

namespace
{
	void bindTexture2D(unsigned int texture, unsigned int unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void bindTextureOrDefault(
		const std::shared_ptr<Texture>& texture,
		unsigned int fallbackTarget,
		unsigned int unit
	)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(texture ? texture->getTextureTarget() : fallbackTarget, texture ? texture->getTexture() : 0);
	}
}

PBRDeferredLightingPassStats PBRDeferredLightingPass::render(
	const PBRGBufferRenderTargets& targets,
	const MaterialBindingContext& context,
	const RendererFramePassProfile& profile,
	ShaderLibrary& shaderLibrary
)
{
	if (!targets.isComplete() || context.camera == nullptr)
	{
		return {};
	}

	const auto shader = shaderLibrary.getPbrDeferredLightingShader();
	if (!shader)
	{
		return {};
	}

	ensureLightingQuad(shader);
	if (!mLightingQuad)
	{
		return {};
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
	bindGBufferTextures(shader, targets);
	PBRDeferredLightingPassStats stats = bindFrameUniforms(shader, context, profile, targets.getWidth(), targets.getHeight(), shaderLibrary);

	const bool drawn = MeshDraw::drawIndexed(mLightingQuad);
	stats.drawCalls = drawn ? 1 : 0;
	shader->end();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	return stats;
}

PBRDeferredLightingPassStats PBRDeferredLightingPass::bindFrameUniforms(
	const std::shared_ptr<Shader>& shader,
	const MaterialBindingContext& context,
	const RendererFramePassProfile& profile,
	unsigned int targetWidth,
	unsigned int targetHeight,
	ShaderLibrary& shaderLibrary
)
{
	PBRDeferredLightingPassStats stats{};
	shader->setVector3("cameraPosition", context.camera->mPosition);
	shader->setMat4("viewMatrix", context.camera->getViewMatrix());
	shader->setFloat("pbrDeferredLightingIntensity", profile.pbrDeferredLightingIntensity);
	const PBRShadowResourceBindResult shadowStats = PBRShadowResourceBinder::bindDetailed(shader, context);
	stats.csmShadowBound = shadowStats.bound;
	stats.csmLayerCount = shadowStats.csmLayerCount;
	stats.csmShadowAtlasBound = shadowStats.source == PBRShadowResourceSource::PBRShadowAtlas;
	stats.pointShadowAtlasBound = shadowStats.pointShadowAtlasBound;
	stats.pointShadowAtlasLightCount = shadowStats.pointShadowAtlasLightCount;

	const PBRDeferredLightBufferStats lightBufferStats = mLightBuffer.bind(context);
	stats.lightBufferBound = lightBufferStats.bound;
	stats.lightBufferPointLightCount = lightBufferStats.pointLightCount;
	stats.lightBufferMaxPointLightCount = lightBufferStats.maxPointLightCount;

	shader->setInt("useClusteredPointLights", 0);
	shader->setInt("clusteredLightTileSize", 0);
	shader->setInt("clusteredLightGridColumns", 0);
	shader->setInt("clusteredLightGridRows", 0);
	shader->setInt("clusteredLightDepthSlices", 0);
	shader->setFloat("clusteredLightNearPlane", std::max(context.camera->mNear, 0.001f));
	shader->setFloat("clusteredLightFarPlane", std::max(context.camera->mFar, context.camera->mNear + 0.001f));

	const bool useClusteredPointLights = profile.pbrDeferredClusteredLightsEnabled && lightBufferStats.pointLightCount > 0;
	if (useClusteredPointLights)
	{
		const PBRDeferredLightCullingConfig clusteredConfig = makePbrDeferredLightCullingConfig(
			profile,
			PBRDeferredLightCullingMode::GpuClustered
		);
		const PBRDeferredClusteredLightGridStats clusteredStats = mClusteredLightGrid.bindCompute(
			context,
			targetWidth,
			targetHeight,
			clusteredConfig,
			shaderLibrary.getPbrDeferredClusteredLightGridComputeShader()
		);
		shader->begin();
		stats.clusteredLightGridEnabled = clusteredStats.enabled;
		stats.clusteredLightGridBound = clusteredStats.bound;
		stats.clusteredLightGridComputeDispatched = clusteredStats.computeDispatched;
		stats.clusteredLightGridTileSize = clusteredStats.layout.tileSize;
		stats.clusteredLightGridColumns = clusteredStats.layout.clusterColumns;
		stats.clusteredLightGridRows = clusteredStats.layout.clusterRows;
		stats.clusteredLightGridDepthSlices = clusteredStats.layout.clusterDepthSlices;
		stats.clusteredLightGridClusterCount = clusteredStats.layout.clusterCount;
		stats.clusteredLightGridMaxLightsPerCluster = clusteredStats.layout.maxLightsPerCluster;
		stats.clusteredLightGridMaxIndexCount = clusteredStats.layout.maxLightIndexCount;
		stats.clusteredLightGridPointLightCount = clusteredStats.pointLightCount;
		stats.clusteredLightGridIndexCount = clusteredStats.lightIndexCount;
		stats.clusteredLightGridCulledIndexCount = clusteredStats.culledLightIndexCount;
		if (clusteredStats.bound)
		{
			shader->setInt("useClusteredPointLights", 1);
			shader->setInt("clusteredLightTileSize", clusteredStats.layout.tileSize);
			shader->setInt("clusteredLightGridColumns", clusteredStats.layout.clusterColumns);
			shader->setInt("clusteredLightGridRows", clusteredStats.layout.clusterRows);
			shader->setInt("clusteredLightDepthSlices", clusteredStats.layout.clusterDepthSlices);
		}
	}
	else if (profile.pbrDeferredClusteredLayoutStatsEnabled)
	{
		const PBRDeferredLightCullingConfig clusteredConfig = makePbrDeferredLightCullingConfig(
			profile,
			PBRDeferredLightCullingMode::GpuClustered
		);
		const PBRDeferredClusteredLightGridLayout clusteredLayout = makePbrDeferredClusteredLightGridLayout(
			targetWidth,
			targetHeight,
			clusteredConfig
		);
		stats.clusteredLightGridEnabled = usesPbrDeferredGpuClusteredLightGrid(clusteredConfig);
		stats.clusteredLightGridBound = false;
		stats.clusteredLightGridTileSize = clusteredLayout.tileSize;
		stats.clusteredLightGridColumns = clusteredLayout.clusterColumns;
		stats.clusteredLightGridRows = clusteredLayout.clusterRows;
		stats.clusteredLightGridDepthSlices = clusteredLayout.clusterDepthSlices;
		stats.clusteredLightGridClusterCount = clusteredLayout.clusterCount;
		stats.clusteredLightGridMaxLightsPerCluster = clusteredLayout.maxLightsPerCluster;
		stats.clusteredLightGridMaxIndexCount = clusteredLayout.maxLightIndexCount;
		stats.clusteredLightGridPointLightCount = lightBufferStats.pointLightCount;
	}

	const bool useTiledPointLights = !stats.clusteredLightGridBound && profile.pbrDeferredTiledLightsEnabled && lightBufferStats.pointLightCount > 0;
	const PBRDeferredLightCullingConfig lightCullingConfig = makePbrDeferredLightCullingConfig(
		profile,
		useTiledPointLights ? PBRDeferredLightCullingMode::CpuTiled : PBRDeferredLightCullingMode::Disabled
	);
	stats.tiledLightGridEnabled = usesPbrDeferredCpuTiledLightGrid(lightCullingConfig);
	shader->setInt("useTiledPointLights", stats.tiledLightGridEnabled ? 1 : 0);
	shader->setInt("tiledLightTileSize", lightCullingConfig.tileSize);
	shader->setInt("tiledLightGridColumns", 0);
	shader->setInt("tiledLightGridRows", 0);
	if (stats.tiledLightGridEnabled)
	{
		const PBRDeferredTiledLightGridStats tiledStats = mTiledLightGrid.bind(
			context,
			targetWidth,
			targetHeight,
			lightCullingConfig
		);
		stats.tiledLightGridBound = tiledStats.bound;
		stats.tiledLightGridEnabled = tiledStats.enabled;
		stats.tiledLightGridTileSize = tiledStats.tileSize;
		stats.tiledLightGridCutoff = tiledStats.lightCutoff;
		stats.tiledLightGridColumns = tiledStats.tileColumns;
		stats.tiledLightGridRows = tiledStats.tileRows;
		stats.tiledLightGridTileCount = tiledStats.tileCount;
		stats.tiledLightGridPointLightCount = tiledStats.pointLightCount;
		stats.tiledLightGridFullIndexCount = tiledStats.fullLightIndexCount;
		stats.tiledLightGridIndexCount = tiledStats.lightIndexCount;
		stats.tiledLightGridCulledIndexCount = tiledStats.culledLightIndexCount;
		stats.tiledLightGridOccupiedTileCount = tiledStats.occupiedTileCount;
		stats.tiledLightGridEmptyTileCount = tiledStats.emptyTileCount;
		stats.tiledLightGridMaxTileLightCount = tiledStats.maxTileLightCount;
		shader->setInt("useTiledPointLights", tiledStats.bound ? 1 : 0);
		shader->setInt("tiledLightTileSize", tiledStats.tileSize);
		shader->setInt("tiledLightGridColumns", tiledStats.tileColumns);
		shader->setInt("tiledLightGridRows", tiledStats.tileRows);
	}

	const bool useIBL = context.environmentTargets
		&& context.environmentTargets->isInitialized()
		&& context.environmentTargets->hasPrecomputedEnvironment();
	shader->setInt("useIBL", useIBL ? 1 : 0);
	shader->setFloat("iblDiffuseStrength", profile.pbrDeferredIblDiffuseStrength);
	shader->setFloat("iblSpecularStrength", profile.pbrDeferredIblSpecularStrength);
	if (!useIBL)
	{
		return stats;
	}

	const unsigned int maxMipLevels = context.environmentTargets->getMaxPrefilterMipLevels();
	shader->setFloat("iblMaxReflectionLod", maxMipLevels > 0 ? static_cast<float>(maxMipLevels - 1) : 0.0f);
	shader->setInt("irradianceMap", 4);
	shader->setInt("prefilterMap", 5);
	shader->setInt("brdfLut", 6);
	bindTextureOrDefault(context.environmentTargets->getIrradianceMap(), GL_TEXTURE_CUBE_MAP, 4);
	bindTextureOrDefault(context.environmentTargets->getPrefilterMap(), GL_TEXTURE_CUBE_MAP, 5);
	bindTextureOrDefault(context.environmentTargets->getBrdfLut(), GL_TEXTURE_2D, 6);
	return stats;
}

void PBRDeferredLightingPass::bindGBufferTextures(
	const std::shared_ptr<Shader>& shader,
	const PBRGBufferRenderTargets& targets
) const
{
	shader->setInt("positionRoughnessTexture", 0);
	shader->setInt("normalMetallicTexture", 1);
	shader->setInt("albedoAoTexture", 2);
	shader->setInt("depthTexture", 3);
	shader->setInt("emissiveTexture", 7);
	shader->setInt("materialParamsTexture", 10);

	bindTexture2D(targets.getPositionRoughnessTexture(), 0);
	bindTexture2D(targets.getNormalMetallicTexture(), 1);
	bindTexture2D(targets.getAlbedoAoTexture(), 2);
	bindTexture2D(targets.getDepthTexture(), 3);
	bindTexture2D(targets.getEmissiveTexture(), 7);
	bindTexture2D(targets.getMaterialParamsTexture(), 10);
}

void PBRDeferredLightingPass::ensureLightingQuad(const std::shared_ptr<Shader>& shader)
{
	if (mLightingQuad || !shader)
	{
		return;
	}

	auto geometry = Geometry::createScreenPlane(shader);
	mLightingQuad = std::make_shared<Mesh>(geometry, nullptr);
	mLightingQuad->setName("PBR Deferred Lighting Quad");
}

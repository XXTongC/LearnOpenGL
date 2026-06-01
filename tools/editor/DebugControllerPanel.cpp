#include "DebugControllerPanel.h"

#include <cstdint>
#include <string>

#include "DebugControllerContext.h"
#include "DebugProfileControlsPanel.h"
#include "EngineDiagnosticsContext.h"
#include "EngineDiagnosticsPanel.h"
#include "../../light/directionalLight.h"
#include "../../light/pointLight.h"
#include "../../light/shadow/shadow.h"
#include "../../mesh/mesh.h"
#include "../../renderer/RendererFrameStats.h"
#include "../../renderer/renderer.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
	void drawRendererFrameStats(const std::shared_ptr<GLframework::Renderer>& renderer)
	{
		if (!renderer)
		{
			return;
		}

		if (ImGui::CollapsingHeader("Renderer Frame Stats", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const GLframework::RendererFrameStats& stats = renderer->getLastFrameStats();
			const bool pbrPathActive = stats.pbrDepthPrepassDrawCalls > 0 || stats.pbrSceneDrawCalls > 0;

			ImGui::Text("PBR Path Active: %s", pbrPathActive ? "Yes" : "No");
			ImGui::Text("Renderer Passes Executed: %d", stats.rendererPassCount);
			if (stats.rendererGpuTimingEnabled)
			{
				const auto nsToMs = [](std::uint64_t value)
				{
					return static_cast<double>(value) / 1000000.0;
				};
				ImGui::Text("Renderer GPU Timing: %s, timed passes %d, frame %.3f ms",
					stats.rendererGpuTimingAvailable ? "Available" : "Unavailable",
					stats.rendererGpuTimedPassCount,
					nsToMs(stats.rendererGpuFrameTimeNs));
				ImGui::Text("Renderer GPU Timing Readback: %s, pending queries %d",
					stats.rendererGpuTimingDeferredReadback ? "Deferred" : "Immediate",
					stats.rendererGpuTimingPendingQueries);
				ImGui::Text("GPU PBR: depth %.3f ms, gbuffer %.3f ms, deferred %.3f ms",
					nsToMs(stats.rendererGpuPbrDepthPrepassTimeNs),
					nsToMs(stats.rendererGpuPbrGBufferTimeNs),
					nsToMs(stats.rendererGpuPbrDeferredLightingTimeNs));
				ImGui::Text("GPU Debug: tiled %.3f ms, clustered %.3f ms, gbuffer debug %.3f ms",
					nsToMs(stats.rendererGpuPbrDeferredTiledLightDebugTimeNs),
					nsToMs(stats.rendererGpuPbrDeferredClusteredLightDebugTimeNs),
					nsToMs(stats.rendererGpuPbrGBufferDebugTimeNs));
			}
			ImGui::Text("Shadow Casters: %d", stats.shadowCasterCount);
			ImGui::Text("Directional Shadow Layers: %d", stats.directionalShadowLayerCount);
			ImGui::Text("Directional Shadow Draw Calls: %d", stats.directionalShadowDrawCalls);
			ImGui::Text("Directional Alpha-Masked Shadow Draw Calls: %d", stats.directionalAlphaMaskedShadowDrawCalls);
			ImGui::Text("Point Shadow Lights: %d", stats.pointShadowLightCount);
			ImGui::Text("Point Shadow Faces: %d", stats.pointShadowFaceCount);
			ImGui::Text("Point Shadow Draw Calls: %d", stats.pointShadowDrawCalls);
			ImGui::Text("Point Alpha-Masked Shadow Draw Calls: %d", stats.pointAlphaMaskedShadowDrawCalls);
			ImGui::Text("PBR Shadow Atlas Ready: %s", stats.pbrShadowAtlasReady ? "Yes" : "No");
			ImGui::Text("PBR Shadow Atlas Directional Layers: %d @ %d", stats.pbrShadowAtlasDirectionalLayers, stats.pbrShadowAtlasDirectionalResolution);
			ImGui::Text("PBR Shadow Atlas Point Faces: %d / %d @ %d", stats.pbrShadowAtlasPointFacesRendered, stats.pbrShadowAtlasPointFaces, stats.pbrShadowAtlasPointResolution);
			ImGui::Text("PBR Shadow Atlas Draw Calls: directional %d, point %d", stats.pbrShadowAtlasDirectionalDrawCalls, stats.pbrShadowAtlasPointDrawCalls);
			ImGui::Text("PBR Shadow Atlas Alpha-Masked Draw Calls: directional %d, point %d", stats.pbrShadowAtlasDirectionalAlphaMaskedDrawCalls, stats.pbrShadowAtlasPointAlphaMaskedDrawCalls);
			ImGui::Text("Legacy Scene Draw Calls: %d", stats.legacySceneDrawCalls);
			ImGui::Text("Legacy Transparent Draw Calls: %d", stats.legacyTransparentDrawCalls);
			ImGui::Text("PBR Depth Prepass Draw Calls: %d", stats.pbrDepthPrepassDrawCalls);
			ImGui::Text("PBR GBuffer Ready: %s", stats.pbrGBufferReady ? "Yes" : "No");
			ImGui::Text("PBR GBuffer Size: %d x %d", stats.pbrGBufferWidth, stats.pbrGBufferHeight);
			ImGui::Text("PBR GBuffer Draw Calls: %d", stats.pbrGBufferDrawCalls);
			ImGui::Text("PBR Deferred Lighting Draw Calls: %d", stats.pbrDeferredLightingDrawCalls);
			ImGui::Text("PBR Deferred CSM Shadow Bound: %s", stats.pbrDeferredCsmShadowBound ? "Yes" : "No");
			ImGui::Text("PBR Deferred CSM Shadow Layers: %d", stats.pbrDeferredCsmShadowLayers);
			ImGui::Text("PBR Deferred CSM Shadow Atlas Bound: %s", stats.pbrDeferredCsmShadowAtlasBound ? "Yes" : "No");
			ImGui::Text("PBR Deferred Point Shadow Atlas Bound: %s", stats.pbrDeferredPointShadowAtlasBound ? "Yes" : "No");
			ImGui::Text("PBR Deferred Point Shadow Atlas Lights: %d", stats.pbrDeferredPointShadowAtlasLights);
			ImGui::Text("PBR Deferred Light Buffer Bound: %s", stats.pbrDeferredLightBufferBound ? "Yes" : "No");
			ImGui::Text("PBR Deferred Light Buffer Point Lights: %d / %d", stats.pbrDeferredLightBufferPointLights, stats.pbrDeferredLightBufferMaxPointLights);
			ImGui::Text("PBR Deferred Tiled Lights Enabled: %s", stats.pbrDeferredTiledLightsEnabled ? "Yes" : "No");
			ImGui::Text("PBR Deferred Tiled Light Grid Bound: %s", stats.pbrDeferredTiledLightGridBound ? "Yes" : "No");
			ImGui::Text("PBR Deferred Tiled Light Grid: %d x %d tiles @ %d px", stats.pbrDeferredTiledLightGridColumns, stats.pbrDeferredTiledLightGridRows, stats.pbrDeferredTiledLightGridTileSize);
			ImGui::Text("PBR Deferred Tiled Light Cutoff: %.4f", stats.pbrDeferredTiledLightGridCutoff);
			ImGui::Text("PBR Deferred Tiled Light Occupancy: %d / %d occupied, %d empty", stats.pbrDeferredTiledLightGridOccupiedTiles, stats.pbrDeferredTiledLightGridTileCount, stats.pbrDeferredTiledLightGridEmptyTiles);
			ImGui::Text("PBR Deferred Tiled Light Indices: %d, max per tile %d", stats.pbrDeferredTiledLightGridIndexCount, stats.pbrDeferredTiledLightGridMaxTileLights);
			const float tiledLightCullPercent = stats.pbrDeferredTiledLightGridFullIndexCount > 0
				? (static_cast<float>(stats.pbrDeferredTiledLightGridCulledIndexCount) * 100.0f) / static_cast<float>(stats.pbrDeferredTiledLightGridFullIndexCount)
				: 0.0f;
			ImGui::Text("PBR Deferred Tiled Light Culling: %d / %d indices skipped (%.1f%%)", stats.pbrDeferredTiledLightGridCulledIndexCount, stats.pbrDeferredTiledLightGridFullIndexCount, tiledLightCullPercent);
			ImGui::Text("PBR Deferred Clustered Layout Enabled: %s", stats.pbrDeferredClusteredLightGridEnabled ? "Yes" : "No");
			ImGui::Text("PBR Deferred Clustered Grid Bound: %s", stats.pbrDeferredClusteredLightGridBound ? "Yes" : "No");
			ImGui::Text("PBR Deferred Clustered Compute: %s", stats.pbrDeferredClusteredLightGridComputeDispatched ? "Yes" : "No");
			ImGui::Text("PBR Deferred Clustered Stats Readback: %s", stats.pbrDeferredClusteredLightGridStatsReadbackEnabled ? "Yes" : "No");
			ImGui::Text("PBR Deferred Clustered Grid: %d x %d x %d clusters @ %d px", stats.pbrDeferredClusteredLightGridColumns, stats.pbrDeferredClusteredLightGridRows, stats.pbrDeferredClusteredLightGridDepthSlices, stats.pbrDeferredClusteredLightGridTileSize);
			ImGui::Text("PBR Deferred Clustered Capacity: %d clusters, %d max lights/cluster, %d max indices", stats.pbrDeferredClusteredLightGridClusterCount, stats.pbrDeferredClusteredLightGridMaxLightsPerCluster, stats.pbrDeferredClusteredLightGridMaxIndexCount);
			if (stats.pbrDeferredClusteredLightGridLightIndexStatsAvailable)
			{
				ImGui::Text("PBR Deferred Clustered Light Indices: %d used, %d skipped", stats.pbrDeferredClusteredLightGridIndexCount, stats.pbrDeferredClusteredLightGridCulledIndexCount);
			}
			else
			{
				ImGui::TextUnformatted("PBR Deferred Clustered Light Indices: not read back");
			}
			ImGui::Text("PBR Deferred Tiled Light Debug Draw Calls: %d", stats.pbrDeferredTiledLightDebugDrawCalls);
			ImGui::Text("PBR Deferred Clustered Light Debug Draw Calls: %d", stats.pbrDeferredClusteredLightDebugDrawCalls);
			ImGui::Text("PBR GBuffer Debug Draw Calls: %d", stats.pbrGBufferDebugDrawCalls);
			ImGui::Text("PBR Scene Draw Calls: %d", stats.pbrSceneDrawCalls);
			ImGui::Text("PBR Transparent Draw Calls: %d", stats.pbrTransparentDrawCalls);
			ImGui::Text("IBL Debug Draw Calls: %d", stats.iblDebugDrawCalls);
			ImGui::TextWrapped("Use these values to verify whether the current scene is actually using the PBR render path.");
		}
	}
}

void GL_EDITOR::drawDebugControllerPanel(const DebugControllerContext& context)
{
	ImGui::Begin("controller");

	if (context.directionalLight && *context.directionalLight)
	{
		auto pos = (*context.directionalLight)->getPosition();
		if (ImGui::SliderFloat("light.x", &pos.x, 0.0f, 50.0f, "%.2f"))
		{
			(*context.directionalLight)->setPosition(pos);
		}

		if ((*context.directionalLight)->getShadow())
		{
			ImGui::SliderFloat("tightness", &(*context.directionalLight)->getShadow()->mDiskTightness, 0.0f, 1.0f, "%.3f");
			ImGui::SliderFloat("pcfRadius", &(*context.directionalLight)->getShadow()->mPcfRadius, 0.0f, 10.0f, "%.3f");
		}
	}

	if (context.textObject)
	{
		float rotate = context.textObject->getAngleX();
		if (ImGui::SliderFloat("Text Rotate:", &rotate, -360.0f, 360.0f))
		{
			context.textObject->setAngleX(rotate);
		}
	}

	if (context.orbitAngle && context.pointLights && !context.pointLights->empty())
	{
		if (ImGui::SliderAngle("angle", context.orbitAngle))
		{
			const float radius = 3.0f;
			const float x = radius * glm::sin(*context.orbitAngle);
			const float z = radius * glm::cos(*context.orbitAngle);
			(*context.pointLights)[0]->setPosition({ x, 3.0f, z });
		}
	}

	GL_EDITOR::drawDebugPipelineProfileControls(context);
	drawRendererFrameStats(context.renderer);
	GL_EDITOR::EngineDiagnosticsContext engineDiagnosticsContext{};
	engineDiagnosticsContext.engine = context.engine;
	engineDiagnosticsContext.engineWorld = context.engineWorld;
	engineDiagnosticsContext.assetSubsystem = context.assetSubsystem;
	engineDiagnosticsContext.rendererSubsystem = context.rendererSubsystem;
	GL_EDITOR::drawEngineDiagnosticsPanel(engineDiagnosticsContext);
	GL_EDITOR::drawDebugSceneProfileControls(context);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

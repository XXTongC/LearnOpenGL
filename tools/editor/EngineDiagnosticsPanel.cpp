#include "EngineDiagnosticsPanel.h"

#include "EngineDiagnosticsContext.h"
#include "../../engine/AssetRegistry.h"
#include "../../engine/AssetSubsystem.h"
#include "../../engine/Engine.h"
#include "../../engine/EngineLifecycleSnapshot.h"
#include "../../engine/Level.h"
#include "../../engine/RendererSubsystemFrameBridgeStats.h"
#include "../../engine/RendererSubsystem.h"
#include "../../engine/World.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
	const char* runModeLabel(GLengine::EngineRunMode runMode)
	{
		switch (runMode)
		{
		case GLengine::EngineRunMode::Editor:
			return "Editor";
		case GLengine::EngineRunMode::Game:
			return "Game";
		case GLengine::EngineRunMode::Verification:
			return "Verification";
		default:
			return "Unknown";
		}
	}

	int countWorldActors(const GLengine::World* world)
	{
		const auto* persistentLevel = world ? world->getPersistentLevel() : nullptr;
		return persistentLevel ? static_cast<int>(persistentLevel->getActors().size()) : 0;
	}

	const GLengine::EngineSubsystemLifecycleSummary* findSubsystemSummary(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		const char* name
	)
	{
		for (const auto& summary : snapshot.subsystemSummaries)
		{
			if (summary.name == name)
			{
				return &summary;
			}
		}

		return nullptr;
	}

	void drawSubsystemHealthStats(
		const GLengine::Engine* engine,
		const GLengine::World* engineWorld
	)
	{
		if (!engine)
		{
			return;
		}

		if (ImGui::CollapsingHeader("Subsystem Health", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto snapshot = engine->captureLifecycleSnapshot();
			const auto* assetSummary = findSubsystemSummary(snapshot, "AssetSubsystem");
			const auto* rendererSummary = findSubsystemSummary(snapshot, "RendererSubsystem");
			const auto* activeWorld = engine->getActiveWorld();
			const auto* displayWorld = engineWorld ? engineWorld : activeWorld;
			const bool contextWorldMatchesActive = displayWorld != nullptr && displayWorld == activeWorld;
			ImGui::Text("Engine Ticks: %d", snapshot.engineTickCount);
			ImGui::Text("World Ticks: %d", snapshot.activeWorldTickCount);
			ImGui::Text("Asset Subsystem Ticks: %d", assetSummary ? assetSummary->tickCount : 0);
			ImGui::Text("Renderer Subsystem Ticks: %d", rendererSummary ? rendererSummary->tickCount : 0);
			ImGui::Text("Context World Matches Active: %s", contextWorldMatchesActive ? "Yes" : "No");
			ImGui::Text("World Actors: %d", displayWorld == activeWorld ? snapshot.activeWorldActorCount : countWorldActors(displayWorld));
		}
	}

	void drawEngineRuntimeStats(const GLengine::Engine* engine)
	{
		if (!engine)
		{
			return;
		}

		if (ImGui::CollapsingHeader("Engine Runtime", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto snapshot = engine->captureLifecycleSnapshot();
			ImGui::Text("Initialized: %s", snapshot.initialized ? "Yes" : "No");
			ImGui::Text("Run Mode: %s", runModeLabel(snapshot.runMode));
			ImGui::Text("Viewport: %d x %d", snapshot.viewportWidth, snapshot.viewportHeight);
			ImGui::Text("Engine Time: %.6f s", snapshot.timeSeconds);
			ImGui::Text("Engine Delta: %.6f s", snapshot.deltaSeconds);
			ImGui::Text("Tick Count: %d", snapshot.engineTickCount);
			ImGui::Text("Subsystems: %d", snapshot.subsystemCount);
			ImGui::Text("Initialized Subsystems: %d", snapshot.initializedSubsystemCount);
			ImGui::Text("Ticked Subsystems: %d", snapshot.tickedSubsystemCount);
			if (ImGui::TreeNode("Subsystem Summary"))
			{
				for (const auto& summary : snapshot.subsystemSummaries)
				{
					ImGui::Text(
						"[%d] %s: initialized=%s, ticks=%d",
						summary.index,
						summary.name.c_str(),
						summary.initialized ? "Yes" : "No",
						summary.tickCount
					);
				}
				ImGui::TreePop();
			}
		}
	}

	void drawWorldStats(const GLengine::Engine* engine, const GLengine::World* engineWorld)
	{
		const auto* activeWorld = engine ? engine->getActiveWorld() : nullptr;
		const auto* displayWorld = engineWorld ? engineWorld : activeWorld;
		if (!displayWorld)
		{
			return;
		}

		if (ImGui::CollapsingHeader("Engine World", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto snapshot = engine ? engine->captureLifecycleSnapshot() : GLengine::EngineLifecycleSnapshot{};
			ImGui::TextWrapped("Name: %s", displayWorld->getName().c_str());
			ImGui::TextWrapped("Persistent Id: %s", displayWorld->getPersistentId().c_str());
			ImGui::Text("Playing: %s", displayWorld->isPlaying() ? "Yes" : "No");
			ImGui::Text("Context World Matches Active: %s", displayWorld == activeWorld ? "Yes" : "No");

			const auto* persistentLevel = displayWorld->getPersistentLevel();
			ImGui::Text("Persistent Level: %s", persistentLevel ? persistentLevel->getName().c_str() : "None");
			ImGui::Text("Actors: %d", displayWorld == activeWorld ? snapshot.activeWorldActorCount : countWorldActors(displayWorld));
			ImGui::Text("Tick Count: %d", displayWorld == activeWorld ? snapshot.activeWorldTickCount : displayWorld->getTickCount());
		}
	}

	void drawAssetSubsystemStats(const GLengine::AssetSubsystem* assetSubsystem)
	{
		if (!assetSubsystem)
		{
			return;
		}

		if (ImGui::CollapsingHeader("Asset Subsystem", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto& registry = assetSubsystem->getRegistry();
			ImGui::Text("Initialized: %s", assetSubsystem->isInitialized() ? "Yes" : "No");
			ImGui::Text("Tick Count: %d", assetSubsystem->getTickCount());
			ImGui::Text("Assets: %d", registry.count());
			ImGui::Text("Meshes: %d", registry.countByKind(GLengine::AssetKind::Mesh));
			ImGui::Text("Materials: %d", registry.countByKind(GLengine::AssetKind::Material));
			ImGui::Text("Textures: %d", registry.countByKind(GLengine::AssetKind::Texture));
			ImGui::Text("Lights: %d", registry.countByKind(GLengine::AssetKind::Light));
			ImGui::Text("Cameras: %d", registry.countByKind(GLengine::AssetKind::Camera));
		}
	}

	void drawRendererSubsystemStats(const GLengine::RendererSubsystem* rendererSubsystem)
	{
		if (!rendererSubsystem)
		{
			return;
		}

		if (ImGui::CollapsingHeader("Renderer Subsystem Frame Bridge", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const auto& stats = rendererSubsystem->getFrameBridgeStats();
			ImGui::Text("Initialized: %s", stats.initialized ? "Yes" : "No");
			ImGui::Text("Has Renderer: %s", stats.hasRenderer ? "Yes" : "No");
			ImGui::Text("Frame Bridge Active: %s", stats.frameBridgeActive ? "Yes" : "No");
			ImGui::Text("Renderer Backend Attached: %s", stats.rendererBackendAttached ? "Yes" : "No");
			ImGui::Text("Renderer Backend Ready: %s", stats.rendererBackendReady ? "Yes" : "No");
			ImGui::Text("Frame Config Valid: %s", stats.frameConfigValid ? "Yes" : "No");
			ImGui::Text("Framebuffer: %u x %u", stats.framebufferWidth, stats.framebufferHeight);
			ImGui::TextWrapped("Renderer Backend Key: %s", stats.rendererBackendKey.c_str());
			ImGui::TextWrapped("Renderer Backend State: %s", stats.rendererBackendState.c_str());
			ImGui::TextWrapped("Renderer Backend Owner: %s", stats.rendererBackendOwnerKey.c_str());
			ImGui::TextWrapped("Renderer Backend Ownership: %s", stats.rendererBackendOwnership.c_str());
			ImGui::TextWrapped("Renderer Backend Registry Key: %s", stats.rendererBackendRegistryKey.c_str());
			ImGui::Text("Renderer Backend Registry Count: %d", stats.rendererBackendRegistryCount);
			ImGui::Text(
				"Renderer Backend Lifecycle: attach %d, detach %d",
				stats.rendererBackendAttachCount,
				stats.rendererBackendDetachCount
			);
			ImGui::Text(
				"Renderer Backend Frames: ready %d, not ready %d",
				stats.rendererBackendReadyFrameCount,
				stats.rendererBackendNotReadyFrameCount
			);
			ImGui::TextWrapped("Frame Plan Key: %s", stats.framePlanKey.c_str());
			ImGui::Text(
				"Frame Passes: planned %d, executed %d, skipped %d",
				stats.plannedPassCount,
				stats.executedPassCount,
				stats.skippedPassCount
			);
			ImGui::Text("Render Frame Bridge Calls: %d", stats.renderFrameBridgeCallCount);
			ImGui::Text("Renderer Backend Frame Calls: %d", stats.rendererBackendFrameCallCount);
			ImGui::Text("Begin Frames: %d", stats.beginFrameCount);
			ImGui::Text("Completed Frames: %d", stats.completedFrameCount);
			ImGui::Text("Observed Renderer Passes: %d", stats.observedRendererPasses);
			ImGui::Text("Tick Count: %d", rendererSubsystem->getTickCount());
			ImGui::Text("Engine Time: %.6f s", stats.engineTimeSeconds);
			ImGui::Text("Engine Delta: %.6f s", stats.lastDeltaSeconds);
			ImGui::TextWrapped("This section verifies that the Engine-owned RendererSubsystem is observing the runtime renderer frame entry.");
		}
	}
}

void GL_EDITOR::drawEngineDiagnosticsPanel(const EngineDiagnosticsContext& context)
{
	drawSubsystemHealthStats(context.engine, context.engineWorld);
	drawEngineRuntimeStats(context.engine);
	drawWorldStats(context.engine, context.engineWorld);
	drawAssetSubsystemStats(context.assetSubsystem);
	drawRendererSubsystemStats(context.rendererSubsystem);
}

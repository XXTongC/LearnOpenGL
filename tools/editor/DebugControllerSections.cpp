#include "DebugControllerSections.h"

#include "DebugControllerContext.h"
#include "DebugControllerSectionRegistry.h"
#include "DebugLegacyControlsPanel.h"
#include "DebugProfileControlsPanel.h"
#include "DebugSectionRegistration.h"
#include "EngineDiagnosticsContext.h"
#include "EngineDiagnosticsPanel.h"
#include "RendererFrameStatsPanel.h"

namespace
{
	constexpr int kLegacyControlsOrder = 100;
	constexpr int kPipelineProfileControlsOrder = 200;
	constexpr int kRendererFrameStatsOrder = 300;
	constexpr int kEngineDiagnosticsOrder = 400;
	constexpr int kSceneProfileControlsOrder = 500;

	GL_EDITOR::DebugControllerSectionRegistry makeDefaultDebugControllerSectionRegistry()
	{
		GL_EDITOR::DebugControllerSectionRegistry registry{};
		GL_EDITOR::registerDefaultDebugControllerSections(registry);
		return registry;
	}
}

void GL_EDITOR::registerDefaultDebugControllerSections(DebugControllerSectionRegistry& registry)
{
	registerRequiredDebugSection(registry, {
		"legacy-controls",
		kLegacyControlsOrder,
		[](const DebugControllerContext& context)
		{
			drawDebugLegacyControls(context);
		}
	});

	registerRequiredDebugSection(registry, {
		"pipeline-profile-controls",
		kPipelineProfileControlsOrder,
		[](const DebugControllerContext& context)
		{
			drawDebugPipelineProfileControls(context);
		}
	});

	registerRequiredDebugSection(registry, {
		"renderer-frame-stats",
		kRendererFrameStatsOrder,
		[](const DebugControllerContext& context)
		{
			drawRendererFrameStatsPanel(context.renderer.get());
		}
	});

	registerRequiredDebugSection(registry, {
		"engine-diagnostics",
		kEngineDiagnosticsOrder,
		[](const DebugControllerContext& context)
		{
			EngineDiagnosticsContext engineDiagnosticsContext{};
			engineDiagnosticsContext.engine = context.engine;
			engineDiagnosticsContext.engineWorld = context.engineWorld;
			engineDiagnosticsContext.assetSubsystem = context.assetSubsystem;
			engineDiagnosticsContext.rendererSubsystem = context.rendererSubsystem;
			drawEngineDiagnosticsPanel(engineDiagnosticsContext);
		}
	});

	registerRequiredDebugSection(registry, {
		"scene-profile-controls",
		kSceneProfileControlsOrder,
		[](const DebugControllerContext& context)
		{
			drawDebugSceneProfileControls(context);
		}
	});
}

const GL_EDITOR::DebugControllerSectionRegistry& GL_EDITOR::defaultDebugControllerSectionRegistry()
{
	static const DebugControllerSectionRegistry registry = makeDefaultDebugControllerSectionRegistry();
	return registry;
}

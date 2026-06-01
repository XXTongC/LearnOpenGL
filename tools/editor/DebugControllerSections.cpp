#include "DebugControllerSections.h"

#include <utility>

#include "DebugControllerContext.h"
#include "DebugControllerSectionRegistry.h"
#include "DebugLegacyControlsPanel.h"
#include "DebugProfileControlsPanel.h"
#include "EngineDiagnosticsContext.h"
#include "EngineDiagnosticsPanel.h"
#include "RendererFrameStatsPanel.h"

namespace
{
	void registerDefaultSection(
		GL_EDITOR::DebugControllerSectionRegistry& registry,
		GL_EDITOR::DebugControllerSection section
	)
	{
		(void)registry.registerSection(std::move(section));
	}

	GL_EDITOR::DebugControllerSectionRegistry makeDefaultDebugControllerSectionRegistry()
	{
		GL_EDITOR::DebugControllerSectionRegistry registry{};

		registerDefaultSection(registry, {
			"legacy-controls",
			[](const GL_EDITOR::DebugControllerContext& context)
			{
				GL_EDITOR::drawDebugLegacyControls(context);
			}
		});

		registerDefaultSection(registry, {
			"pipeline-profile-controls",
			[](const GL_EDITOR::DebugControllerContext& context)
			{
				GL_EDITOR::drawDebugPipelineProfileControls(context);
			}
		});

		registerDefaultSection(registry, {
			"renderer-frame-stats",
			[](const GL_EDITOR::DebugControllerContext& context)
			{
				GL_EDITOR::drawRendererFrameStatsPanel(context.renderer.get());
			}
		});

		registerDefaultSection(registry, {
			"engine-diagnostics",
			[](const GL_EDITOR::DebugControllerContext& context)
			{
				GL_EDITOR::EngineDiagnosticsContext engineDiagnosticsContext{};
				engineDiagnosticsContext.engine = context.engine;
				engineDiagnosticsContext.engineWorld = context.engineWorld;
				engineDiagnosticsContext.assetSubsystem = context.assetSubsystem;
				engineDiagnosticsContext.rendererSubsystem = context.rendererSubsystem;
				GL_EDITOR::drawEngineDiagnosticsPanel(engineDiagnosticsContext);
			}
		});

		registerDefaultSection(registry, {
			"scene-profile-controls",
			[](const GL_EDITOR::DebugControllerContext& context)
			{
				GL_EDITOR::drawDebugSceneProfileControls(context);
			}
		});

		return registry;
	}
}

const GL_EDITOR::DebugControllerSectionRegistry& GL_EDITOR::defaultDebugControllerSectionRegistry()
{
	static const DebugControllerSectionRegistry registry = makeDefaultDebugControllerSectionRegistry();
	return registry;
}

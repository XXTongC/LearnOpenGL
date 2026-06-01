#include "DebugControllerSections.h"

#include <cassert>
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
	constexpr int kLegacyControlsOrder = 100;
	constexpr int kPipelineProfileControlsOrder = 200;
	constexpr int kRendererFrameStatsOrder = 300;
	constexpr int kEngineDiagnosticsOrder = 400;
	constexpr int kSceneProfileControlsOrder = 500;

	void registerDefaultSection(
		GL_EDITOR::DebugControllerSectionRegistry& registry,
		GL_EDITOR::DebugControllerSection section
	)
	{
		const bool registered = registry.registerSection(std::move(section));
		assert(registered && registry.lastRegistrationFailure().empty());
	}

	GL_EDITOR::DebugControllerSectionRegistry makeDefaultDebugControllerSectionRegistry()
	{
		GL_EDITOR::DebugControllerSectionRegistry registry{};

		registerDefaultSection(registry, {
			"legacy-controls",
			kLegacyControlsOrder,
			[](const GL_EDITOR::DebugControllerContext& context)
			{
				GL_EDITOR::drawDebugLegacyControls(context);
			}
		});

		registerDefaultSection(registry, {
			"pipeline-profile-controls",
			kPipelineProfileControlsOrder,
			[](const GL_EDITOR::DebugControllerContext& context)
			{
				GL_EDITOR::drawDebugPipelineProfileControls(context);
			}
		});

		registerDefaultSection(registry, {
			"renderer-frame-stats",
			kRendererFrameStatsOrder,
			[](const GL_EDITOR::DebugControllerContext& context)
			{
				GL_EDITOR::drawRendererFrameStatsPanel(context.renderer.get());
			}
		});

		registerDefaultSection(registry, {
			"engine-diagnostics",
			kEngineDiagnosticsOrder,
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
			kSceneProfileControlsOrder,
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

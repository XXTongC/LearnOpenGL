#include "DebugControllerPanel.h"

#include "DebugControllerContext.h"
#include "DebugLegacyControlsPanel.h"
#include "DebugProfileControlsPanel.h"
#include "EngineDiagnosticsContext.h"
#include "EngineDiagnosticsPanel.h"
#include "RendererFrameStatsPanel.h"
#include "../../third_party/imgui/imgui.h"

void GL_EDITOR::drawDebugControllerPanel(const DebugControllerContext& context)
{
	ImGui::Begin("controller");

	GL_EDITOR::drawDebugLegacyControls(context);
	GL_EDITOR::drawDebugPipelineProfileControls(context);
	GL_EDITOR::drawRendererFrameStatsPanel(context.renderer.get());
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

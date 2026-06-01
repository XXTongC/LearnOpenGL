#include "DebugProfileControlsPanel.h"

#include "EditorUiModuleRegistry.h"

void GL_EDITOR::drawDebugPipelineProfileControls(const DebugControllerContext& context)
{
	GL_EDITOR::defaultEditorUiModuleRegistries().pipelineProfileControls.drawAll(context);
}

void GL_EDITOR::drawDebugSceneProfileControls(const DebugControllerContext& context)
{
	GL_EDITOR::defaultEditorUiModuleRegistries().sceneProfileControls.drawAll(context);
}

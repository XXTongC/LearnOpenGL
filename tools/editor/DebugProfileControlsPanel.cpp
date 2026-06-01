#include "DebugProfileControlsPanel.h"

#include "DebugControllerContext.h"
#include "EditorUiModuleRegistry.h"

void GL_EDITOR::drawDebugPipelineProfileControls(const DebugControllerContext& context)
{
	const auto& editorUiModules = context.editorUiModules ? *context.editorUiModules : GL_EDITOR::defaultEditorUiModuleRegistries();
	editorUiModules.pipelineProfileControls.drawAll(context);
}

void GL_EDITOR::drawDebugSceneProfileControls(const DebugControllerContext& context)
{
	const auto& editorUiModules = context.editorUiModules ? *context.editorUiModules : GL_EDITOR::defaultEditorUiModuleRegistries();
	editorUiModules.sceneProfileControls.drawAll(context);
}

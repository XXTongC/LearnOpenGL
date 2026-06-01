#include "RuntimeEditorRenderResourceAdapter.h"

#include "RuntimeRenderResourceState.h"
#include "../framework/scene.h"
#include "../tools/editor/DebugControllerContext.h"
#include "../tools/editor/EditorPanels.h"
#include "../tools/editor/EditorSelectionState.h"

void GL_RUNTIME::RuntimeEditorRenderResourceAdapter::applyDebugControllerResources(
	const RuntimeRenderResourceState& renderResources,
	GL_EDITOR::DebugControllerContext& editorContext
)
{
	editorContext.textObject = renderResources.textD();
	editorContext.renderer = renderResources.renderer();
}

void GL_RUNTIME::RuntimeEditorRenderResourceAdapter::applyEditorPanelResources(
	const RuntimeRenderResourceState& renderResources,
	GL_EDITOR::EditorPanelContext& editorContext
)
{
	editorContext.sceneOffScreen = renderResources.sceneOffScreen();
	editorContext.sceneInScreen = renderResources.sceneInScreen();
}

void GL_RUNTIME::RuntimeEditorRenderResourceAdapter::ensureDefaultSelection(
	const RuntimeRenderResourceState& renderResources,
	GL_EDITOR::SelectionContext& selection
)
{
	GL_EDITOR::ensureSelectionIsInitialized(selection, renderResources.sceneOffScreen());
}

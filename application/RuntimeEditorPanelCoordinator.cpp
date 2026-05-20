#include "RuntimeEditorPanelCoordinator.h"

namespace GL_RUNTIME
{
	GL_EDITOR::DebugControllerContext RuntimeEditorPanelCoordinator::makeDebugControllerContext(
		GLframework::AppRuntimeContext& context,
		float* orbitAngle
	)
	{
		return {
			&context.dirLight,
			&context.ambientLight,
			&context.spotLight,
			&context.pointLights,
			context.textD,
			&context.framePipelineProfile,
			&context.framePipelineProfilePath,
			&context.rendererFramePassProfilePath,
			&context.postProcessSettings,
			&context.postProcessSettingsPath,
			context.renderer,
			&context.environmentProfile,
			&context.environmentProfilePath,
			&context.pbrPreviewProfile,
			&context.pbrPreviewProfilePath,
			&context.pbrExperimentProfilePath,
			&context.pbrLightRigProfile,
			&context.pbrCameraRigProfile,
			context.camera,
			orbitAngle
		};
	}

	GL_EDITOR::EditorPanelContext RuntimeEditorPanelCoordinator::makeEditorPanelContext(
		GLframework::AppRuntimeContext& context
	)
	{
		GL_EDITOR::EditorPanelContext editorContext{};
		editorContext.sceneOffScreen = context.sceneOffScreen;
		editorContext.sceneInScreen = context.sceneInScreen;
		editorContext.directionalLight = context.dirLight;
		editorContext.spotLight = context.spotLight;
		editorContext.pointLights = &context.pointLights;
		editorContext.mainCamera = context.camera;
		return editorContext;
	}

	void RuntimeEditorPanelCoordinator::drawPanels(
		GLframework::AppRuntimeContext& context,
		GL_EDITOR::SelectionContext& selection,
		float* orbitAngle
	)
	{
		GL_EDITOR::drawDebugControllerPanel(makeDebugControllerContext(context, orbitAngle));
		const auto editorContext = makeEditorPanelContext(context);
		GL_EDITOR::ensureSelectionIsInitialized(selection, context.sceneOffScreen);
		GL_EDITOR::drawHierarchyPanel(editorContext, selection);
		GL_EDITOR::drawSelectionInspectorPanel(editorContext, selection);
	}
}

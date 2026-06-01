#include "RuntimeEditorPanelCoordinator.h"

#include "AppRuntimeContext.h"
#include "../engine/AssetSubsystem.h"
#include "../framework/scene.h"
#include "../tools/editor/DebugControllerContext.h"
#include "../tools/editor/DebugControllerPanel.h"
#include "../tools/editor/EditorPanels.h"

namespace GL_RUNTIME
{
	GL_EDITOR::DebugControllerContext RuntimeEditorPanelCoordinator::makeDebugControllerContext(
		GLframework::AppRuntimeContext& context,
		float* orbitAngle
	)
	{
		return {
			&context.cameraLights.dirLight,
			&context.cameraLights.ambientLight,
			&context.cameraLights.spotLight,
			&context.cameraLights.pointLights,
			context.renderResources.textD(),
			&context.profiles.framePipelineProfile(),
			&context.profiles.framePipelineProfilePath,
			&context.profiles.rendererFramePassProfilePath,
			&context.profiles.postProcessSettings(),
			&context.profiles.postProcessSettingsPath,
			context.renderResources.renderer,
			context.engineAttachments.rendererSubsystem,
			&context.profiles.environmentProfile(),
			&context.profiles.environmentProfilePath,
			&context.profiles.pbrPreviewProfile(),
			&context.profiles.pbrPreviewProfilePath,
			&context.profiles.pbrExperimentProfilePath,
			&context.profiles.pbrLightRigProfile(),
			&context.profiles.pbrCameraRigProfile(),
			context.cameraLights.camera,
			orbitAngle,
			context.engineAttachments.engine,
			context.engineAttachments.engineWorld,
			context.engineAttachments.assetSubsystem
		};
	}

	GL_EDITOR::EditorPanelContext RuntimeEditorPanelCoordinator::makeEditorPanelContext(
		GLframework::AppRuntimeContext& context,
		GL_EDITOR::EditTransactionLog& editTransactions
	)
	{
		GL_EDITOR::EditorPanelContext editorContext{};
		editorContext.sceneOffScreen = context.renderResources.sceneOffScreen;
		editorContext.sceneInScreen = context.renderResources.sceneInScreen;
		editorContext.directionalLight = context.cameraLights.dirLight;
		editorContext.spotLight = context.cameraLights.spotLight;
		editorContext.pointLights = &context.cameraLights.pointLights;
		editorContext.mainCamera = context.cameraLights.camera;
		editorContext.engineWorld = context.engineAttachments.engineWorld;
		editorContext.assetRegistry = context.engineAttachments.assetSubsystem ? &context.engineAttachments.assetSubsystem->getRegistry() : nullptr;
		editorContext.engineWorldEditable = context.engineAttachments.engineWorldEditable;
		editorContext.editTransactions = &editTransactions;
		return editorContext;
	}

	void RuntimeEditorPanelCoordinator::drawPanels(
		GLframework::AppRuntimeContext& context,
		GL_EDITOR::SelectionContext& selection,
		GL_EDITOR::EditTransactionLog& editTransactions,
		float* orbitAngle
	)
	{
		GL_EDITOR::drawDebugControllerPanel(makeDebugControllerContext(context, orbitAngle));
		const auto editorContext = makeEditorPanelContext(context, editTransactions);
		GL_EDITOR::ensureSelectionIsInitialized(selection, context.renderResources.sceneOffScreen);
		GL_EDITOR::drawHierarchyPanel(editorContext, selection);
		GL_EDITOR::drawAssetBrowserPanel(editorContext, selection);
		GL_EDITOR::drawSelectionInspectorPanel(editorContext, selection);
	}
}

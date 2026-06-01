#include "RuntimeEditorPanelCoordinator.h"

#include "AppRuntimeContext.h"
#include "RuntimeEditorLifecycleState.h"
#include "RuntimeEditorRenderResourceAdapter.h"
#include "../engine/AssetSubsystem.h"
#include "../tools/editor/DebugControllerContext.h"
#include "../tools/editor/DebugControllerPanel.h"
#include "../tools/editor/EditorPanelContext.h"
#include "../tools/editor/EditorPanelFacades.h"

namespace GL_RUNTIME
{
	GL_EDITOR::DebugControllerContext RuntimeEditorPanelCoordinator::makeDebugControllerContext(
		GLframework::AppRuntimeContext& context,
		RuntimeEditorLifecycleState& editorState,
		float* orbitAngle
	)
	{
		GL_EDITOR::DebugControllerContext editorContext{};
		editorContext.directionalLight = &context.cameraLights.dirLight;
		editorContext.ambientLight = &context.cameraLights.ambientLight;
		editorContext.spotLight = &context.cameraLights.spotLight;
		editorContext.pointLights = &context.cameraLights.pointLights;
		editorContext.framePipelineProfile = &context.profiles.framePipelineProfile();
		editorContext.framePipelineProfilePath = &context.profiles.framePipelineProfilePath;
		editorContext.rendererFramePassProfilePath = &context.profiles.rendererFramePassProfilePath;
		editorContext.postProcessSettings = &context.profiles.postProcessSettings();
		editorContext.postProcessSettingsPath = &context.profiles.postProcessSettingsPath;
		editorContext.rendererSubsystem = context.engineAttachments.rendererSubsystem;
		editorContext.environmentProfile = &context.profiles.environmentProfile();
		editorContext.environmentProfilePath = &context.profiles.environmentProfilePath;
		editorContext.pbrPreviewProfile = &context.profiles.pbrPreviewProfile();
		editorContext.pbrPreviewProfilePath = &context.profiles.pbrPreviewProfilePath;
		editorContext.pbrExperimentProfilePath = &context.profiles.pbrExperimentProfilePath;
		editorContext.lightRigProfile = &context.profiles.pbrLightRigProfile();
		editorContext.cameraRigProfile = &context.profiles.pbrCameraRigProfile();
		editorContext.mainCamera = context.cameraLights.camera;
		editorContext.orbitAngle = orbitAngle;
		editorContext.engine = context.engineAttachments.engine;
		editorContext.engineWorld = context.engineAttachments.engineWorld;
		editorContext.assetSubsystem = context.engineAttachments.assetSubsystem;
		editorContext.editorUiModuleProfile = &context.profiles.editorUiModuleProfile();
		editorContext.editorUiModuleProfilePath = &context.profiles.editorUiModuleProfilePath;
		editorContext.requestEditorUiModuleProfileApply = [&editorState](const GL_EDITOR::EditorUiModuleProfile& profile)
		{
			return editorState.requestEditorUiModuleReconfiguration(profile);
		};
		editorContext.editorUiModules = &editorState.editorUiModules();
		RuntimeEditorRenderResourceAdapter::applyDebugControllerResources(context.renderResources, editorContext);
		return editorContext;
	}

	GL_EDITOR::EditorPanelContext RuntimeEditorPanelCoordinator::makeEditorPanelContext(
		GLframework::AppRuntimeContext& context,
		GL_EDITOR::EditTransactionLog& editTransactions,
		const GL_EDITOR::EditorUiModuleRegistries& editorUiModules
	)
	{
		GL_EDITOR::EditorPanelContext editorContext{};
		RuntimeEditorRenderResourceAdapter::applyEditorPanelResources(context.renderResources, editorContext);
		editorContext.directionalLight = context.cameraLights.dirLight;
		editorContext.spotLight = context.cameraLights.spotLight;
		editorContext.pointLights = &context.cameraLights.pointLights;
		editorContext.mainCamera = context.cameraLights.camera;
		editorContext.engineWorld = context.engineAttachments.engineWorld;
		editorContext.assetRegistry = context.engineAttachments.assetSubsystem ? &context.engineAttachments.assetSubsystem->getRegistry() : nullptr;
		editorContext.engineWorldEditable = context.engineAttachments.engineWorldEditable;
		editorContext.editTransactions = &editTransactions;
		editorContext.editorUiModules = &editorUiModules;
		return editorContext;
	}

	void RuntimeEditorPanelCoordinator::drawPanels(
		GLframework::AppRuntimeContext& context,
		RuntimeEditorLifecycleState& editorState,
		float* orbitAngle
	)
	{
		GL_EDITOR::drawDebugControllerPanel(makeDebugControllerContext(context, editorState, orbitAngle));
		const auto editorContext = makeEditorPanelContext(
			context,
			editorState.editTransactions(),
			editorState.editorUiModules()
		);
		auto& selection = editorState.selection();
		RuntimeEditorRenderResourceAdapter::ensureDefaultSelection(context.renderResources, selection);
		GL_EDITOR::drawHierarchyPanel(editorContext, selection);
		GL_EDITOR::drawAssetBrowserPanel(editorContext, selection);
		GL_EDITOR::drawSelectionInspectorPanel(editorContext, selection);
	}
}

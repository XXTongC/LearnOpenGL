#include "EditorUiModuleComposition.h"

#include "DebugControllerSections.h"
#include "DebugPipelineProfileControlSections.h"
#include "DebugSceneProfileControlSections.h"
#include "EditorUiModuleDiagnosticsSection.h"
#include "EditorUiModuleProfileControlsSection.h"
#include "SampleEditorUiModule.h"
#include "../inspector/SelectionInspectorProviders.h"

namespace
{
	void registerCoreEditorUiModule(GL_EDITOR::EditorUiModuleRegistries& registries)
	{
		GL_EDITOR::registerDefaultDebugControllerSections(registries.debugControllerSections);
		GL_EDITOR::registerEditorUiModuleDiagnosticsSection(registries.debugControllerSections);
		GL_EDITOR::registerEditorUiModuleProfileControlsSection(registries.debugControllerSections);
		GL_EDITOR::registerDefaultDebugPipelineProfileControlSections(registries.pipelineProfileControls);
		GL_EDITOR::registerDefaultDebugSceneProfileControlSections(registries.sceneProfileControls);
		GL_EDITOR::registerDefaultSelectionInspectorProviders(registries.selectionInspectors);
	}
}

const GL_EDITOR::EditorUiModuleCompositionPolicy& GL_EDITOR::defaultEditorUiModuleCompositionPolicy()
{
	static const EditorUiModuleCompositionPolicy policy{};
	return policy;
}

GL_EDITOR::EditorUiModuleList GL_EDITOR::buildEditorUiModuleList(const EditorUiModuleCompositionPolicy& policy)
{
	EditorUiModuleList modules{};
	if (policy.includeCoreEditorUi)
	{
		modules.push_back({ "core-editor-ui", registerCoreEditorUiModule });
	}

	if (policy.includeSampleEditorUi)
	{
		modules.push_back(sampleEditorUiModule());
	}

	return modules;
}

const GL_EDITOR::EditorUiModuleList& GL_EDITOR::defaultEditorUiModules()
{
	static const EditorUiModuleList modules = buildEditorUiModuleList(defaultEditorUiModuleCompositionPolicy());
	return modules;
}

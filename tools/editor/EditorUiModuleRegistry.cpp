#include "EditorUiModuleRegistry.h"

#include "DebugControllerSections.h"
#include "DebugPipelineProfileControlSections.h"
#include "DebugSceneProfileControlSections.h"
#include "../inspector/SelectionInspectorProviders.h"

void GL_EDITOR::registerDefaultEditorUiModules(EditorUiModuleRegistries& registries)
{
	registerDefaultDebugControllerSections(registries.debugControllerSections);
	registerDefaultDebugPipelineProfileControlSections(registries.pipelineProfileControls);
	registerDefaultDebugSceneProfileControlSections(registries.sceneProfileControls);
	registerDefaultSelectionInspectorProviders(registries.selectionInspectors);
}

GL_EDITOR::EditorUiModuleRegistries GL_EDITOR::buildDefaultEditorUiModuleRegistries()
{
	EditorUiModuleRegistries registries{};
	registerDefaultEditorUiModules(registries);
	return registries;
}

const GL_EDITOR::EditorUiModuleRegistries& GL_EDITOR::defaultEditorUiModuleRegistries()
{
	static const auto registries = buildDefaultEditorUiModuleRegistries();
	return registries;
}

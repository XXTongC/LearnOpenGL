#include "EditorUiModuleRegistry.h"

#include "DebugControllerSections.h"
#include "DebugPipelineProfileControlSections.h"
#include "DebugSceneProfileControlSections.h"
#include "SampleEditorUiModule.h"
#include "../inspector/SelectionInspectorProviders.h"

#include <cassert>

namespace
{
	void registerCoreEditorUiModule(GL_EDITOR::EditorUiModuleRegistries& registries)
	{
		GL_EDITOR::registerDefaultDebugControllerSections(registries.debugControllerSections);
		GL_EDITOR::registerDefaultDebugPipelineProfileControlSections(registries.pipelineProfileControls);
		GL_EDITOR::registerDefaultDebugSceneProfileControlSections(registries.sceneProfileControls);
		GL_EDITOR::registerDefaultSelectionInspectorProviders(registries.selectionInspectors);
	}
}

void GL_EDITOR::registerEditorUiModules(EditorUiModuleRegistries& registries, const EditorUiModuleList& modules)
{
	for (const auto& module : modules)
	{
		assert(!module.key.empty() && module.registerModule);
		if (module.key.empty() || !module.registerModule)
		{
			continue;
		}

		module.registerModule(registries);
	}
}

GL_EDITOR::EditorUiModuleRegistries GL_EDITOR::buildEditorUiModuleRegistries(const EditorUiModuleList& modules)
{
	EditorUiModuleRegistries registries{};
	registerEditorUiModules(registries, modules);
	return registries;
}

const GL_EDITOR::EditorUiModuleList& GL_EDITOR::defaultEditorUiModules()
{
	static const EditorUiModuleList modules{
		{ "core-editor-ui", registerCoreEditorUiModule },
		GL_EDITOR::sampleEditorUiModule(),
	};
	return modules;
}

void GL_EDITOR::registerDefaultEditorUiModules(EditorUiModuleRegistries& registries)
{
	registerEditorUiModules(registries, defaultEditorUiModules());
}

GL_EDITOR::EditorUiModuleRegistries GL_EDITOR::buildDefaultEditorUiModuleRegistries()
{
	return buildEditorUiModuleRegistries(defaultEditorUiModules());
}

const GL_EDITOR::EditorUiModuleRegistries& GL_EDITOR::defaultEditorUiModuleRegistries()
{
	static const auto registries = buildDefaultEditorUiModuleRegistries();
	return registries;
}

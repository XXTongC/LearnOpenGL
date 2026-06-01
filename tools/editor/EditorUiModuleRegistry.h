#pragma once

#include <functional>
#include <string>
#include <vector>

#include "DebugControllerSectionRegistry.h"
#include "DebugProfileControlSectionRegistry.h"
#include "../inspector/SelectionInspectorProviderRegistry.h"

namespace GL_EDITOR
{
	struct EditorUiModuleRegistries
	{
		DebugControllerSectionRegistry debugControllerSections{};
		DebugProfileControlSectionRegistry pipelineProfileControls{};
		DebugProfileControlSectionRegistry sceneProfileControls{};
		SelectionInspectorProviderRegistry selectionInspectors{};
	};

	using EditorUiModuleRegister = std::function<void(EditorUiModuleRegistries&)>;

	struct EditorUiModule
	{
		std::string key{};
		EditorUiModuleRegister registerModule{};
	};

	using EditorUiModuleList = std::vector<EditorUiModule>;

	void registerEditorUiModules(EditorUiModuleRegistries& registries, const EditorUiModuleList& modules);
	EditorUiModuleRegistries buildEditorUiModuleRegistries(const EditorUiModuleList& modules);
	const EditorUiModuleList& defaultEditorUiModules();
	void registerDefaultEditorUiModules(EditorUiModuleRegistries& registries);
	EditorUiModuleRegistries buildDefaultEditorUiModuleRegistries();
	const EditorUiModuleRegistries& defaultEditorUiModuleRegistries();
}

#pragma once

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

	void registerDefaultEditorUiModules(EditorUiModuleRegistries& registries);
	EditorUiModuleRegistries buildDefaultEditorUiModuleRegistries();
	const EditorUiModuleRegistries& defaultEditorUiModuleRegistries();
}

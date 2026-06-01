#pragma once

#include "EditorUiModuleRegistry.h"

namespace GL_EDITOR
{
	struct EditorUiModuleCompositionPolicy
	{
		bool includeCoreEditorUi = true;
		bool includeSampleEditorUi = true;
	};

	const EditorUiModuleCompositionPolicy& defaultEditorUiModuleCompositionPolicy();
	EditorUiModuleList buildEditorUiModuleList(const EditorUiModuleCompositionPolicy& policy);
}

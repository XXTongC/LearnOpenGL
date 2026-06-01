#pragma once

namespace GL_EDITOR
{
	class PropertyBuilder;
	struct EditorUiModuleProfile;

	void buildEditorUiModuleProfileConfigSchema(
		PropertyBuilder& builder,
		EditorUiModuleProfile& profile
	);
}

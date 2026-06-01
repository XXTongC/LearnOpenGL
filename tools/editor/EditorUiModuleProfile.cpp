#include "EditorUiModuleProfile.h"

#include "EditorUiModuleProfileConfig.h"
#include "../config/ProfileConfigIO.h"

std::string GL_EDITOR::EditorUiModuleProfileStorage::defaultPath()
{
	return "config/editor_ui_modules.local.ini";
}

bool GL_EDITOR::EditorUiModuleProfileStorage::loadFromFile(
	const std::string& path,
	EditorUiModuleProfile& profile
)
{
	EditorUiModuleProfile loadedProfile = profile;
	GL_EDITOR::PropertyBuilder builder{};
	buildEditorUiModuleProfileConfigSchema(builder, loadedProfile);
	const bool loaded = GL_CONFIG::loadPropertyConfig(path, builder);
	if (!loaded)
	{
		return false;
	}

	profile = loadedProfile;
	return true;
}

bool GL_EDITOR::EditorUiModuleProfileStorage::saveToFile(
	const std::string& path,
	const EditorUiModuleProfile& profile
)
{
	EditorUiModuleProfile snapshot = profile;
	GL_EDITOR::PropertyBuilder builder{};
	buildEditorUiModuleProfileConfigSchema(builder, snapshot);
	return GL_CONFIG::savePropertyConfig(
		path,
		"# Local editor UI module enablement profile",
		builder
	);
}

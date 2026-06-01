#pragma once

#include <string>

namespace GL_EDITOR
{
	struct EditorUiModuleProfile
	{
		bool enableCoreEditorUiModule{ true };
		bool enableSampleEditorUiModule{ true };
	};

	class EditorUiModuleProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, EditorUiModuleProfile& profile);
		static bool saveToFile(const std::string& path, const EditorUiModuleProfile& profile);
	};
}

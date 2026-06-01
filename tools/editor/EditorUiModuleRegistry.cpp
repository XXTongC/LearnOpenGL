#include "EditorUiModuleRegistry.h"

#include <cassert>

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

#include "EditorUiModuleProfileConfig.h"

#include "EditorUiModuleProfile.h"
#include "../inspector/PropertySchema.h"

void GL_EDITOR::buildEditorUiModuleProfileConfigSchema(
	PropertyBuilder& builder,
	EditorUiModuleProfile& profile
)
{
	builder.addSection("Editor UI Modules");
	builder.addConfigBool(
		"enableCoreEditorUiModule",
		"Core Editor UI Module",
		&profile.enableCoreEditorUiModule
	);
	builder.addConfigBool(
		"enableSampleEditorUiModule",
		"Sample Editor UI Module",
		&profile.enableSampleEditorUiModule
	);
}

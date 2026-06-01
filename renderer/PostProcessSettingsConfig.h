#pragma once

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GLframework
{
	struct PostProcessSettings;

	void buildPostProcessSettingsConfigSchema(
		GL_EDITOR::PropertyBuilder& builder,
		PostProcessSettings& settings
	);
}

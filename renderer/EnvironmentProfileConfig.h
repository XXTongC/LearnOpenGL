#pragma once

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GLframework
{
	struct EnvironmentProfile;

	void buildEnvironmentProfileConfigSchema(
		GL_EDITOR::PropertyBuilder& builder,
		EnvironmentProfile& profile
	);
}

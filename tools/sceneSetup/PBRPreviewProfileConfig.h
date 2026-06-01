#pragma once

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GL_SCENE
{
	struct PBRPreviewProfile;

	void buildPBRPreviewProfileConfigSchema(
		GL_EDITOR::PropertyBuilder& builder,
		PBRPreviewProfile& profile
	);
}

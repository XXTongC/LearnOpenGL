#pragma once

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GL_SCENE
{
	struct PBRCameraRigProfile;

	void buildPBRCameraRigProfileConfigSchema(
		GL_EDITOR::PropertyBuilder& builder,
		PBRCameraRigProfile& profile
	);
}

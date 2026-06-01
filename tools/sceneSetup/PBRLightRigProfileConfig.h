#pragma once

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GL_SCENE
{
	struct PBRLightRigProfile;

	void buildPBRLightRigProfileConfigSchema(
		GL_EDITOR::PropertyBuilder& builder,
		PBRLightRigProfile& profile
	);
}

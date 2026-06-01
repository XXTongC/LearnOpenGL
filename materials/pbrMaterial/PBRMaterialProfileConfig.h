#pragma once

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GLframework
{
	struct PBRMaterialProfile;

	void buildPBRMaterialProfileConfigSchema(
		GL_EDITOR::PropertyBuilder& builder,
		PBRMaterialProfile& profile
	);
}

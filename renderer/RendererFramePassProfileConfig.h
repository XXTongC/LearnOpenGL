#pragma once

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GLframework
{
	struct RendererFramePassProfile;

	void buildRendererFramePassProfileConfigSchema(
		GL_EDITOR::PropertyBuilder& builder,
		RendererFramePassProfile& profile
	);
}

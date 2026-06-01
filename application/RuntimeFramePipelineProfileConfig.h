#pragma once

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineProfile;

	void buildRuntimeFramePipelineProfileConfigSchema(
		GL_EDITOR::PropertyBuilder& builder,
		RuntimeFramePipelineProfile& profile
	);
}

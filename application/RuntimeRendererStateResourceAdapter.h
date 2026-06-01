#pragma once

namespace GLframework
{
	struct RendererFramePassProfile;
}

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState;

	class RuntimeRendererStateResourceAdapter
	{
	public:
		static bool syncClearColorToRenderer(const RuntimeRenderResourceState& renderResources);
		static GLframework::RendererFramePassProfile* rendererFramePassProfile(
			RuntimeRenderResourceState& renderResources
		);
	};
}

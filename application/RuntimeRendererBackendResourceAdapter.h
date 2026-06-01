#pragma once

namespace GLengine
{
	class RendererSubsystem;
}

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState;

	class RuntimeRendererBackendResourceAdapter
	{
	public:
		static bool hasRuntimeRenderer(const RuntimeRenderResourceState& renderResources);
		static void attachRuntimeRenderer(
			GLengine::RendererSubsystem& rendererSubsystem,
			const RuntimeRenderResourceState& renderResources
		);
		static bool isRuntimeRendererAttached(
			const GLengine::RendererSubsystem& rendererSubsystem,
			const RuntimeRenderResourceState& renderResources
		);
	};
}

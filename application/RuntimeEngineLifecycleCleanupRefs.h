#pragma once

namespace GLengine
{
	class AssetSubsystem;
	class RendererSubsystem;
}

namespace GL_RUNTIME
{
	struct RuntimeEngineLifecycleCleanupRefs
	{
		const GLengine::AssetSubsystem* assetSubsystem{ nullptr };
		const GLengine::RendererSubsystem* rendererSubsystem{ nullptr };
	};
}

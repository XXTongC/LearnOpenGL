#pragma once

namespace GLengine
{
	class AssetSubsystem;
	class RendererSubsystem;
}

namespace GL_RUNTIME
{
	struct RuntimeEngineLifecycleState
	{
		GLengine::AssetSubsystem* assetSubsystem{ nullptr };
		GLengine::RendererSubsystem* rendererSubsystem{ nullptr };
	};
}

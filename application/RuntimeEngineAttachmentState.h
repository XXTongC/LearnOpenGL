#pragma once

namespace GLengine
{
	class AssetSubsystem;
	class Engine;
	class RendererSubsystem;
	class World;
}

namespace GL_RUNTIME
{
	struct RuntimeEngineAttachmentState
	{
		GLengine::Engine* engine{ nullptr };
		GLengine::World* engineWorld{ nullptr };
		GLengine::AssetSubsystem* assetSubsystem{ nullptr };
		GLengine::RendererSubsystem* rendererSubsystem{ nullptr };
		bool engineWorldEditable{ false };
	};
}

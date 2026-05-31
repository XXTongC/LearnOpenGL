#pragma once

namespace GLengine
{
	class AssetSubsystem;
	class Engine;
	class RendererSubsystem;
	class World;
}

namespace GL_EDITOR
{
	struct EngineDiagnosticsContext
	{
		const GLengine::Engine* engine{ nullptr };
		const GLengine::World* engineWorld{ nullptr };
		const GLengine::AssetSubsystem* assetSubsystem{ nullptr };
		const GLengine::RendererSubsystem* rendererSubsystem{ nullptr };
	};
}

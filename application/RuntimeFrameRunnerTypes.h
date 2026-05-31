#pragma once

namespace GLengine
{
	class Engine;
	class RendererSubsystem;
}

namespace GL_RUNTIME
{
	struct RuntimeFrameConfig
	{
		GLengine::Engine* engine{ nullptr };
		GLengine::RendererSubsystem* rendererSubsystem{ nullptr };
		unsigned int framebufferWidth{ 0 };
		unsigned int framebufferHeight{ 0 };
		float deltaSeconds{ 1.0f / 60.0f };
	};
}

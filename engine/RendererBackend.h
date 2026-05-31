#pragma once

#include <string>

namespace GLengine
{
	struct EngineContext;

	struct RendererFrameIntent
	{
		unsigned int framebufferWidth{ 0 };
		unsigned int framebufferHeight{ 0 };
	};

	struct RendererFrameResult
	{
		std::string framePlanKey{ "none" };
		int plannedPassCount{ 0 };
		int executedPassCount{ 0 };
		int skippedPassCount{ 0 };
	};

	class RendererBackend
	{
	public:
		virtual ~RendererBackend() = default;

		virtual const char* getBackendKey() const = 0;
		virtual bool isBackendReady() const = 0;
		virtual RendererFrameResult renderFrame(
			const EngineContext& context,
			const RendererFrameIntent& frameIntent
		) = 0;
	};

}

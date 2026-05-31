#pragma once

#include "engine/RendererBackendFrameTypes.h"

namespace GLengine
{
	struct EngineContext;

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

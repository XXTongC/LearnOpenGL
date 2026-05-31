#pragma once

#include "../engine/RendererBackend.h"

namespace GL_RUNTIME
{
	class RuntimeNoOpRendererBackend : public GLengine::RendererBackend
	{
	public:
		~RuntimeNoOpRendererBackend() override = default;

		const char* getBackendKey() const override;
		bool isBackendReady() const override;
		GLengine::RendererFrameResult renderFrame(
			const GLengine::EngineContext& engineContext,
			const GLengine::RendererFrameIntent& frameIntent
		) override;
	};
}

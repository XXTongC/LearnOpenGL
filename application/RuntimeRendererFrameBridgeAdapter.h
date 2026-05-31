#pragma once

#include "../engine/RendererBackend.h"

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	class RuntimeRendererFrameBridgeAdapter : public GLengine::RendererBackend
	{
	public:
		explicit RuntimeRendererFrameBridgeAdapter(GLframework::AppRuntimeContext& context);
		~RuntimeRendererFrameBridgeAdapter() override = default;

		const char* getBackendKey() const override;
		bool isBackendReady() const override;
		GLengine::RendererFrameResult renderFrame(
			const GLengine::EngineContext& engineContext,
			const GLengine::RendererFrameIntent& frameIntent
		) override;

		static GLengine::RendererFrameResult renderRuntimeFrame(
			GLframework::AppRuntimeContext& context,
			const GLengine::RendererFrameIntent& frameIntent
		);

	private:
		GLframework::AppRuntimeContext& mContext;
	};
}

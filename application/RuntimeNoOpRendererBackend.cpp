#include "RuntimeNoOpRendererBackend.h"

#include "core.h"
#include "../engine/RendererBackendFrameTypes.h"

namespace GL_RUNTIME
{
	const char* RuntimeNoOpRendererBackend::getBackendKey() const
	{
		return "test-noop-renderer-backend";
	}

	bool RuntimeNoOpRendererBackend::isBackendReady() const
	{
		return true;
	}

	GLengine::RendererFrameResult RuntimeNoOpRendererBackend::renderFrame(
		const GLengine::EngineContext& engineContext,
		const GLengine::RendererFrameIntent& frameIntent
	)
	{
		(void)engineContext;

		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL_CALL(glViewport(0, 0, static_cast<GLsizei>(frameIntent.framebufferWidth), static_cast<GLsizei>(frameIntent.framebufferHeight)));
		GL_CALL(glDisable(GL_SCISSOR_TEST));
		GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
		GL_CALL(glDepthMask(GL_TRUE));
		GL_CALL(glClearColor(0.07f, 0.18f, 0.31f, 1.0f));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		return {
			"test-noop-renderer-backend:clear",
			1,
			1,
			0
		};
	}
}

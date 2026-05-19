#include "FrameRenderState.h"

#include "core.h"

using namespace GLframework;

void FrameRenderState::begin(unsigned int fbo) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilMask(0xff);

	glDisable(GL_BLEND);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

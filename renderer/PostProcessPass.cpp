#include "PostProcessPass.h"

#include "core.h"

using namespace GLframework;

void PostProcessPass::resolveMultisample(
	const std::shared_ptr<Framebuffer>& src,
	const std::shared_ptr<Framebuffer>& dst
) const
{
	if (src == nullptr || dst == nullptr)
	{
		return;
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, src->getFBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->getFBO());
	glBlitFramebuffer(
		0,
		0,
		src->getWidth(),
		src->getHeight(),
		0,
		0,
		dst->getWidth(),
		dst->getHeight(),
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST
	);
}

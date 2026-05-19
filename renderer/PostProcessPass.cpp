#include "PostProcessPass.h"

#include "core.h"
#include "materials/screenMaterial.h"
#include "renderer/Bloom/Bloom.h"

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

void PostProcessPass::renderScreenComposite(
	const std::shared_ptr<Mesh>& screenQuad,
	const std::shared_ptr<Shader>& shader,
	unsigned int width,
	unsigned int height,
	unsigned int targetFbo
) const
{
	if (screenQuad == nullptr || shader == nullptr)
	{
		return;
	}

	auto material = screenQuad->getMaterial();
	if (material == nullptr || material->getMaterialType() != MaterialType::ScreenMaterial)
	{
		return;
	}

	auto screenMaterial = std::static_pointer_cast<ScreenMaterial>(material);
	if (screenMaterial->mScreenTexture == nullptr)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
	glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glClear(GL_COLOR_BUFFER_BIT);

	shader->begin();
	shader->setInt("screenTextureSampler", 0);
	shader->setInt("depthTextureSampler", 1);
	shader->setFloat("texWidth", static_cast<float>(width));
	shader->setFloat("texHeight", static_cast<float>(height));
	shader->setFloat("exposure", screenMaterial->mExposure);

	screenMaterial->mScreenTexture->setUnit(0);
	screenMaterial->mScreenTexture->Bind();
	if (screenMaterial->mDepthStencilTexture != nullptr)
	{
		screenMaterial->mDepthStencilTexture->setUnit(1);
		screenMaterial->mDepthStencilTexture->Bind();
	}

	auto geometry = screenQuad->getGeometry();
	if (geometry != nullptr)
	{
		glBindVertexArray(geometry->getVao());
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}
	shader->end();

	glDepthMask(GL_TRUE);
}

void PostProcessPass::extractBloomBright(
	const std::shared_ptr<Bloom>& bloom,
	const std::shared_ptr<Framebuffer>& src,
	const std::shared_ptr<Framebuffer>& dst
) const
{
	if (bloom == nullptr)
	{
		return;
	}

	bloom->extractBright(src, dst);
}

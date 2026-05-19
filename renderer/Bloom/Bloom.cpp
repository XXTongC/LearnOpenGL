#include "Bloom.h"

using namespace GLframework;

Bloom::Bloom(int width,int height, int min_Resolution)
	:mWidth(width),mHeight(height)
{
	float width_levels = std::log2(static_cast<float>(width) / static_cast<float>(min_Resolution));
	float height_levels = std::log2(static_cast<float>(height) / static_cast<float>(min_Resolution));

	mMipLevels = static_cast<int>(std::min(width_levels, height_levels));
	int w = mWidth, h = mHeight;
	for(int i = 0;i<mMipLevels;++i)
	{
		mDownSamples.emplace_back(Framebuffer::createHDRBloomFbo(w, h));
		w /= 2;
		h /= 2;
	}

	w = 4 * w, h = 4 * h;
	for(int i  = 0;i<mMipLevels - 1;++i)
	{
		mUpSamples.emplace_back(Framebuffer::createHDRBloomFbo(w, h));
		w *= 2;
		h *= 2;
	}

	mExtractBrightShader = std::make_shared<Shader>("shaders/bloom/extractBright.vert", "shaders/bloom/extractBright.frag");
	mQuad = Geometry::createScreenPlane(mExtractBrightShader);
}

Bloom::~Bloom()
{
	
}

void Bloom::extractBright(
	const std::shared_ptr<Framebuffer>& src,
	const std::shared_ptr<Framebuffer>& dst
) const
{
	if (src == nullptr || dst == nullptr || mExtractBrightShader == nullptr || mQuad == nullptr)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, dst->getFBO());
	glViewport(0, 0, dst->getWidth(), dst->getHeight());
	glClear(GL_COLOR_BUFFER_BIT);

	mExtractBrightShader->begin();
	auto srcTex = src->getColorAttachment();
	if (srcTex != nullptr)
	{
		srcTex->setUnit(0);
		srcTex->Bind();
	}
	mExtractBrightShader->setInt("srcTex", 0);
	mExtractBrightShader->setFloat("threshold", mThreshold);

	glBindVertexArray(mQuad->getVao());
	glDrawElements(GL_TRIANGLES, mQuad->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	mExtractBrightShader->end();
}


#include "Bloom.h"

using namespace GLframework;

Bloom::Bloom(int width,int height, int min_Resolution)
	:mWidth(width),mHeight(height)
{
	float width_levels = std::log2(static_cast<float>(width) / static_cast<float>(min_Resolution));
	float height_levels = std::log2(static_cast<float>(height) / static_cast<float>(min_Resolution));

	mMipLevels = std::min(width_levels, height_levels);
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


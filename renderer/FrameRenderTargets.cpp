#include "FrameRenderTargets.h"

using namespace GLframework;

void FrameRenderTargets::initialize(unsigned int width, unsigned int height, unsigned int samples)
{
	if (width == 0 || height == 0)
	{
		return;
	}

	mWidth = width;
	mHeight = height;
	mSamples = samples;
	mMultisample = Framebuffer::createMultiSampleFbo(width, height, samples);
	mResolved = Framebuffer::createHDRFbo(width, height);
	mBloomBright = Framebuffer::createHDRBloomFbo(width, height);
	mBloomPing = Framebuffer::createHDRBloomFbo(width, height);
	mBloomPong = Framebuffer::createHDRBloomFbo(width, height);
}

bool FrameRenderTargets::resize(unsigned int width, unsigned int height)
{
	if (width == 0 || height == 0)
	{
		return false;
	}

	if (isInitialized() && width == mWidth && height == mHeight)
	{
		return false;
	}

	const unsigned int samples = mSamples != 0 ? mSamples : 4;
	initialize(width, height, samples);
	return true;
}

bool FrameRenderTargets::isInitialized() const
{
	return mMultisample != nullptr
		&& mResolved != nullptr
		&& mBloomBright != nullptr
		&& mBloomPing != nullptr
		&& mBloomPong != nullptr;
}

unsigned int FrameRenderTargets::getSceneFbo() const
{
	return mMultisample != nullptr ? mMultisample->getFBO() : 0;
}

unsigned int FrameRenderTargets::getWidth() const
{
	return mWidth;
}

unsigned int FrameRenderTargets::getHeight() const
{
	return mHeight;
}

unsigned int FrameRenderTargets::getSamples() const
{
	return mSamples;
}

const std::shared_ptr<Framebuffer>& FrameRenderTargets::getMultisample() const
{
	return mMultisample;
}

const std::shared_ptr<Framebuffer>& FrameRenderTargets::getResolved() const
{
	return mResolved;
}

const std::shared_ptr<Framebuffer>& FrameRenderTargets::getBloomBright() const
{
	return mBloomBright;
}

const std::shared_ptr<Framebuffer>& FrameRenderTargets::getBloomPing() const
{
	return mBloomPing;
}

const std::shared_ptr<Framebuffer>& FrameRenderTargets::getBloomPong() const
{
	return mBloomPong;
}

std::shared_ptr<Texture> FrameRenderTargets::getResolvedColorAttachment() const
{
	return mResolved != nullptr ? mResolved->getColorAttachment() : nullptr;
}

std::shared_ptr<Texture> FrameRenderTargets::getResolvedDepthStencilAttachment() const
{
	return mResolved != nullptr ? mResolved->getDepthStencilAttachment() : nullptr;
}

std::shared_ptr<Texture> FrameRenderTargets::getBloomBrightColorAttachment() const
{
	return mBloomBright != nullptr ? mBloomBright->getColorAttachment() : nullptr;
}

std::shared_ptr<Texture> FrameRenderTargets::getBloomPongColorAttachment() const
{
	return mBloomPong != nullptr ? mBloomPong->getColorAttachment() : nullptr;
}

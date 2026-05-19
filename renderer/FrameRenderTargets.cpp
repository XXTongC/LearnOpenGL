#include "FrameRenderTargets.h"

using namespace GLframework;

void FrameRenderTargets::initialize(unsigned int width, unsigned int height, unsigned int samples)
{
	mWidth = width;
	mHeight = height;
	mSamples = samples;
	mMultisample = Framebuffer::createMultiSampleFbo(width, height, samples);
	mResolved = Framebuffer::createHDRFbo(width, height);
}

bool FrameRenderTargets::isInitialized() const
{
	return mMultisample != nullptr && mResolved != nullptr;
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

std::shared_ptr<Texture> FrameRenderTargets::getResolvedColorAttachment() const
{
	return mResolved != nullptr ? mResolved->getColorAttachment() : nullptr;
}

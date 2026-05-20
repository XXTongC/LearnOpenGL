#include "PBRGBufferRenderTargets.h"

#include <iostream>

#include "core.h"

using namespace GLframework;

PBRGBufferRenderTargets::~PBRGBufferRenderTargets()
{
	release();
}

bool PBRGBufferRenderTargets::ensureSize(unsigned int width, unsigned int height)
{
	if (width == 0 || height == 0)
	{
		return false;
	}

	if (mFbo != 0 && mWidth == width && mHeight == height && mComplete)
	{
		return true;
	}

	release();

	mWidth = width;
	mHeight = height;
	mPositionRoughnessTexture = createColorAttachment(width, height);
	mNormalMetallicTexture = createColorAttachment(width, height);
	mAlbedoAoTexture = createColorAttachment(width, height);
	mEmissiveTexture = createColorAttachment(width, height);
	mDepthTexture = createDepthAttachment(width, height);

	glGenFramebuffers(1, &mFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mPositionRoughnessTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mNormalMetallicTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mAlbedoAoTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, mEmissiveTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);

	const unsigned int attachments[4]{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3
	};
	glDrawBuffers(4, attachments);

	mComplete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	if (!mComplete)
	{
		std::cerr << "Error: PBR G-buffer framebuffer is not complete" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return mComplete;
}

void PBRGBufferRenderTargets::release()
{
	if (mPositionRoughnessTexture != 0)
	{
		glDeleteTextures(1, &mPositionRoughnessTexture);
		mPositionRoughnessTexture = 0;
	}
	if (mNormalMetallicTexture != 0)
	{
		glDeleteTextures(1, &mNormalMetallicTexture);
		mNormalMetallicTexture = 0;
	}
	if (mAlbedoAoTexture != 0)
	{
		glDeleteTextures(1, &mAlbedoAoTexture);
		mAlbedoAoTexture = 0;
	}
	if (mEmissiveTexture != 0)
	{
		glDeleteTextures(1, &mEmissiveTexture);
		mEmissiveTexture = 0;
	}
	if (mDepthTexture != 0)
	{
		glDeleteTextures(1, &mDepthTexture);
		mDepthTexture = 0;
	}
	if (mFbo != 0)
	{
		glDeleteFramebuffers(1, &mFbo);
		mFbo = 0;
	}

	mWidth = 0;
	mHeight = 0;
	mComplete = false;
}

bool PBRGBufferRenderTargets::isComplete() const
{
	return mComplete;
}

unsigned int PBRGBufferRenderTargets::getFbo() const
{
	return mFbo;
}

unsigned int PBRGBufferRenderTargets::getWidth() const
{
	return mWidth;
}

unsigned int PBRGBufferRenderTargets::getHeight() const
{
	return mHeight;
}

unsigned int PBRGBufferRenderTargets::getPositionRoughnessTexture() const
{
	return mPositionRoughnessTexture;
}

unsigned int PBRGBufferRenderTargets::getNormalMetallicTexture() const
{
	return mNormalMetallicTexture;
}

unsigned int PBRGBufferRenderTargets::getAlbedoAoTexture() const
{
	return mAlbedoAoTexture;
}

unsigned int PBRGBufferRenderTargets::getEmissiveTexture() const
{
	return mEmissiveTexture;
}

unsigned int PBRGBufferRenderTargets::getDepthTexture() const
{
	return mDepthTexture;
}

unsigned int PBRGBufferRenderTargets::createColorAttachment(unsigned int width, unsigned int height) const
{
	unsigned int texture{ 0 };
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

unsigned int PBRGBufferRenderTargets::createDepthAttachment(unsigned int width, unsigned int height) const
{
	unsigned int texture{ 0 };
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

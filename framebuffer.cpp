#include "framebuffer.h"
using namespace GLframework;

Framebuffer::Framebuffer(unsigned width, unsigned height)
{
	mWidth = width;
	mHeight = height;

	//	1.生成fbo对象并绑定
	glGenFramebuffers(1, &mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

	//	2.生成颜色控件，并且加入fbo
	mColorAttachment = Texture::createColorAttachment(mWidth, mHeight, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorAttachment->getTexture(), 0);
	//	3.生成depth Stencil附件，加入fbo
	mDepthStencilAttachment = Texture::createDepthStencilAttachment(mWidth, mHeight, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, mDepthStencilAttachment->getTexture(), 0);

	//	检查当前构建的fbo是否完整
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: FrameBuff is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDeleteFramebuffers(1, &fbo);
}


std::shared_ptr<Texture> Framebuffer::getColorAttachment() const
{
	return mColorAttachment;
}

std::shared_ptr<Texture> Framebuffer::getDepthStencilAttachment() const
{
	return mDepthStencilAttachment;
}

unsigned Framebuffer::getFBO() const
{
	return mFBO;
}

unsigned Framebuffer::getHeight() const
{
	return mHeight;
}

unsigned Framebuffer::getWidth() const
{
	return mWidth;
}

void Framebuffer::setColorAttachment(std::shared_ptr<Texture> value)
{
	mColorAttachment = value;
}

void Framebuffer::setDepthStencilAttachment(std::shared_ptr<Texture> value)
{
	mDepthStencilAttachment = value;
}

void Framebuffer::setFBO(unsigned value)
{
	mFBO = value;
}

void Framebuffer::setHeight(unsigned value)
{
	mHeight = value;
}

void Framebuffer::setWidth(unsigned value)
{
	mWidth = value;
}
#include "framebuffer.h"
using namespace GLframework;

std::shared_ptr<Framebuffer> Framebuffer::createPointLightShadowFBO(unsigned width, unsigned height, unsigned layerCount)
{
	std::shared_ptr<Framebuffer> fb = std::make_shared<Framebuffer>();
	fb->setWidth(width);
	fb->setHeight(height);

	glGenFramebuffers(1, &fb->mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, fb->mFBO);
	std::cout << fb->mFBO;
	
	// 创建深度纹理数组
	fb->mDepthAttachment = Texture::createTexture2DArray(width, height, layerCount, 0, GL_DEPTH_COMPONENT32F);

	// 绑定深度纹理数组到帧缓冲
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fb->mDepthAttachment->getTexture(), 0, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// 检查帧缓冲是否完整
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: PointLight Shadow Framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fb;
}

std::shared_ptr<Texture> Framebuffer::getDepthAttachment() const
{
	return mDepthAttachment;
}

void Framebuffer::setDepthAttachment(std::shared_ptr<Texture> value)
{
	mDepthAttachment = std::move(value);
}

std::shared_ptr<Framebuffer> Framebuffer::createShadowFBO(unsigned width, unsigned height)
{
	std::shared_ptr<Framebuffer> fb = std::make_shared<Framebuffer>();
	fb->setWidth(width);
	fb->setHeight(height);

	glGenFramebuffers(1, &fb->mFBO);
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fb->mFBO));

	fb->mDepthAttachment = Texture::createDepthAttachment(width, height, 0);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb->mDepthAttachment->getTexture(), 0);
	std::cout << fb->mDepthAttachment << std::endl;
	glDrawBuffer(GL_NONE);	//tell opengl there is no color output in current fbo
	
	if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
	{
		std::cerr <<"Error: FrameBuff is not complete! ERROR:GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fb;
}

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
		std::cerr << "Error: FrameBuff is not complete! " << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}

std::shared_ptr<Framebuffer> Framebuffer::createCSMShadowFBO(unsigned width, unsigned height, unsigned int layerNum)
{
	std::shared_ptr<Framebuffer> fb = std::make_shared<Framebuffer>();
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//	add depthAttachment
	auto depthAttachment = Texture::createDepthAttachmentCSMArray(width, height, layerNum, 0);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthAttachment->getTexture(), 0, 0);
	glDrawBuffer(GL_NONE);
	glBindBuffer(GL_FRAMEBUFFER, 0);

	fb->setFBO(fbo);
	fb->setDepthAttachment(depthAttachment);
	fb->setWidth(width);
	fb->setHeight(height);

	return fb;

}


std::shared_ptr<Framebuffer> Framebuffer::createMultiSampleFbo(
	unsigned int width,
	unsigned int height, 
	unsigned int sampleNumber
)
{
	std::shared_ptr<Framebuffer> fb = std::make_shared<Framebuffer>();
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	auto colorAttachment = Texture::createMultiSampleTexture(width, height, sampleNumber,GL_RGBA,0);
	auto depthStencilAttachment = Texture::createMultiSampleTexture(width, height, sampleNumber, GL_DEPTH24_STENCIL8, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorAttachment->getTexture(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthStencilAttachment->getTexture(), 0);

	fb->setFBO(fbo);
	fb->setDepthStencilAttachment(depthStencilAttachment);
	fb->setColorAttachment(colorAttachment);
	fb->setWidth(width);
	fb->setHeight(height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return fb;
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
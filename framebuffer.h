#pragma once
#include "core.h"
#include "texture.h"

namespace GLframework
{
	class Framebuffer
	{
	public:
		unsigned int getWidth() const;
		unsigned int getHeight() const;
		unsigned int getFBO() const;
		std::shared_ptr<Texture> getColorAttachment() const;
		std::shared_ptr<Texture> getDepthStencilAttachment() const;
		std::shared_ptr<Texture> getDepthAttachment() const;

		void setWidth(unsigned int value);
		void setHeight(unsigned int value);
		void setFBO(unsigned int value);
		void setColorAttachment(std::shared_ptr<Texture> value);
		void setDepthStencilAttachment(std::shared_ptr<Texture> value);
		void setDepthAttachment(std::shared_ptr<Texture> value);

		static std::shared_ptr<Framebuffer> createShadowFBO(unsigned width, unsigned height);
		static std::shared_ptr<Framebuffer> createCSMShadowFBO(unsigned width, unsigned height,unsigned int layerNum);
		static std::shared_ptr<Framebuffer> createPointLightShadowFBO(unsigned width, unsigned height, unsigned layerCount);
		static std::shared_ptr<Framebuffer> createMultiSampleFbo(unsigned int width, unsigned int height, unsigned int sampleNumber);
		Framebuffer(){}
		Framebuffer(
			unsigned int width,
			unsigned int height
		);
		~Framebuffer()
		{
			if(mFBO)
			{
				glDeleteFramebuffers(1,&mFBO);
			}
		}


	private:
		unsigned int mWidth{ 0 };
		unsigned int mHeight{ 0 };
		unsigned int mFBO { 0 };

		std::shared_ptr<Texture> mColorAttachment{ nullptr };
		std::shared_ptr<Texture> mDepthStencilAttachment{ nullptr };
		std::shared_ptr<Texture> mDepthAttachment{ nullptr };
	};
}

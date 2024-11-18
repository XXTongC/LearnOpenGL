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

		void setWidth(unsigned int value);
		void setHeight(unsigned int value);
		void setFBO(unsigned int value);
		void setColorAttachment(std::shared_ptr<Texture> value);
		void setDepthStencilAttachment(std::shared_ptr<Texture> value);

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
		};


	private:
		unsigned int mWidth{ 0 };
		unsigned int mHeight{ 0 };
		unsigned int mFBO { 0 };

		std::shared_ptr<Texture> mColorAttachment{ nullptr };
		std::shared_ptr<Texture> mDepthStencilAttachment{ nullptr };
	};
}

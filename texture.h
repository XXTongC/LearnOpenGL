#pragma once
#include "core.h"

namespace GLframework
{
	class Texture
	{
	public:
		static std::shared_ptr<Texture> createTexture(const std::string& path, unsigned int unit);
		static std::shared_ptr<Texture> createTextureFromMemory(
			const std::string& path,
			unsigned int unit,
			unsigned char* dataIn,
			uint32_t widthIn,
			uint32_t heightIn
		);
		static std::shared_ptr<Texture> createColorAttachment(
			unsigned int width, 
			unsigned int height, 
			unsigned int unit
		);
		static std::shared_ptr<Texture> createDepthStencilAttachment(
			unsigned int width,
			unsigned int height,
			unsigned int unit
		);

		Texture(){};
		Texture(const std::string& path, unsigned int unit, unsigned int internalFormat = GL_RGBA);
		Texture(
			unsigned int unit,
			unsigned char* dataIn,
			uint32_t widthIn,
			uint32_t heightIn,
			unsigned int internalFormat = GL_RGBA
		);
		Texture(const std::vector<std::string>& paths,unsigned int unit, unsigned int internalFormat = GL_RGBA);
		Texture(unsigned int width, unsigned int height, unsigned int unit, unsigned int internalFormat = GL_RGBA);
		~Texture();
		void Bind();
		void setUnit(unsigned int unit);
		GLuint getTexture() const { return mTexture; }
		void setTexture(GLuint value);
		void setWidth(GLuint value);
		void setHeight(GLuint value);
		void setTextureTarget(unsigned int value);
		unsigned int getTextureTarget() const;

	private:
		GLuint mTexture{ 0 };
		int mWidth{ 0 };
		int mHeight{ 0 };
		unsigned int mUnit{ 0 };
		int mTextureTarget{ GL_TEXTURE_2D };
		//添加静态成员变量
		static std::map<std::string, std::shared_ptr<Texture>> mTextureCache;
	};
}

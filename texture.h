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
		Texture(const std::string& path, unsigned int unit);
		Texture(
			unsigned int unit,
			unsigned char* dataIn,
			uint32_t widthIn,
			uint32_t heightIn
		);
		Texture(unsigned int width, unsigned int height, unsigned int unit);
		~Texture();
		void Bind();
		void setUnit(unsigned int unit);
		GLuint getTexture() const { return mTexture; }
		void setTexture(GLuint value);
		void setWidth(GLuint value);
		void setHeight(GLuint value);

	private:
		GLuint mTexture{ 0 };
		int mWidth{ 0 };
		int mHeight{ 0 };
		int mUnit{ 0 };

		//添加静态成员变量
		static std::map<std::string, std::shared_ptr<Texture>> mTextureCache;
	};
}

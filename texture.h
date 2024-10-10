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

		Texture(const std::string& path, unsigned int unit);
		Texture(
			unsigned int unit,
			unsigned char* dataIn,
			uint32_t widthIn,
			uint32_t heightIn
		);
		~Texture();
		void Bind();
		void setUnit(unsigned int unit);
	private:
		GLuint mTexture{ 0 };
		int mWidth{ 0 };
		int mHeight{ 0 };
		int mUnit{ 0 };

		//添加静态成员变量
		static std::map<std::string, std::shared_ptr<Texture>> mTextureCache;
	};
}

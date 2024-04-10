#pragma once
#include "core.h"

namespace GLframework
{
	class Texture
	{
	public:
		Texture(const std::string& path, unsigned int unit);
		~Texture();
		void Bind();
	private:
		GLuint mTexture{ 0 };
		int mWidth{ 0 };
		int mHeight{ 0 };
		int mUnit{ 0 };
	};
}

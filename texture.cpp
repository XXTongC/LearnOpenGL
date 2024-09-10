#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
GLframework::Texture::Texture(const std::string& path, unsigned int unit)
{
	//声明图片长宽，颜色通道
	int channels;
	mUnit = unit;
	//反转y轴
	stbi_set_flip_vertically_on_load(true);
	//读取图片数据
	unsigned char* data = stbi_load(path.c_str(), &mWidth, &mHeight, &channels, STBI_rgb_alpha);

	//生成纹理
	GL_CALL(glGenTextures(1, &mTexture));
	//激活纹理单元
	GL_CALL(glActiveTexture(GL_TEXTURE0 + mUnit));
	//绑定纹理对象
	GL_CALL(glBindTexture(GL_TEXTURE_2D, mTexture));
	//开辟显存并传输纹理数据
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
	//设置自动MipMap
	GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
	

	//释放RAM上的data
	stbi_image_free(data);
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));

	//如果渲染像素小于原图，则就近原则，成图较清晰，锯齿感强
	//GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 设置纹理包裹方式，设置x, y方向的超原图范围属性, S对应x(u), T对应y(v)
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
	
}

GLframework::Texture::~Texture()
{
	if (mTexture != 0)
	{
		std::cout << "texture delete\n";
		glDeleteTextures(1, &mTexture);
	}
}

void GLframework::Texture::Bind()
{
	//先切换纹理单元，让后绑定纹理单元
	GL_CALL(glActiveTexture(GL_TEXTURE0 + mUnit));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, mTexture));
}

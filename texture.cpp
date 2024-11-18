#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace GLframework;
std::map<std::string, std::shared_ptr<GLframework::Texture>> GLframework::Texture::mTextureCache{};


std::shared_ptr<Texture> Texture::createColorAttachment(
	unsigned width, 
	unsigned height, 
	unsigned unit
)
{
	return std::make_shared<Texture>(width, height, unit);
}



std::shared_ptr<Texture> Texture::createDepthStencilAttachment(
	unsigned int width,
	unsigned int height,
	unsigned int unit
)
{
	std::shared_ptr<Texture> dsTex = std::make_shared<Texture>();

	unsigned int depthStencil;
	glGenTextures(1, &depthStencil);
	glBindTexture(GL_TEXTURE_2D, depthStencil);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	dsTex->setTexture(depthStencil);
	dsTex->setWidth(width);
	dsTex->setHeight(height);
	dsTex->setUnit(unit);

	return dsTex;
}

void Texture::setHeight(GLuint value)
{
	mHeight = value;
}

void Texture::setWidth(GLuint value)
{
	mWidth = value;
}



void Texture::setTexture(GLuint value)
{
	mTexture = value;
}

static std::shared_ptr<Texture> createTexture(
	unsigned int width,
	unsigned int height,
	unsigned int unit
)
{
	return std::make_shared<Texture>(width, height, unit);

}
std::shared_ptr<GLframework::Texture> GLframework::Texture::createTexture(const std::string& path, unsigned unit)
{
	// 1. 检查是否缓存过本路径对象的纹理对象
	auto iter = mTextureCache.find(path);
	if(iter!=mTextureCache.end())
	{
		return iter->second;
	}

	// 2. 如果本六九对应的texture没有生成过，则生成
	auto texture = std::make_shared<Texture>(path, unit);
	mTextureCache[path] = texture;
	return texture;
}

std::shared_ptr<GLframework::Texture> GLframework::Texture::createTextureFromMemory(const std::string& path, unsigned unit, unsigned char* dataIn, uint32_t widthIn, uint32_t heightIn)
{
	// 1. 检查是否缓存过本路径对象的纹理对象
	auto iter = mTextureCache.find(path);
	if (iter != mTextureCache.end())
	{
		return iter->second;
	}

	// 2. 如果本六九对应的texture没有生成过，则生成
	auto texture = std::make_shared<Texture>( unit,dataIn,widthIn,heightIn);
	mTextureCache[path] = texture;
	return texture;

}


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



GLframework::Texture::Texture(unsigned unit, unsigned char* dataIn, uint32_t widthIn, uint32_t heightIn)
{
	//声明图片长宽，颜色通道
	int channels;
	mUnit = unit;
	//反转y轴
	stbi_set_flip_vertically_on_load(true);

	//计算整张图片大小
	//Assimp规定：如果内嵌纹理是png或者jpg格式，height为0，width直接代表整张图片大小
	uint32_t dataInSize = 0;
	if(!heightIn)
	{
		dataInSize = widthIn;
	}else
	{
		//统一认为格式RGBA
		dataInSize = widthIn * heightIn * 4;
	}

	//读取图片数据
	unsigned char* data = stbi_load_from_memory(dataIn,dataInSize,&mWidth,&mHeight,&channels,STBI_rgb_alpha);

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

Texture::Texture(unsigned width, unsigned height, unsigned unit)
{
	mWidth = width;
	mHeight = height;
	mUnit = unit;

	glGenTextures(1, &mTexture);
	glActiveTexture(GL_TEXTURE0 + mUnit);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

void GLframework::Texture::setUnit(unsigned unit)
{
	mUnit = unit;
}

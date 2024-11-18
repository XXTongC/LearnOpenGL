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
	// 1. ����Ƿ񻺴����·��������������
	auto iter = mTextureCache.find(path);
	if(iter!=mTextureCache.end())
	{
		return iter->second;
	}

	// 2. ��������Ŷ�Ӧ��textureû�����ɹ���������
	auto texture = std::make_shared<Texture>(path, unit);
	mTextureCache[path] = texture;
	return texture;
}

std::shared_ptr<GLframework::Texture> GLframework::Texture::createTextureFromMemory(const std::string& path, unsigned unit, unsigned char* dataIn, uint32_t widthIn, uint32_t heightIn)
{
	// 1. ����Ƿ񻺴����·��������������
	auto iter = mTextureCache.find(path);
	if (iter != mTextureCache.end())
	{
		return iter->second;
	}

	// 2. ��������Ŷ�Ӧ��textureû�����ɹ���������
	auto texture = std::make_shared<Texture>( unit,dataIn,widthIn,heightIn);
	mTextureCache[path] = texture;
	return texture;

}


GLframework::Texture::Texture(const std::string& path, unsigned int unit)
{
	//����ͼƬ������ɫͨ��
	int channels;
	mUnit = unit;
	
	//��תy��
	stbi_set_flip_vertically_on_load(true);
	//��ȡͼƬ����
	unsigned char* data = stbi_load(path.c_str(), &mWidth, &mHeight, &channels, STBI_rgb_alpha);

	//��������
	GL_CALL(glGenTextures(1, &mTexture));
	//��������Ԫ
	GL_CALL(glActiveTexture(GL_TEXTURE0 + mUnit));
	//���������
	GL_CALL(glBindTexture(GL_TEXTURE_2D, mTexture));
	//�����Դ沢������������
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
	//�����Զ�MipMap
	GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
	

	//�ͷ�RAM�ϵ�data
	stbi_image_free(data);
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));

	//�����Ⱦ����С��ԭͼ����ͽ�ԭ�򣬳�ͼ����������ݸ�ǿ
	//GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 �������������ʽ������x, y����ĳ�ԭͼ��Χ����, S��Ӧx(u), T��Ӧy(v)
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
	
}



GLframework::Texture::Texture(unsigned unit, unsigned char* dataIn, uint32_t widthIn, uint32_t heightIn)
{
	//����ͼƬ������ɫͨ��
	int channels;
	mUnit = unit;
	//��תy��
	stbi_set_flip_vertically_on_load(true);

	//��������ͼƬ��С
	//Assimp�涨�������Ƕ������png����jpg��ʽ��heightΪ0��widthֱ�Ӵ�������ͼƬ��С
	uint32_t dataInSize = 0;
	if(!heightIn)
	{
		dataInSize = widthIn;
	}else
	{
		//ͳһ��Ϊ��ʽRGBA
		dataInSize = widthIn * heightIn * 4;
	}

	//��ȡͼƬ����
	unsigned char* data = stbi_load_from_memory(dataIn,dataInSize,&mWidth,&mHeight,&channels,STBI_rgb_alpha);

	//��������
	GL_CALL(glGenTextures(1, &mTexture));
	//��������Ԫ
	GL_CALL(glActiveTexture(GL_TEXTURE0 + mUnit));
	//���������
	GL_CALL(glBindTexture(GL_TEXTURE_2D, mTexture));
	//�����Դ沢������������
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
	//�����Զ�MipMap
	GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));


	//�ͷ�RAM�ϵ�data
	stbi_image_free(data);

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));

	//�����Ⱦ����С��ԭͼ����ͽ�ԭ�򣬳�ͼ����������ݸ�ǿ
	//GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 �������������ʽ������x, y����ĳ�ԭͼ��Χ����, S��Ӧx(u), T��Ӧy(v)
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
	//���л�����Ԫ���ú������Ԫ
	GL_CALL(glActiveTexture(GL_TEXTURE0 + mUnit));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, mTexture));
}

void GLframework::Texture::setUnit(unsigned unit)
{
	mUnit = unit;
}

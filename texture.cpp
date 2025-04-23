#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace GLframework;
std::map<std::string, std::shared_ptr<GLframework::Texture>> GLframework::Texture::mTextureCache{};

unsigned int Texture::getUnit() const
{
	return mUnit;
}


std::shared_ptr<Texture> Texture::createDepthAttachment(unsigned width, unsigned height, unsigned unit)
{
	std::shared_ptr<Texture> dsTex = std::make_shared<Texture>();

	unsigned int depth;
	glGenTextures(1, &depth);
	glBindTexture(GL_TEXTURE_2D, depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT,GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLfloat borderColor[] = { 1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	//u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);	//v

	dsTex->setTexture(depth);
	dsTex->setWidth(width);
	dsTex->setHeight(height);
	dsTex->setUnit(unit);

	return dsTex;
}


unsigned int Texture::getTextureTarget() const
{
	return mTextureTarget;
}

void Texture::setTextureTarget(unsigned value)
{
	mTextureTarget = value;
}

//�ҡ����ϡ��¡���ǰ��+x��-x��+y��-y��+z��-z��
Texture::Texture(const std::vector<std::string>& paths, unsigned int unit, unsigned int internalFormat)
{
	//����ͼƬ������ɫͨ��
	mUnit = unit;
	setTextureTarget(GL_TEXTURE_CUBE_MAP);
	//cubemap����Ҫ��תy��
	stbi_set_flip_vertically_on_load(false);
	//1. ����cubemap����
	GL_CALL(glGenTextures(1, &mTexture));
	GL_CALL(glActiveTexture(GL_TEXTURE0 + mUnit));
	GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture));

	//2. ѭ����ȡ������ͼ�����䴢�����Դ���
	int channels;
	int width = 0, height = 0;
	unsigned char* data = nullptr;
	for(int i = 0;i<paths.size();i++)
	{
		data = stbi_load(paths[i].c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if(data!=nullptr)
		{
			GL_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
			//�ͷ�RAM�ϵ�data
			stbi_image_free(data);
			
		}else
		{
			std::cerr << "Error: CubeMap Texture failed to load at path - " << paths[i] << std::endl;
			stbi_image_free(data);
		}
	}
	//3. �����������
	GL_CALL(glTexParameteri(getTextureTarget(), GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(getTextureTarget(), GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	//�������������ʽ������x, y����ĳ�ԭͼ��Χ����, S��Ӧx(u), T��Ӧy(v)
	GL_CALL(glTexParameteri(getTextureTarget(), GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(getTextureTarget(), GL_TEXTURE_WRAP_T, GL_REPEAT));
}

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
		std::cout << "\"" << path << " �����������������\n";
		return iter->second;
	}

	// 2. ��������Ŷ�Ӧ��textureû�����ɹ���������
	auto texture = std::make_shared<Texture>(path, unit,GL_SRGB_ALPHA);
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
	auto texture = std::make_shared<Texture>( unit,dataIn,widthIn,heightIn, GL_SRGB_ALPHA);
	mTextureCache[path] = texture;
	return texture;

}


GLframework::Texture::Texture(const std::string& path, unsigned int unit, unsigned int internalFormat)
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
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
	//�����Զ�MipMap
	//GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
	

	//�ͷ�RAM�ϵ�data
	stbi_image_free(data);
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

	//�����Ⱦ����С��ԭͼ����ͽ�ԭ�򣬳�ͼ����������ݸ�ǿ
	//GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 �������������ʽ������x, y����ĳ�ԭͼ��Χ����, S��Ӧx(u), T��Ӧy(v)
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));
	
}



GLframework::Texture::Texture(unsigned unit, unsigned char* dataIn, uint32_t widthIn, uint32_t heightIn, unsigned int internalFormat)
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
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
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

Texture::Texture(unsigned int width, unsigned int height, unsigned int unit, unsigned int internalFormat)
{
	mWidth = width;
	mHeight = height;
	mUnit = unit;

	glGenTextures(1, &mTexture);
	glActiveTexture(GL_TEXTURE0 + mUnit);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

std::shared_ptr<Texture> Texture::createDepthAttachmentCSMArray(
	unsigned int width,
	unsigned int height,
	unsigned int layerNum,
	unsigned int unit
)
{
	/*
	std::shared_ptr<Texture> dTex = std::make_shared<Texture>();

	unsigned int depth;
	glGenTextures(1, &depth);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depth);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, width, height, layerNum, 0, GL_DEPTH_COMPONENT, GL_FLOAT,nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLfloat borderColor[] = { 1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	//u
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);	//v

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	dTex->setTexture(depth);
	dTex->setWidth(width);
	dTex->setHeight(height);
	dTex->setUnit(unit);
	dTex->setTextureTarget(GL_TEXTURE_2D_ARRAY);

	return dTex;
	*/

	std::shared_ptr<Texture> dTex = createTexture2DArray(width, height, layerNum, unit, GL_DEPTH_COMPONENT32F);

	glBindTexture(GL_TEXTURE_2D_ARRAY, dTex->getTexture());

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLfloat borderColor[] = { 1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	return dTex;
}


GLframework::Texture::~Texture()
{
	if (mTexture != 0)
	{
		std::cout << "texture delete\n";
		glDeleteTextures(1, &mTexture);
	}
}

GLuint Texture::getHeight() const
{
	return mHeight;
}

GLuint Texture::getWidth() const
{
	return mWidth;
}


void GLframework::Texture::Bind()
{
	//change texture unit first, bind texture unit after
	GL_CALL(glActiveTexture(GL_TEXTURE0 + mUnit));
	GL_CALL(glBindTexture(mTextureTarget, mTexture));
}

void GLframework::Texture::setUnit(unsigned unit)
{
	mUnit = unit;
}


std::shared_ptr<Texture> Texture::createTexture2DArray(unsigned int width, unsigned int height, unsigned int layers, unsigned int unit, unsigned int internalFormat)
{
	std::shared_ptr<Texture> tex = std::make_shared<Texture>();

	
	glGenTextures(1, &tex->mTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex->mTexture);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height, layers, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	std::cout << "layers = " << layers << std::endl;

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

	
	tex->mWidth = width;
	tex->mHeight = height;
	tex->mLayers = layers;
	tex->mUnit = unit;
	tex->mTextureTarget = GL_TEXTURE_2D_ARRAY;
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	return tex;
}

void Texture::setLayerCount(unsigned int layers)
{
	mLayers = layers;
}

unsigned int Texture::getLayerCount() const
{
	return mLayers;
}

std::shared_ptr<Texture>  Texture::createDepthCubemap(unsigned int size, unsigned int unit)
{
	std::shared_ptr<Texture> cubemapTex = std::make_shared<Texture>();

	glGenTextures(1, &cubemapTex->mTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex->mTexture);

	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	cubemapTex->setWidth(size);
	cubemapTex->setHeight(size);
	cubemapTex->setUnit(unit);
	cubemapTex->setTextureTarget(GL_TEXTURE_CUBE_MAP);

	return cubemapTex;
}
std::shared_ptr<Texture> Texture::createMultiSampleTexture(
	unsigned int width,
	unsigned int height,
	unsigned int samplesNumber,
	unsigned int format,
	unsigned int unit
)
{
	std::shared_ptr<Texture> tex = std::make_shared<Texture>();

	GLuint glTex;
	glGenTextures(1, &glTex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE , glTex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samplesNumber, format, width, height, GL_TRUE);

	tex->setTexture(glTex);
	tex->mWidth = width;
	tex->mHeight = height;
	tex->mUnit = unit;
	tex->mTextureTarget = GL_TEXTURE_2D_MULTISAMPLE;
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	return tex;
}
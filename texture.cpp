#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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

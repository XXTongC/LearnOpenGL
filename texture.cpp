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
	glGenTextures(1, &mTexture);
	//��������Ԫ
	glActiveTexture(GL_TEXTURE0 + mUnit);
	//���������
	glBindTexture(GL_TEXTURE_2D, mTexture);
	//�����Դ沢������������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	//�ͷ�RAM�ϵ�data
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//�����Ⱦ����С��ԭͼ����ͽ�ԭ�򣬳�ͼ����������ݸ�ǿ
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 �������������ʽ������x, y����ĳ�ԭͼ��Χ����, S��Ӧx(u), T��Ӧy(v)
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
}

GLframework::Texture::~Texture()
{
	if (mTexture != 0)
		glDeleteTextures(1, &mTexture);
}

void GLframework::Texture::Bind()
{
	//���л�����Ԫ���ú������Ԫ
	glActiveTexture(GL_TEXTURE0 + mUnit);
	glBindTexture(GL_TEXTURE_2D, mUnit);
}

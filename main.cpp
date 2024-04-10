#include "core.h"
#include <iostream>
#include "GL_ERROR_FIND.h"
#include "Application.h"
#include "shader.h"
#include "OldTestCode.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


/*
 * to make main.cpp clear
 * if u need the code belong to past classes
 * check file "OldTestCode.h/.cpp"
 *
*/


//vao��vbo������

void prepareUVGLTranglesTest2();
void prepareTextureTest2();
//shader�Ĵ���
void prepareShader();
//��������
void render();
//������
void prepareTexture();

//TextureTest 1 ����ƽ��



//����ȫ�ֱ���vao�Լ�shaderProram
GLuint vao,Texture;
GLShaderwork::Shader* shader = nullptr;

int main()
{
	if (!GL_APP->init(640, 480)) return -1;
	GL_APP->setResizeCallback(OnResize);
	GL_APP->setKeyboardCallback(OnKeyboardCallback);
	GL_APP->setMouseCallback(OnMouseCallback);
	prepareShader();
	prepareUVGLTranglesTest2();
	prepareTextureTest2();
	GL_CALL(glViewport(0, 0, 640, 480));
	GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
	while (GL_APP->update())
	{
		render();
	}

	GL_APP->destory();
	return 0;
}

void prepareShader()
{
	shader = new GLShaderwork::Shader("vertexAuto.glsl", "fragmentAuto.glsl");
}

void prepareUVGLTrangles()
{
	float vertex[]{
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,0.5f,0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,0.25f,1.0f,
	};
	int indexes[]
	{
		0, 1, 2,
	};
	//�Ȱ󶨺�vao
	GL_CALL(glGenVertexArrays(1, &vao));
	GL_CALL(glBindVertexArray(vao));
	//װ��vbo
	GLuint vbo{ 0 };
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW));

	//��̬��ȡ���location
	GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
	GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");
	GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");

	//����vao��ۣ�����װ������
	GL_CALL(glEnableVertexAttribArray(positionLocation));
	GL_CALL(glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0));
	GL_CALL((glEnableVertexAttribArray(colorLocation)));
	GL_CALL(glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))));
	GL_CALL(glEnableVertexAttribArray(uvLocation));
	GL_CALL(glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))));

	//����ebo, �ڵ�ǰ�԰�vao������°�eboʱebo���Զ�������vao
	GLuint ebo{ 0 };
	GL_CALL(glGenBuffers(1, &ebo));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW));

	//���vao
	GL_CALL(glBindVertexArray(0));
}




void render()
{
	/*
	* glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	* ����һ�����ܼ����������ᱨ�����½��ͣ�
	* �ڵ�һ֡��ʱ�����ǽ�����GL״̬����ebo��󣬵���״̬����Ȼ��ͨ��vao��ȡebo��Ϣ
	* ���Ե�һ֡������������ͼƬ
	* �����˵ڶ�֡�����ǽ�vao�е�eboҲ����ˣ�ʹ�õڶ�֡�޷���ȡ����Ч��ebo������
	*/
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	//1 �󶨵�ǰprogram
	
	shader->begin();
	//����shader�Ĳ�����Ϊ0�Ų�����
	shader->setInt("sampler", 0);
	shader->setFloat("time", glfwGetTime());
	shader->setFloat("speed", 0.5);
	//2 �󶨵�ǰvao

	GL_CALL(glBindVertexArray(vao));
	//��������ָ��
	//GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
	//GL_TRIANGLE_STRIP ���ĩβ�����nΪż���������ӹ���Ϊ[n-2,n-1,n]
	//����Ϊ[n-1,n-2,n]
	//��֤��˳ʱ�����ʱ�������˳����ͬ
	//GL_TRIANGLE_FAN ������V0Ϊ��������
	//GL_CALL(glDrawArrays(GL_SMOOTH_LINE_WIDTH_GRANULARITY, 0, 1));

	GL_CALL(glDrawElements(GL_TRIANGLES,6 , GL_UNSIGNED_INT,static_cast<void*>(0)));
	
	
	//�����������������ָ���ô����������˵�Ĵ���Ϳ��Ա���
	//����ʾ��һ����OpenGL�н����Ĵ���ϰ�ߣ��п��й�
	GL_CALL(glBindVertexArray(0));
	shader->end();
}

void prepareTexture()
{
	//1 ��ȡstbImage 
	int width, height, channels;

	//��תY�ᣬԭ����ͼƬ�Ĵ洢�Ǵ����Ͻǿ�ʼ����OpenGL��ͼƬ��ȡ�Ǵ����½ǿ�ʼ
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("Texture/panda.jpg", &width, &height, &channels, STBI_rgb_alpha);
	
	if (data == nullptr) std::cout << "nullptr" << std::endl;
	//2 ���������Ҽ��Ԫ��
	GL_CALL(glGenTextures(1, &Texture));
	//��������Ԫ
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	//���������
	GL_CALL(glBindTexture(GL_TEXTURE_2D, Texture));
	//3 �����������ݣ���ʱ�ŻῪ���Դ�
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
	
	//�ͷ����ݣ���ʱdata�����Ǵ洢��RAM�ϵģ��������������Ѿ����俽������GPURAM�ϣ���������������Դ���GPURAM�����ݣ����Կ��Ե�����������ݣ�����ʹ������ָ������������
	stbi_image_free(data);

	//4 ����������˷�ʽ
	//�����Ⱦ���ش���ԭͼ�����ò�ֵ�㷨����ͼ��ģ������ݸ���
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//�����Ⱦ����С��ԭͼ����ͽ�ԭ�򣬳�ͼ����������ݸ�ǿ
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 �������������ʽ������x, y����ĳ�ԭͼ��Χ����, S��Ӧx(u), T��Ӧy(v)
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));

	/*
	 *	С�����ﲻ�ܽ��Texture����Ϊ���Ǽ�����һ������Ԫ��������ǽ������Ԫ�ͻ�ʧȥ����ʹ��shader�����޷�����
	 *	GL_CALL(glBindTexture(GL_TEXTURE_2D,0));
����������*/
}

void prepareUVGLTranglesTest2()
{
	float vertex[]
{
		-1.0f,-1.0f,0.0f,0.0f,0.0f,
		-1.0f,1.0f,0.0f,0.0f,1.0f,
		1.0f,1.0f,0.0f,1.0f,1.0f,
		1.0f,-1.0f,0.0f,1.0f,0.0f,
	};
	int index[]
	{
		0,1,2,
		0,2,3,
	};

	GLuint vbo{ 0 }, uv{ 0 };
	//�Ƚ�vao��
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//��ʼ��vbo��ע�����ݵȲ���
	GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
	GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(positionLocation);
	glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(uvLocation);
	glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glGenBuffers(1, &uv);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uv);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void prepareTextureTest2()
{
	//����ͼƬ������ɫͨ��
	int width, height, channels;
	//��תy��
	stbi_set_flip_vertically_on_load(true);
	//��ȡͼƬ����
	unsigned char* data = stbi_load("Texture/panda.jpg", &width, &height, &channels, STBI_rgb_alpha);

	//��������
	glGenTextures(1, &Texture);
	//��������Ԫ
	glActiveTexture(GL_TEXTURE0);
	//���������
	glBindTexture(GL_TEXTURE_2D, Texture);
	//�����Դ沢������������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,data);

	//�ͷ�RAM�ϵ�data
	stbi_image_free(data);

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//�����Ⱦ����С��ԭͼ����ͽ�ԭ�򣬳�ͼ����������ݸ�ǿ
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 �������������ʽ������x, y����ĳ�ԭͼ��Χ����, S��Ӧx(u), T��Ӧy(v)
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

}
#include "core.h"
#include <iostream>
#include "GL_ERROR_FIND.h"
#include "Application.h"
#include "shader.h"
#include "OldTestCode.h"
#include "texture.h"



/*
 * to make main.cpp clear
 * if u need the code belong to past classes
 * check file "OldTestCode.h/.cpp"
 *
*/

bool setAndInitWindow(int weith = 800,int height = 600);
//vao��vbo������

void prepareUVGLTranglesTest2();
//shader�Ĵ���
void prepareShader();
//��������
void render();
//������
void prepareTexture();
//TextureTest 1 ����ƽ��
//������תƽ�ƵȲ���
void doRotateTransform(); //��ת�任
void doTranslationTransform(); //ƽ�Ʊ任
void doScaleTransform();
void doTransform();
void doRotate();
void doTestTransform();


//����ȫ�ֱ���vao�Լ�shaderProram
GLuint vao;
float angle = 0.0f;
GLframework::Shader* shader = nullptr;
GLframework::Texture* textureGrass = nullptr;
GLframework::Texture* textureLand = nullptr;
GLframework::Texture* textureNoise = nullptr;

glm::mat4 transform(1.0f);

int main()
{

	//����
	glm::vec2 v0{ 0 };
	glm::mat2x3 nm{ 1.0 };
	std::cout << glm::to_string(nm)<<std::endl;

	std::cout << "Please set the window as x * y" << std::endl;
	int width = 800, height = 600;
	//std::cin >> width >> height;
	//��ʼ��GLFW����
	if (!setAndInitWindow(width,height)) return -1;
	
	prepareShader();
	prepareUVGLTranglesTest2();
	prepareTexture();
	GL_CALL(glViewport(0, 0, width, height));
	GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
	while (GL_APP->update())
	{
		render();
	}

	GL_APP->destory();

	delete textureGrass;
	delete textureLand;
	delete textureNoise;

	return 0;
}

bool setAndInitWindow(int weith, int height)
{
	if (!GL_APP->init(weith,height)) return false;
	GL_APP->setResizeCallback(OnResize);
	GL_APP->setKeyboardCallback(OnKeyboardCallback);
	GL_APP->setMouseCallback(OnMouseCallback);
	return true;
}


void prepareShader()
{
	shader = new GLframework::Shader("vertexAuto.glsl", "fragmentAuto.glsl");
}

void prepareTexture()
{
	textureGrass = new GLframework::Texture("Texture/grass.jpg", 0);
	textureLand = new GLframework::Texture("Texture/land.jpg", 1);
	textureNoise = new GLframework::Texture("Texture/noise.jpg", 2);
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
	shader->setInt("samplerGrass", 0);
	shader->setInt("samplerLand", 1);
	shader->setInt("samplerNoise", 2);
	
	// ��ע��������Ǿ����ʼ����һ�����ӣ����ʹ�õ���0.9.9�����ϰ汾
	// �������д������Ҫ��Ϊ:
	
	// ֮�󽫲��ٽ�����ʾ
	//trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
	doTestTransform();
	//trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
	shader->setMat4("transform", transform);
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
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GL_CALL(glDrawElements(GL_TRIANGLES,6 , GL_UNSIGNED_INT,static_cast<void*>(0)));
	
	
	//�����������������ָ���ô����������˵�Ĵ���Ϳ��Ա���
	//����ʾ��һ����OpenGL�н����Ĵ���ϰ�ߣ��п��й�
	GL_CALL(glBindVertexArray(0));
	shader->end();
}

void prepareUVGLTranglesTest2()
{
	float vertex[]
{
		-0.5f,-0.5f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,
		-0.5f,0.5f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,
		0.5f,0.5f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,
		0.5f,-0.5f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,
	};
	int index[]
	{
		0,1,2,
		0,2,3,
	};

	GLuint vbo{ 0 }, ebo{ 0 };
	//�Ƚ�vao��
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//��ʼ��vbo��ע�����ݵȲ���
	
	GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
	GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");
	GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(positionLocation);
	glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(colorLocation);
	glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(uvLocation);
	glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void doRotateTransform()
{
	//�ڶ���������Ҫ�����Ӧ���ǻ�����
	transform = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3{0.0f,0.0f,1.0f});
}

void doTranslationTransform()
{
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, 0.3f, 0.0f));
}

void doScaleTransform()
{
	transform = glm::scale(glm::mat4(1.0f), glm::vec3{0.5f,0.5f,1.0f});
}

void doTransform()
{
	auto transformRotate = glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(glfwGetTime()) * 10), glm::vec3{ 0.0f,0.0f,1.0f });
	auto transformTranslate = glm::translate(glm::mat4(1.0f), glm::vec3{0.5f,0.5f,0.0f});
	auto transformScale = glm::scale(glm::mat4(1.0f), glm::vec3{0.5f,0.5f,1.0f});
	transform = transformRotate * transformScale * transformTranslate;
}

void doRotate()
{
	//ÿ֡���¹�����ת����
	angle += 0.01f;
	transform = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3{ 0.0f,0.0f,1.0f });
}

void doTestTransform()
{
	//Ŀ��һ����ת��������
	transform = glm::rotate(transform, glm::radians(0.1f),glm::vec3{0.0f,0.0f,1.0f});


}
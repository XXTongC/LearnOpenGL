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
//׼������ͷ����
void prepareCamera();

//����ȫ�ֱ���vao�Լ�shaderProram
GLuint vao;
float angle = 0.0f;
GLframework::Shader* shader = nullptr;
GLframework::Texture* textureGrass = nullptr;
GLframework::Texture* textureLand = nullptr;
GLframework::Texture* textureNoise = nullptr;

glm::mat4 transform(1.0f);
glm::mat4 viewMatrix(1.0f);



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
	prepareCamera();
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
	shader->setMat4("transform", transform);
	shader->setMat4("viewMatrix", viewMatrix);
	//doTestTransform();

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
	GL_CALL(glDrawElements(GL_TRIANGLES,3 , GL_UNSIGNED_INT,static_cast<void*>(0)));
	
	
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
		0.5f,-0.5f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,
		0.0f,0.5f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,
	};
	int index[]
	{
		0,1,2,
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

void prepareCamera()
{
	//lookAt(): ����һ��viewMatrix
	//eye: ��ǰ���������λ��
	//center: ��ǰ�����������Ǹ���
	//up: 񷶥����
	viewMatrix = glm::lookAt(glm::vec3{ 0.0f, 0.0f, 0.5f }, glm::vec3{ 0.0f,0.0f,0.0f }, glm::vec3{ 0.0f,1.0f,0.0f });
	
}

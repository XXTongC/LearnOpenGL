#include "core.h"
#include <iostream>
#include <typeinfo>
#include "GL_ERROR_FIND.h"
#include "Application.h"
#include "shader.h"
#include "OldTestCode.h"
#include "texture.h"
#include "perspectivecamera.h"
#include "orthographiccamera.h"
#include "trackBallCameraControl.h"
#include "gamecameracontrol.h"
#include "geometry.h"

/*
 * to make main.cpp clear
 * if u need the code belong to past classes
 * check file "OldTestCode.h/.cpp"
 *
*/

#pragma region ���ֻص�����
void OnScroll(double offset);
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void OnResize(int width, int height);
void OnKeyboardCallback(int key, int action, int mods);
void OnMouseCallback(int button, int action, int mods);
void OnCursor(double xpos, double ypos);
#pragma endregion

bool setAndInitWindow(int weith = 800,int height = 600);
//vao��vbo������
void prepareUVGLTranglesTest2();
void prepareVao();
//shader�Ĵ���
void prepareShader();
//��������
void render();
//������
void prepareTexture();
//׼������ͷ����
void prepareCamera();
//׼������ͶӰ����
void prepareOrtho();
//״̬���ó�ʼ��
void prepareState();

//����ȫ�ֱ���vao�Լ�shaderProram
//GLuint vao;
float angle = 0.0f;
GLframework::Shader* shader = nullptr;
GLframework::Texture* textureGrass = nullptr;
GLframework::Texture* textureLand = nullptr;
GLframework::Texture* textureNoise = nullptr;
GLframework::Texture* texturePanda = nullptr;
Geometry* geomety = nullptr;
Camera* camera = nullptr;
CameraControl* cameracontrol = nullptr;
glm::mat4 transformGrass(1.0f);
glm::mat4 transformPanda(1.0f);
glm::mat4 ortherMartix(1.0f);

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

	//����opengl �ӿڲ���������ɫ
	GL_CALL(glViewport(0, 0, width, height));
	GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));

	prepareShader();
	prepareVao();
	prepareTexture();
	prepareCamera();
	prepareState();

	//���Ի�ȡ���Կ������ṩ��Arrribbutes����
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
	
	while (GL_APP->update())
	{
		cameracontrol->update();
		render();
	}

	GL_APP->destory();

	return 0;
}

bool setAndInitWindow(int weith, int height)
{
	if (!GL_APP->init(weith,height)) return false;
	GL_APP->setResizeCallback(OnResize);
	GL_APP->setKeyboardCallback(OnKeyboardCallback);
	GL_APP->setMouseCallback(OnMouseCallback);
	GL_APP->setCursorCallback(OnCursor);
	GL_APP->setScrollCallback(OnScroll);
	
	//glClearDepth(0.0);
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
	//texturePanda = new GLframework::Texture("Texture/panda.jpg", 0);
}

void prepareState()
{
	//��ȼ������
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
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
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT));

	//1 �󶨵�ǰprogram
	
	shader->begin();
	//����shader�Ĳ�����Ϊ0�Ų�����
	shader->setInt("samplerGrass", 0);
	shader->setInt("samplerLand", 1);
	shader->setInt("samplerNoise", 2);
	shader->setMat4("transform", transformGrass);
	shader->setMat4("viewMatrix", camera->getViewMatrix());
	shader->setMat4("projectionMatrix", camera->getProjectionMatrix());

	//std::cout << glm::to_string(camera->getViewMatrix()) << std::endl;
	//std::cout << glm::to_string(camera->getProjectionMatrix()) << std::endl;
	shader->setFloat("time", glfwGetTime());
	shader->setFloat("speed", 0.5);
	//2 �󶨵�ǰvao
	textureGrass->Bind();
	GL_CALL(glBindVertexArray(geomety->getVao()));
	//��������ָ��
	//GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
	//GL_TRIANGLE_STRIP ���ĩβ�����nΪż���������ӹ���Ϊ[n-2,n-1,n]
	//����Ϊ[n-1,n-2,n]
	//��֤��˳ʱ�����ʱ�������˳����ͬ
	//GL_TRIANGLE_FAN ������V0Ϊ��������
	//GL_CALL(glDrawArrays(GL_SMOOTH_LINE_WIDTH_GRANULARITY, 0, 1));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
	GL_CALL(glDrawElements(GL_TRIANGLES,geomety->getIndicesCount() , GL_UNSIGNED_INT,static_cast<void*>(0)));

	//transformPanda = glm::translate(glm::mat4(1.0f), glm::vec3{ 0.8f,0.0f,-1.0f });
	//shader->setMat4("transform", transformPanda);
	//texturePanda->Bind();
	//GL_CALL(glDrawElements(GL_TRIANGLES, geomety->getIndicesCount(), GL_UNSIGNED_INT, static_cast<void*>(0)));

	//�����������������ָ���ô����������˵�Ĵ���Ϳ��Ա���
	//����ʾ��һ����OpenGL�н����Ĵ���ϰ�ߣ��п��й�
	GL_CALL(glBindVertexArray(0));
	shader->end();
}

void prepareVao()
{
	geomety = Geometry::createPlane(shader,1,1);
}

void prepareCamera()
{

	camera = new PerspectiveCamera(
		60.0f,
		static_cast<float>(GL_APP->getWidth()) / static_cast<float>(GL_APP->getHeight()),
		0.1f,
		1000.0f
	);
	
	//float size = 6.0f;
	//camera = new OrthographicCamera(-size,size,size,-size,size,-size);
	cameracontrol = new TrackBallCameraControl();
	cameracontrol->setCamera(camera);
}
void prepareOrtho()
{
	ortherMartix = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f);
}

#pragma region �ص���������
//�����֣��������ص�����
void OnScroll(double offset)
{
	cameracontrol->onScroll(offset);
}

void OnResize(int width, int height)
{
	GL_CALL(glViewport(0, 0, width, height));
	std::cout << "OnResize" << std::endl;
}

void OnKeyboardCallback(int key, int action, int mods)
{
	GL_CALL();
	std::cout << "OnKeyboardCallback Pressed: " << key << " " << action << " " << mods << std::endl;
	cameracontrol->onKey(key, action, mods);
}

void OnMouseCallback(int button, int action, int mods)
{
	double x, y;
	GL_APP->getCursorPosition(&x, &y);
	std::cout << "OnMouseCallback : " << button << " " << action << " " << mods << std::endl;
	cameracontrol->onMouse(button, action,x, y);
	if(CameraControl* control=dynamic_cast<GameCameraControl*>(cameracontrol))
	{
		if(PerspectiveCamera* icamera = dynamic_cast<PerspectiveCamera*>(camera))
		{
			if(button==GLFW_MOUSE_BUTTON_MIDDLE&&action==GLFW_PRESS)
			{
				if(icamera!=nullptr)
				{
					icamera->setFovy(icamera->mFovy / 2.0f);
				}
			}
			else if(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
			{
				if (icamera != nullptr)
				{
					icamera->setFovy(icamera->mFovy * 2.0f);
				}
			}
		}
	}
	
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		std::cout << "press the bottom" << std::endl;
	else if (action == GLFW_RELEASE)
		std::cout << "release the bottom" << std::endl;
	if (mods == GLFW_MOD_CONTROL)
		std::cout << "press the ctrl and key" << std::endl;
	std::cout << "Pressed: " << key << std::endl;
	std::cout << "Action: " << action << std::endl;
	std::cout << "Mods: " << mods << std::endl;
}

void OnCursor(double xpos, double ypos)
{
	//std::cout << "(" << xpos << ", " << ypos << ")" << std::endl;
	cameracontrol->onCursor(xpos, ypos);
}
#pragma endregion
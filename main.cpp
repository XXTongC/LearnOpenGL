#include "core.h"
#include <iostream>
#include <windows.h>
#include <memory>
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
#include "mesh.h"
#include "phongMaterial.h"
#include "material.h"


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
//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
#pragma endregion

bool setAndInitWindow(int weith = 800,int height = 600);

//��������
void render();

//׼������ͷ����
void prepareCamera();

//״̬���ó�ʼ��
void prepareState();

void prepare();

//����ȫ�ֱ���vao�Լ�shaderProram
//GLuint vao;
float angle = 0.0f;
GLframework::Shader* shader = nullptr;

GLframework::Geometry* geomety = nullptr;
Camera* camera = nullptr;
CameraControl* cameracontrol = nullptr;



float specularIntensity = 0.5f;
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

void perpare()
{
	//1. ����geogetry
	std::shared_ptr<GLframework::Geometry> geometry{ GLframework::Geometry::createSphere(shader, 3.0f) };

	//2. ����material�������ò���
	auto matertial{ std::make_shared<GLframework::PhongMaterial>() };
	matertial->mShiness = 32.0f;
	matertial->mDiffuse = new GLframework::Texture("panda.jpg",0);
	//3. ����mesh
	auto mesh{ std::make_shared<GLframework::Mesh>(geometry.get(),matertial.get()) };

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





void prepareState()
{
	//��ȼ������
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void render()
{

	GL_CALL(glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT));


}


void prepareCamera() 
{
	
	camera = new PerspectiveCamera(
		60.0f,
		static_cast<float>(GL_APP->getWidth()) / static_cast<float>(GL_APP->getHeight()),
		0.1f,
		1000.0f
	);
	
	
	//std::cout << "APP SIZE : " << GL_APP->getWidth() << ":" << GL_APP->getHeight() << std::endl;
	//float size = 10.0f;
	//camera = new OrthographicCamera(-size,size,size,-size,size,-size);
	
	cameracontrol = new TrackBallCameraControl();
	cameracontrol->setCamera(camera);
}
//void prepareOrtho()
//{
//	ortherMartix = glm::ortho(-2.0f, 2.0f, 2.0f, -2.0f, 2.0f, -2.0f);
//}

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
	GL_CALL(cameracontrol->onKey(key, action, mods));
	std::cout << "OnKeyboardCallback Pressed: " << key << " " << action << " " << mods << std::endl;
	
}

void OnMouseCallback(int button, int action, int mods)
{
	double x, y;
	GL_APP->getCursorPosition(&x, &y);
	std::cout << "OnMouseCallback : " << button << " " << action << " " << mods << std::endl;
	cameracontrol->onMouse(button, action,x, y);
	if(CameraControl* control=dynamic_cast<GameCameraControl*>(cameracontrol))
	{
		std::cout << "call1\n";
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



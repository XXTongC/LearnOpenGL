#include "core.h"
#include <iostream>
#include <windows.h>
#include <memory>
#include <vector>
#include <typeinfo>
#include "GL_ERROR_FIND.h"
#include "Application.h"
#include "shader.h"
#include "texture.h"
#include "perspectivecamera.h"
#include "orthographiccamera.h"
#include "trackBallCameraControl.h"
#include "gamecameracontrol.h"
#include "geometry.h"
#include "mesh.h"
#include "phongMaterial.h"
#include "material.h"
#include "renderer.h"

/*
 * to make main.cpp clear
 * if u need the code belong to past classes
 * check file "OldTestCode.h/.cpp"
 *
*/

#pragma region 部分回调函数
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

//绘制命令
void render();

//准备摄像头坐标
void prepareCamera();

//状态设置初始化
void prepareState();

void prepare();

//声明全局变量vao以及shaderProram
//GLuint vao;
float angle = 0.0f;
std::shared_ptr < GLframework::Renderer> renderer = nullptr;
std::vector<std::shared_ptr < GLframework::Mesh>> meshes{};
std::shared_ptr < GLframework::DirectionalLight> dirLight = nullptr;
std::shared_ptr <GLframework::AmbientLight> ambientLight = nullptr;
Camera* camera = nullptr;
CameraControl* cameracontrol = nullptr;



float specularIntensity = 0.5f;
int main()
{
	std::cout << "Please set the window as x * y" << std::endl;
	int width = 800, height = 600;
	//std::cin >> width >> height;
	//初始化GLFW窗口
	if (!setAndInitWindow(width,height)) return -1;

	//设置opengl 视口并且清理颜色
	GL_CALL(glViewport(0, 0, width, height));
	GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));


	prepareCamera();

	prepare();

	//测试获取该显卡驱动提供的Arrribbutes数量
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
	
	while (GL_APP->update())
	{
		meshes[3]->rotateX(0.01f);
		meshes[3]->rotateY(0.1f);
		cameracontrol->update();
		renderer->render(meshes,camera,dirLight,ambientLight);
	}

	GL_APP->destory();

	return 0;
}

void prepare()
{
	renderer = std::make_shared<GLframework::Renderer>();

	//1. 创建material并且配置参数
	auto matertialGrass = std::make_shared<GLframework::PhongMaterial>();
	matertialGrass->mShiness = 10.0f;
	matertialGrass->mDiffuse = std::make_shared<GLframework::Texture>("Texture/grass.jpg", 0);

	auto matertialLand = std::make_shared<GLframework::PhongMaterial>();
	matertialLand->mShiness = 10.0f;
	matertialLand->mDiffuse = std::make_shared<GLframework::Texture>("Texture/land.jpg", 1);

	auto matertialNoise = std::make_shared<GLframework::PhongMaterial>();
	matertialNoise->mShiness = 10.0f;
	matertialNoise->mDiffuse = std::make_shared<GLframework::Texture>("Texture/noise.jpg", 2);
	
	//wall
	auto matertialWall = std::make_shared<GLframework::PhongMaterial>();
	matertialWall->mShiness = 10.0f;
	matertialWall->mDiffuse = std::make_shared<GLframework::Texture>("Texture/wall.jpg", 0);
	

	//2. 创建geogetry
	auto geometry = GLframework::Geometry::createSphere(renderer->getShader(matertialGrass->getMaterialType()), 2.0f);

	//3. 生成mesh
	auto meshwall = std::make_shared<GLframework::Mesh>(geometry, matertialWall);
	meshwall->setPosition(glm::vec3(4.0f, 0.0f, 0.0f));
	meshes.push_back(std::make_shared<GLframework::Mesh>(geometry,matertialGrass));
	meshes.push_back(std::make_shared<GLframework::Mesh>(geometry, matertialLand));
	meshes.push_back(std::make_shared<GLframework::Mesh>(geometry, matertialNoise));
	meshes.push_back(std::move(meshwall));

	
	dirLight = std::make_shared<GLframework::DirectionalLight>();
	ambientLight = std::make_shared<GLframework::AmbientLight>();
	ambientLight->setColor(glm::vec3{ 0.01f });
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
	//深度检测设置
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
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

#pragma region 回调函数定义
//鼠标滚轮（放缩）回调函数
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



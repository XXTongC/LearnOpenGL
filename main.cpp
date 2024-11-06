#include "core.h"
#include <iostream>
#include <windows.h>
#include <memory>
#include <vector>
#include <typeinfo>
#include "GL_ERROR_FIND.h"
#include "Application.h"
#include "tools.h"
#include "shader.h"
#include "texture.h"
#include "perspectivecamera.h"
#include "orthographiccamera.h"
#include "trackBallCameraControl.h"
#include "opacityMaskMatetial.h"
#include "screenMaterial.h"
#include "gamecameracontrol.h"
#include "geometry.h"
#include "mesh.h"
#include "phongMaterial.h"
#include "depthMaterial.h"
#include "whiteMaterial.h"
#include "material.h"

#include "scene.h"
#include "renderer.h"
#include "pointLight.h"
//imgui thirdparty
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_glfw.h"
#include "third_party/imgui/imgui_impl_opengl3.h"

#include "assimpLoader.h"

/*
 * to make main.cpp clear
 * if u need the code belong to past classes
 * check file "OldTestCode.h/.cpp"
 *
 * refer to ColorBlend, there are still some problem should be solve such as opacity order, look up OIT and Depth Peeling
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

bool setAndInitWindow(int weith = 1200,int height = 900);

//绘制命令
void render();

//准备摄像头坐标
void prepareCamera();

//状态设置初始化
void initIMGUI();
void prepareState();
void prepare();
void prepareFBO();
//绘制IMGUI
void renderIMGUI();



//声明全局变量vao以及shaderProram
//GLuint vao;
float angle = 0.0f;
std::shared_ptr < GLframework::Renderer> renderer = nullptr;
std::shared_ptr<GLframework::Scene> sceneOffScreen = nullptr;
std::shared_ptr<GLframework::Scene> sceneInScreen = nullptr;
std::shared_ptr<GLframework::Mesh> meshPointLight = nullptr;
std::shared_ptr <GLframework::AmbientLight> ambientLight = nullptr;
Camera* camera = nullptr;
CameraControl* cameracontrol = nullptr;
int width = 1200, height = 900;
glm::vec3 clearColor{};
unsigned int fbo = 0;
std::shared_ptr<GLframework::Texture> colorAttachment = nullptr;
//声明灯光
std::shared_ptr < GLframework::DirectionalLight> dirLight = nullptr;
std::shared_ptr < GLframework::SpotLight> spotLight = nullptr;
std::vector<std::shared_ptr<GLframework::PointLight>> pointLights{};
float specularIntensity = 0.8f;
int main()
{
	std::cout << "Please set the window as x * y" << std::endl;
	//std::cin >> width >> height;
	//初始化GLFW窗口
	if (!setAndInitWindow(width,height)) return -1;

	//设置opengl 视口并且清理颜色
	GL_CALL(glViewport(0, 0, width, height));
	GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

	prepareCamera();
	//注意先创建FBO，否则Attachment将为空
	prepareFBO();
	prepare();
	
	initIMGUI();
	//测试获取该显卡驱动提供的Arrribbutes数量
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
	
	while (GL_APP->update())
	{
		cameracontrol->update();
		renderer->setClearColor(clearColor);
		//pass 1: 将box渲染到colorAttachment上，新的fob上
		renderer->render(sceneOffScreen, camera, dirLight, spotLight, pointLights,ambientLight, fbo);
		//pass 2: 将colorAttachment作为纹理，绘制到整个屏幕上
		renderer->render(sceneInScreen,camera,dirLight,spotLight,pointLights,ambientLight);
		renderIMGUI();
	}

	GL_APP->destory();

	return 0;
}


void prepare()
{
	renderer = std::make_shared<GLframework::Renderer>();
	sceneInScreen = std::make_shared<GLframework::Scene>();
	sceneOffScreen = std::make_shared<GLframework::Scene>();

	//----------
	//离屏渲染
	
	auto boxMat = std::make_shared<GLframework::PhongMaterial>();
	boxMat->setPreStencilPreSettingType(GLframework::PreStencilType::Custom);
	boxMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/grass.jpg", 0);
	auto boxGeo = GLframework::Geometry::createBox(renderer->getShader(boxMat->getMaterialType()), 1.0f, 1.0f,1.0f);
	auto boxMesh = std::make_shared<GLframework::Mesh>(boxGeo, boxMat);
	sceneOffScreen->addChild(boxMesh);

	auto boxCulling = std::make_shared<GLframework::WhiteMaterial>();
	boxCulling->setPreStencilPreSettingType(GLframework::PreStencilType::Outlining);
	auto boxCullingGeo = GLframework::Geometry::createBox(renderer->getShader(boxCulling->getMaterialType()), 1.0f, 1.0f, 1.0f);
	auto BoxCullingMesh = std::make_shared<GLframework::Mesh>(boxCullingGeo, boxCulling);
	BoxCullingMesh->setScale({ 1.1f,1.1f,1.1f });
	BoxCullingMesh->setPosition(boxMesh->getPosition());
	sceneOffScreen->addChild(BoxCullingMesh);

	auto boxMat2 = std::make_shared<GLframework::PhongMaterial>();
	boxMat2->mDiffuse = std::make_shared<GLframework::Texture>("Texture/box.png", 0);
	boxMat2->mSpecularMask = std::make_shared<GLframework::Texture>("Texture/sp_mask.png", 1);
	auto Boxmesh2 = std::make_shared<GLframework::Mesh>(boxGeo, boxMat2);
	Boxmesh2->setPosition({ 3.0f, 0.0f,0.0f });
	sceneOffScreen->addChild(Boxmesh2);
	

	/*
	auto textModel = GL_APPLICATION::AssimpLoader::load("fbx/bag/backpack.obj",renderer);
	textModel->setScale(glm::vec3(1.0f));
	textModel->setPosition({ -3.0f,0.0f,0.0f });
	GLframework::Tools::setModelBlend(textModel, true, 0.2);
	sceneOffScreen->addChild(textModel);
	*/
	

	//在屏渲染
	auto met = std::make_shared<GLframework::ScreenMaterial>();
	met->mScreenTexture = colorAttachment;
	//if (met->getMaterialType() == GLframework::MaterialType::ScreenMaterial) std::cout << 1 << "\n";
	auto geo = GLframework::Geometry::createScreenPlane(renderer->getShader(met->getMaterialType()));
	auto mesh = std::make_shared<GLframework::Mesh>(geo, met);
	sceneInScreen->addChild(mesh);
	//----------


	spotLight	= std::make_shared<GLframework::SpotLight>(glm::vec3(-1.0f, 0.0f, 0.0f), 30.0f, 60.0f);
	spotLight	->	setPosition(glm::vec3(1.5f, 0.0f, 0.0f));
	spotLight	->	setColor(glm::vec3{0.0f});

	dirLight	= std::make_shared<GLframework::DirectionalLight>();
	dirLight	->	setDirection(glm::vec3(-1.0f,-1.0f,-1.0f));
	dirLight	->	setSpecularIntensity(0.5f);

	auto pointLight1 = std::make_shared<GLframework::PointLight>();
	pointLight1	->	setSpecularIntensity(0.01f);
	pointLight1	->	setK(0.017f, 0.07f, 1.0f);
	pointLight1	->	setColor(glm::vec3(0.0F));
	pointLight1	->	setPosition(glm::vec3(-1.5f, 0.0f, 0.0f));
	pointLights.push_back(std::move(pointLight1));

	auto pointLight2 = std::make_shared<GLframework::PointLight>();
	pointLight2	->	setSpecularIntensity(0.01f);
	pointLight2	->	setK(0.017f, 0.07f, 1.0f);
	pointLight2	->	setColor(glm::vec3(0.0f));
	pointLight2	->	setPosition(glm::vec3(1.5f, 0.0f, 0.0f));
	pointLights.push_back(std::move(pointLight2));

	auto pointLight3 = std::make_shared<GLframework::PointLight>();
	pointLight3	->	setSpecularIntensity(0.01f);
	pointLight3	->	setK(0.017f, 0.07f, 1.0f);
	pointLight3	->	setColor(glm::vec3(0.0f));
	pointLight3	->	setPosition(glm::vec3(0.0f, 1.5f, 0.0f));
	pointLights.push_back(std::move(pointLight3));

	auto pointLight4 = std::make_shared<GLframework::PointLight>();
	pointLight4	->	setSpecularIntensity(0.01f);
	pointLight4	->	setK(0.017f, 0.07f, 1.0f);
	pointLight4	->	setColor(glm::vec3(0.0f));
	pointLight4	->	setPosition(glm::vec3(0.0f, 0.0f, 1.5f));
	pointLights.push_back(std::move(pointLight4));
	ambientLight	 = std::make_shared<GLframework::AmbientLight>();
	ambientLight->	setColor(glm::vec3(0.2f));
	
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


void prepareFBO()
{
	//	1.生成fbo对象并绑定
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//	2.生成颜色控件，并且加入fbo
	colorAttachment = std::make_shared<GLframework::Texture>(width, height, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment->getTexture(), 0);
	//	3.生成depth Stencil附件，加入fbo
	unsigned int depthStencil;
	glGenTextures(1, &depthStencil);
	glBindTexture(GL_TEXTURE_2D, depthStencil);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencil, 0);

	//	检查当前构建的fbo是否完整
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: FrameBuff is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glDeleteFramebuffers(1, &fbo);
}

void prepareState()
{
	//深度检测设置
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void renderIMGUI()
{
	// 1. 开启当前IMGUI渲染
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 2. 决定当前的GUI上面有哪些控件，从上到下
	ImGui::Begin("Control");
	ImGui::Text("Background Color\n");
	ImGui::Button("Text Button", ImVec2(40, 20));
	ImGui::ColorEdit3("Clear Color",(float *)(&clearColor));
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	// 3. 执行UI渲染
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(GL_APP->getWindow(), &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
	
	cameracontrol = new GameCameraControl();
	cameracontrol->setCamera(camera);
}

void initIMGUI()
{
	ImGui::CreateContext();		//创建ImGui上下文
	ImGui::StyleColorsDark();	//选择一个主题

	//	设置ImGui与GLFW和OpenGL的绑定
	ImGui_ImplGlfw_InitForOpenGL(GL_APP->getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");
}



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



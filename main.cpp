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
#include "cubeMaterial.h"
#include "screenMaterial.h"
#include "gamecameracontrol.h"
#include "geometry.h"
#include "mesh/mesh.h"
#include "phongMaterial.h"
#include "phongEnvSphereMaterial.h"
#include "../mesh/instancedMesh.h"
#include "phongInstanceMaterial.h"
#include "cubeSphereMaterial.h"
#include "depthMaterial.h"
#include "whiteMaterial.h"
#include "material.h"
#include "framebuffer.h"
#include "scene.h"
#include "renderer.h"
#include "pointLight.h"
//imgui thirdparty
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_glfw.h"
#include "third_party/imgui/imgui_impl_opengl3.h"
#include "assimpLoader.h"
#include "phongEnvMaterial.h"

/*
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

bool setAndInitWindow(int width = 1200,int height = 900);

//绘制命令
void render();

//准备摄像头坐标
void prepareCamera();

//状态设置初始化
void initIMGUI();
void prepareState();
void prepare();
//行星旋转函数
void rotatePlant();

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
std::shared_ptr<GLframework::Framebuffer> framebuffer = nullptr;
Camera* camera = nullptr;
CameraControl* cameracontrol = nullptr;

//----skyBox----
std::shared_ptr<GLframework::Mesh> skyBoxMesh = nullptr;
void prepareSkyBox();
//---------------


#pragma region 太阳系模拟
/*
auto roundForAll = std::make_shared<GLframework::Object>();
auto roundForEarth = std::make_shared<GLframework::Object>();
auto roundForVenus = std::make_shared<GLframework::Object>();
auto roundForUranus = std::make_shared<GLframework::Object>();
auto roundForSaturn = std::make_shared<GLframework::Object>();
auto roundForNeptune = std::make_shared<GLframework::Object>();
auto roundForJupiter = std::make_shared<GLframework::Object>();
auto roundForMars = std::make_shared<GLframework::Object>();
auto roundForMercury = std::make_shared<GLframework::Object>();
auto roundForMoon = std::make_shared<GLframework::Object>();
*/
#pragma endregion
int width = 1200, height = 900;
glm::vec3 clearColor{};


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
		rotatePlant();
		//pass 1: 将box渲染到colorAttachment上，新的fob上
		renderer->render(sceneOffScreen, camera, dirLight, spotLight, pointLights,ambientLight, framebuffer->getFBO());
		//pass 2: 将colorAttachment作为纹理，绘制到整个屏幕上
		renderer->render(sceneInScreen,camera,dirLight,spotLight,pointLights,ambientLight);
		renderIMGUI();
	}

	GL_APP->destory();

	return 0;
}

void prepareSkyBox()
{
	std::vector<std::string> paths = {
		"Texture/skybox/right.jpg",
		"Texture/skybox/left.jpg",
		"Texture/skybox/top.jpg",
		"Texture/skybox/bottom.jpg",
		"Texture/skybox/back.jpg",
		"Texture/skybox/front.jpg",
	};

	//boxMat->setPreStencilPreSettingType(GLframework::PreStencilType::Normal);
	auto skyBoxMat = std::make_shared<GLframework::CubeSphereMaterial>();
	skyBoxMat-> mDiffuse = std::make_shared<GLframework::Texture>("Texture/bk.jpg", 0);
	auto boxGeo = GLframework::Geometry::createBox(renderer->getShader(GLframework::MaterialType::CubeSphereMaterial), 3.0f, 3.0f, 3.0f);
	skyBoxMesh = std::make_shared<GLframework::Mesh>(boxGeo, skyBoxMat);
	sceneOffScreen->addChild(skyBoxMesh);
}



void prepare()
{
	renderer = std::make_shared<GLframework::Renderer>();
	sceneInScreen = std::make_shared<GLframework::Scene>();
	sceneOffScreen = std::make_shared<GLframework::Scene>();
	framebuffer = std::make_shared<GLframework::Framebuffer>(width, height);
	//----------
	//离屏渲染
#pragma region 太阳系模拟
	/*
	float distanceEarth = 10.0f;
	float sizeOfEarth = 1.0f;
	//月球
	auto moonMat = std::make_shared<GLframework::PhongMaterial>();
	moonMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/moon1k.jpg", 0);
	auto sphereGeo = GLframework::Geometry::createSphere(renderer->getShader(moonMat->getMaterialType()),0.3f,1000,1000);
	auto moonSphere = std::make_shared<GLframework::Mesh>(sphereGeo, moonMat);

	auto sunMat = std::make_shared<GLframework::PhongMaterial>();
	sunMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_sun.jpg", 0);
	auto sunSphere = std::make_shared<GLframework::Mesh>(sphereGeo, sunMat);
	//sunSphere->setPosition({ 0.0f,0.0f,0.0f });
	sunSphere->setScale(glm::vec3(10.00f * sizeOfEarth));
	
	//金星
	auto venusSphereMat = std::make_shared<GLframework::PhongMaterial>();
	venusSphereMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_venus_surface.jpg",0);
	auto venusSphere = std::make_shared<GLframework::Mesh>(sphereGeo, venusSphereMat);

	venusSphere->setScale(glm::vec3(0.94f * sizeOfEarth));
	venusSphere->setPosition({0.72f*distanceEarth,0.0f,0.0f});
	
	roundForVenus->addChild(venusSphere);
	sceneOffScreen->addChild(roundForVenus);
	//天王星
	auto uranusMat = std::make_shared<GLframework::PhongMaterial>();
	uranusMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_uranus.jpg", 0);
	auto uranusSphere = std::make_shared<GLframework::Mesh>(sphereGeo, uranusMat);

	uranusSphere->setScale(glm::vec3(4.00f * sizeOfEarth));
	uranusSphere->setPosition({ 19.19f*distanceEarth,0.0f,0.0f });
	
	roundForUranus->addChild(uranusSphere);
	sceneOffScreen->addChild(roundForUranus);
	//土星
	auto saturnMat = std::make_shared<GLframework::PhongMaterial>();
	saturnMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_saturn.jpg", 0);
	auto saturnSphere = std::make_shared<GLframework::Mesh>(sphereGeo, saturnMat);
	roundForSaturn->addChild(saturnSphere);
	saturnSphere->setScale(glm::vec3(9.44f * sizeOfEarth));
	saturnSphere->setPosition({ 9.53f*distanceEarth,0.0f,0.0f });
	
	sceneOffScreen->addChild(roundForSaturn);
	//海王星
	auto neptuneMat = std::make_shared<GLframework::PhongMaterial>();
	neptuneMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_neptune.jpg", 0);
	auto neptuneSphere = std::make_shared<GLframework::Mesh>(sphereGeo, neptuneMat);
	roundForNeptune->addChild(neptuneSphere);
	neptuneSphere->setScale(glm::vec3(3.88f * sizeOfEarth));
	neptuneSphere->setPosition({ 30.06f*distanceEarth,0.0f,0.0f });
	
	sceneOffScreen->addChild(roundForNeptune);
	//木星
	auto jupiterMat = std::make_shared<GLframework::PhongMaterial>();
	jupiterMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_jupiter.jpg", 0);
	auto jupiterSphere = std::make_shared<GLframework::Mesh>(sphereGeo, jupiterMat);
	roundForJupiter->addChild(jupiterSphere);

	jupiterSphere->setScale(glm::vec3(11.20f * sizeOfEarth));
	jupiterSphere->setPosition({ 5.20f*distanceEarth,0.0f,0.0f });
	
	sceneOffScreen->addChild(roundForJupiter);
	//火星
	auto marsMat = std::make_shared<GLframework::PhongMaterial>();
	marsMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_mars.jpg", 0);
	auto marsSphere = std::make_shared<GLframework::Mesh>(sphereGeo, marsMat);
	roundForMars->addChild(marsSphere);

	marsSphere->setScale(glm::vec3(sizeOfEarth));
	marsSphere->setPosition({ 1.52f*distanceEarth,0.0f,0.0f });
	
	sceneOffScreen->addChild(roundForMars);

	auto earthMat = std::make_shared<GLframework::PhongMaterial>();
	earthMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_earth_daymap.jpg", 0);
	auto earthSphere = std::make_shared<GLframework::Mesh>(sphereGeo, earthMat);
	
	roundForEarth->addChild(earthSphere);
	earthSphere->setScale(glm::vec3(sizeOfEarth));
	earthSphere->setPosition({ distanceEarth*1.0f,0.0f,0.0f });
	
	//moonSphere->setPosition({ earthSphere->getPosition().x + 0.5f, earthSphere->getPosition().y, earthSphere->getPosition().z });
	std::cout << earthSphere->getPosition().x << " " << earthSphere->getPosition().y << " " << earthSphere->getPosition().z<<"\n";
	roundForEarth->addChild(roundForMoon);
	roundForMoon->setPosition({ distanceEarth, 0.0f, 0.0f });
	roundForMoon->addChild(moonSphere);
	moonSphere->setScale(glm::vec3(0.27f * sizeOfEarth));
	moonSphere->setPosition({ 1.1f,0.0f,0.0f });
	std::cout << moonSphere->getPosition().x << " " << moonSphere->getPosition().y << " " << moonSphere->getPosition().z << "\n";

	
	auto earth2Mat = std::make_shared<GLframework::PhongMaterial>();
	earth2Mat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_earth_nightmap.jpg", 0);
	auto earth2Sphere = std::make_shared<GLframework::Mesh>(sphereGeo, earth2Mat);
	roundForEarth->addChild(earth2Sphere);
	earth2Sphere->setScale(glm::vec3(sizeOfEarth));
	earth2Sphere->setPosition({ distanceEarth*1.0f + 0.001f,0.0f,0.0f });
	//sceneOffScreen->addChild(roundForMoon);
	sceneOffScreen->addChild(roundForEarth);
	

	//水星
	auto mercuryMat = std::make_shared<GLframework::PhongMaterial>();
	mercuryMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_mercury.jpg", 0);
	auto mercurySphere = std::make_shared<GLframework::Mesh>(sphereGeo, mercuryMat);
	roundForMercury->addChild(mercurySphere);
	mercurySphere->setScale(glm::vec3(0.38f * sizeOfEarth));
	mercurySphere->setPosition({ 0.38f*distanceEarth,0.0f,0.0f });
	
	sceneOffScreen->addChild(roundForMercury);
	sceneOffScreen->addChild(sunSphere);
	*/
#pragma endregion
	prepareSkyBox();
	auto skyBoxMat = std::static_pointer_cast<GLframework::PhongEnvSphereMaterial>(skyBoxMesh->getMaterial())->mDiffuse;
	auto earthMat = std::make_shared<GLframework::PhongEnvSphereMaterial>();
	earthMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_earth_daymap.jpg", 0);

	earthMat->mEnv = skyBoxMat;
	auto earthGeo = GLframework::Geometry::createSphere(renderer->getShader(GLframework::MaterialType::PhongEnvSphereMaterial), 1.0f);
	auto earthMash = std::make_shared<GLframework::Mesh>(earthGeo, earthMat);
	sceneOffScreen->addChild(earthMash);
	
	auto earthN = std::make_shared<GLframework::PhongInstanceMaterial>();
	earthN->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_earth_daymap.jpg", 0);
	auto earthGeoN = GLframework::Geometry::createSphere(renderer->getShader(earthN->getMaterialType()), 1.0f);
	auto earthNMesh = std::make_shared<GLframework::InstancedMesh>(earthGeoN, earthN,2);
	glm::mat4 transform0 = glm::mat4(1.0f);
	glm::mat4 transform1 = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
	earthNMesh->mInstanceMatrices[0] = transform0;
	earthNMesh->mInstanceMatrices[1] = transform1;
	earthNMesh->updateMatrices();
	earthNMesh->setPosition({ 2.0f,0.0f,0.0f });
	sceneOffScreen->addChild(earthNMesh);
	/*
	auto boxCulling = std::make_shared<GLframework::WhiteMaterial>();
	boxCulling->setPreStencilPreSettingType(GLframework::PreStencilType::Outlining);
	auto boxCullingGeo = GLframework::Geometry::createBox(renderer->getShader(boxCulling->getMaterialType()), 1.0f, 1.0f, 1.0f);
	auto BoxCullingMesh = std::make_shared<GLframework::Mesh>(boxCullingGeo, boxCulling);
	BoxCullingMesh->setScale({ 1.07f,1.07f,1.07f });
	BoxCullingMesh->setPosition(boxMesh->getPosition());
	sceneOffScreen->addChild(BoxCullingMesh);
	
	auto boxMat2 = std::make_shared<GLframework::PhongMaterial>();
	boxMat2->mDiffuse = std::make_shared<GLframework::Texture>("Texture/box.png", 0);
	boxMat2->mSpecularMask = std::make_shared<GLframework::Texture>("Texture/sp_mask.png", 1);
	Boxmesh2->setPosition({ 3.0f, 0.0f,0.0f });
	sceneOffScreen->addChild(Boxmesh2);
	*/

	/*
	auto textModel = GL_APPLICATION::AssimpLoader::load("fbx/bag/backpack.obj",renderer);
	textModel->setScale(glm::vec3(1.0f));
	//textModel->setPosition({ -3.0f,0.0f,0.0f });
	GLframework::Tools::setModelBlend(textModel, true, 0.5);
	sceneOffScreen->addChild(textModel);
	*/

	//在屏渲染
	auto met = std::make_shared<GLframework::ScreenMaterial>();
	met->mScreenTexture = framebuffer->getColorAttachment();
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
	dirLight	->	setColor({ 0.8f,0.8f,0.9f });
	dirLight	->	setSpecularIntensity(0.0f);

	auto pointLight1 = std::make_shared<GLframework::PointLight>();
	pointLight1	->	setSpecularIntensity(0.01f);
	pointLight1	->	setK(0.017f, 0.07f, 1.0f);
	pointLight1	->	setColor(glm::vec3(0.0F));
	//pointLight1	->	setPosition(glm::vec3(-1.5f, 0.0f, 0.0f));
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
	ambientLight->	setColor(glm::vec3(0.0f));

}


bool setAndInitWindow(int width, int height)
{
	if (!GL_APP->init(width,height)) return false;
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
	
	cameracontrol = new TrackBallCameraControl();
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

void rotatePlant()
{


#pragma region 太阳系模拟
	/*
	float speed = 0.01f;
	roundForVenus->rotateY(1.6022f * speed);
	for(auto& t:roundForVenus->getChildren())
	{
		t->rotateY(speed*10);
	}
	roundForUranus->rotateY(0.0117f * speed);
	for (auto& t : roundForUranus->getChildren())
	{
		t->rotateY(speed*10);
	}
	roundForEarth->rotateY(0.9863f * speed);
	for (auto& t : roundForEarth->getChildren())
	{
		if(t->getType()==GLframework::ObjectType::Mesh)
			t->rotateY(speed*10);
	}
	roundForMoon->rotateY(5.0f * speed);

	roundForJupiter->rotateY(0.08316f * speed);
	for (auto& t : roundForJupiter->getChildren())
	{
		t->rotateY(speed*10);
	}
	roundForMars->rotateY(0.5240f * speed);
	for (auto& t : roundForMars->getChildren())
	{
		t->rotateY(speed*10);
	}
	roundForSaturn->rotateY(0.0335f * speed);
	for (auto& t : roundForSaturn->getChildren())
	{
		t->rotateY(speed*10);
	}
	roundForMercury->rotateY(4.0927f * speed);
	for (auto& t : roundForMercury->getChildren())
	{
		t->rotateY(speed*10);
	}
	roundForNeptune->rotateY(0.0059f * speed);
	for (auto& t : roundForNeptune->getChildren())
	{
		t->rotateY(speed*10);
	}
	*/
#pragma endregion

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



#include "core.h"
#include <iostream>
#include <windows.h>
#include <memory>
#include <vector>
#include <typeinfo>
#include "GL_ERROR_FIND.h"
#include "Application.h"
#include "tools/tools.h"
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
#include "materials/phongNormalMaterial/phongNormalMaterial.h"
#include "cubeSphereMaterial.h"
#include "materials/grassInstanceMaterial/grassInstanceMaterial.h"
#include "materials/phongParallaxMaterial/phongParallaxMaterial.h"
#include "materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h"
#include "materials/phongPointShadowMaterial/phongPointShadowMaterial.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "materials/phongShadowMaterial/phongShadowMaterial.h"
#include "materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h"

#include "depthMaterial.h"
#include "whiteMaterial.h"
#include "material.h"
#include "framebuffer.h"
#include "scene.h"
#include "renderer.h"
#include "pointLight.h"
//imgui thirdparty
#include "assimpInstanceLoader.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_glfw.h"
#include "third_party/imgui/imgui_impl_opengl3.h"
#include "assimpLoader.h"
#include "phongEnvMaterial.h"
#include "materials/phongPointShadowMaterial/phongPointShadowMaterial.h"
int GLframework::PointLightShadow::MAX_POINT_LIGHTS = 2;
/*
 * refer to ColorBlend, there are still some problem should be solve such as opacity order, look up OIT and Depth Peeling
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

bool setAndInitWindow(int width = 1200,int height = 900);

//
void render();

//׼������ͷ����
void prepareCamera();

//״̬���ó�ʼ��
void initIMGUI();
void prepareState();
void prepare();
//������ת����
void rotatePlant();

//����IMGUI
void renderIMGUI();

//grass texture attribute
int rNum = 30;
int cNum = 30;
float scale = 0.0f;
float brigtnesee = 1.0f;

//parallax texture attribute
std::shared_ptr<GLframework::PhongParallaxMaterial> parallaxMat = nullptr;


//GLuint vao;
float angle = 0.0f;
std::shared_ptr < GLframework::Renderer> renderer = nullptr;
std::shared_ptr<GLframework::Scene> sceneOffScreen = nullptr;
std::shared_ptr<GLframework::Scene> sceneInScreen = nullptr;
std::shared_ptr<GLframework::Mesh> meshPointLight = nullptr;
std::shared_ptr <GLframework::AmbientLight> ambientLight = nullptr;
std::shared_ptr<GLframework::Framebuffer> framebufferMultisample = nullptr;
std::shared_ptr<GLframework::Framebuffer> framebufferResolve = nullptr;
std::shared_ptr<GLframework::GrassInstanceMaterial> grassMaterial = nullptr;
Camera* camera = nullptr;
CameraControl* cameracontrol = nullptr;

//----skyBox----
std::shared_ptr<GLframework::Mesh> skyBoxMesh = nullptr;
std::string TexturePath{ "Texture/bk.jpg" };
void prepareSkyBox();
//---------------


#pragma region solorsystem

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
float speed = 0.01f;
#pragma endregion
int width = 1920, height = 1080;
glm::vec3 clearColor{};



//�����ƹ�
std::shared_ptr < GLframework::DirectionalLight> dirLight = nullptr;
std::shared_ptr < GLframework::SpotLight> spotLight = nullptr;
std::vector<std::shared_ptr<GLframework::PointLight>> pointLights{};
float specularIntensity = 0.8f;

std::shared_ptr<GLframework::PhongCSMShadowMaterial> mat2{nullptr};
//--------text--------
std::shared_ptr<GLframework::Mesh> movePlane = nullptr;
void moveit()
{
	movePlane->setPosition({ 0.0f,(glm::sin(glfwGetTime()) + 1) * 5,0.0f });
}
std::shared_ptr<GLframework::Mesh> textD = nullptr;
void rotateLight();
float m_time = 0.0f;
std::shared_ptr<GLframework::ScreenMaterial> ScreenMat = nullptr;
//--------------------
int main()
{

	std::cout << "Please set the window as x * y" << std::endl;
	//std::cin >> width >> height;
	//��ʼ��GLFW����
	if (!setAndInitWindow(width,height)) return -1;

	//����opengl �ӿڲ���������ɫ
	GL_CALL(glViewport(0, 0, width, height));
	GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	
	prepareCamera();
	prepare();
	
	initIMGUI();
	//���Ի�ȡ���Կ������ṩ��Arrribbutes����
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
	
	while (GL_APP->update())
	{
		cameracontrol->update();
		renderer->setClearColor(clearColor);
		rotatePlant();
		rotateLight();
		//moveit();
		//pass 1: ��box��Ⱦ��colorAttachment�ϣ��µ�fob��
		renderer->render(sceneOffScreen, camera, dirLight, spotLight, pointLights,ambientLight, framebufferMultisample->getFBO());
		renderer->msaaResolve(framebufferMultisample, framebufferResolve);
		//pass 2: ��colorAttachment��Ϊ��������Ƶ�������Ļ��
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
	skyBoxMat-> mDiffuse = std::make_shared<GLframework::Texture>(TexturePath, 0);
	auto boxGeo = GLframework::Geometry::createBox(renderer->getShader(GLframework::MaterialType::CubeSphereMaterial), 3.0f, 3.0f, 3.0f);
	skyBoxMesh = std::make_shared<GLframework::Mesh>(boxGeo, skyBoxMat);
	//sceneOffScreen->addChild(skyBoxMesh);
}


void prepare()
{
	//glEnable(GL_FRAMEBUFFER_SRGB);
	renderer = std::make_shared<GLframework::Renderer>();
	sceneInScreen = std::make_shared<GLframework::Scene>();
	sceneOffScreen = std::make_shared<GLframework::Scene>();
	framebufferMultisample = GLframework::Framebuffer::createMultiSampleFbo(width, height, 4);
	//framebufferResolve = std::make_shared<GLframework::Framebuffer>(width, height);
	//hdr test
	framebufferResolve = GLframework::Framebuffer::createHDRFbo(width, height);
	//

	GLframework::PointLightShadow::initializeSharedDepthTexture(1024, 1024, 2); // 假设最多支持1个点光源


	//----------
#pragma region solorsystem
	/*
	
	float distanceEarth = 10.0f;
	float sizeOfEarth = 1.0f;
	//����
	auto moonMat = std::make_shared<GLframework::PhongMaterial>();
	moonMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/moon1k.jpg", 0);
	auto sphereGeo = GLframework::Geometry::createSphere(renderer->getShader(moonMat->getMaterialType()),0.3f,1000,1000);
	auto moonSphere = std::make_shared<GLframework::Mesh>(sphereGeo, moonMat);

	auto sunMat = std::make_shared<GLframework::PhongMaterial>();
	sunMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_sun.jpg", 0);
	auto sunSphere = std::make_shared<GLframework::Mesh>(sphereGeo, sunMat);
	//sunSphere->setPosition({ 0.0f,0.0f,0.0f });
	sunSphere->setScale(glm::vec3(10.00f * sizeOfEarth));
	
	//����
	auto venusSphereMat = std::make_shared<GLframework::PhongMaterial>();
	venusSphereMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_venus_surface.jpg",0);
	auto venusSphere = std::make_shared<GLframework::Mesh>(sphereGeo, venusSphereMat);

	venusSphere->setScale(glm::vec3(0.94f * sizeOfEarth));
	venusSphere->setPosition({0.72f*distanceEarth,0.0f,0.0f});
	
	roundForVenus->addChild(venusSphere);
	sceneOffScreen->addChild(roundForVenus);
	//������
	auto uranusMat = std::make_shared<GLframework::PhongMaterial>();
	uranusMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_uranus.jpg", 0);
	auto uranusSphere = std::make_shared<GLframework::Mesh>(sphereGeo, uranusMat);

	uranusSphere->setScale(glm::vec3(4.00f * sizeOfEarth));
	uranusSphere->setPosition({ 19.19f*distanceEarth,0.0f,0.0f });
	
	roundForUranus->addChild(uranusSphere);
	sceneOffScreen->addChild(roundForUranus);
	//����
	auto saturnMat = std::make_shared<GLframework::PhongMaterial>();
	saturnMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_saturn.jpg", 0);
	auto saturnSphere = std::make_shared<GLframework::Mesh>(sphereGeo, saturnMat);
	roundForSaturn->addChild(saturnSphere);
	saturnSphere->setScale(glm::vec3(9.44f * sizeOfEarth));
	saturnSphere->setPosition({ 9.53f*distanceEarth,0.0f,0.0f });
	
	sceneOffScreen->addChild(roundForSaturn);
	//������
	auto neptuneMat = std::make_shared<GLframework::PhongMaterial>();
	neptuneMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_neptune.jpg", 0);
	auto neptuneSphere = std::make_shared<GLframework::Mesh>(sphereGeo, neptuneMat);
	roundForNeptune->addChild(neptuneSphere);
	neptuneSphere->setScale(glm::vec3(3.88f * sizeOfEarth));
	neptuneSphere->setPosition({ 30.06f*distanceEarth,0.0f,0.0f });
	
	sceneOffScreen->addChild(roundForNeptune);
	//ľ��
	auto jupiterMat = std::make_shared<GLframework::PhongMaterial>();
	jupiterMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_jupiter.jpg", 0);
	auto jupiterSphere = std::make_shared<GLframework::Mesh>(sphereGeo, jupiterMat);
	roundForJupiter->addChild(jupiterSphere);

	jupiterSphere->setScale(glm::vec3(11.20f * sizeOfEarth));
	jupiterSphere->setPosition({ 5.20f*distanceEarth,0.0f,0.0f });
	
	sceneOffScreen->addChild(roundForJupiter);
	//����
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
	

	//ˮ��
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
	//离屏渲染
	prepareSkyBox();
#pragma region grassplane
	/*
	grassMaterial = std::make_shared<GLframework::GrassInstanceMaterial>();
	grassMaterial->mDiffuse = std::make_shared<GLframework::Texture>("fbx/textures/GRASS.PNG", 0);
	grassMaterial->mOpacityMask = std::make_shared<GLframework::Texture>("fbx/textures/grassMask.png", 2);

	grassMaterial->mCloudMask = std::make_shared<GLframework::Texture>("Texture/CLOUD.PNG", 3);

	auto house = GL_APPLICATION::AssimpLoader::load("fbx/house.fbx",renderer);
	house->setScale(glm::vec3(0.5f));
	house->setPosition(glm::vec3(rNum * 0.2f / 2.0f, 0.4, cNum * 0.2f / 2.0f));
	sceneOffScreen->addChild(house);
	auto grassModel = GL_APPLICATION::AssimpInstanceLoader::load("fbx/grassNew.obj",renderer, rNum * cNum);
	glm::mat4 translate;
	glm::mat4 rotate;
	glm::mat4 transform;

	srand(glfwGetTime());
	for(int i = 0;i<rNum;++i)
	{
		for(int j = 0;j<cNum;++j)
		{
			translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.2*i, 0.0f, 0.2*j));
			rotate = glm::rotate(glm::radians(static_cast<float>(rand() % 90)), glm::vec3(0.0f, 1.0f, 0.0f));
			transform = translate * rotate;
			GL_APPLICATION::AssimpInstanceLoader::setInstanceMatrix(grassModel,j + i * cNum,transform);
		}
	}
	GL_APPLICATION::AssimpInstanceLoader::updateInstanceMatrix(grassModel);
	GL_APPLICATION::AssimpInstanceLoader::setInstanceMaterial(grassModel, grassMaterial);
	sceneOffScreen->addChild(grassModel);
	*/


	/*
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
*/
#pragma endregion
	/*
	mat2 = std::make_shared<GLframework::PhongCSMShadowMaterial>();
	mat2->mDiffuse = std::make_shared < GLframework::Texture >("Texture/box.png", 0, GL_SRGB_ALPHA);
	//mat2->mDiffuse = renderer->mShadowFBO->getDepthAttachment();
	auto boxGeo = GLframework::Geometry::createPlane(renderer->getShader(mat2->getMaterialType()), 1.0, 1.0);
	movePlane = std::make_shared<GLframework::Mesh>(boxGeo, mat2);
	movePlane->setPosition({ 0.0f,0.0f,0.0f });
	movePlane->rotateX(-90);
	sceneOffScreen->addChild(movePlane);

	auto parallaxMat = std::make_shared<GLframework::PhongCSMShadowMaterial>();
	parallaxMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/parallax/bricks.jpg",0,GL_SRGB_ALPHA);
	//parallaxMat->mNormal = std::make_shared<GLframework::Texture>("Texture/parallax/bricks_normal.jpg", 2);
	//parallaxMat->mParallaxMap = std::make_shared<GLframework::Texture>("Texture/parallax/disp.jpg", 3);
	auto planeGe = GLframework::Geometry::createPlane(renderer->getShader(parallaxMat->getMaterialType()),10,10);
	auto planeMesh = std::make_shared<GLframework::Mesh>(planeGe, parallaxMat);
	planeMesh->rotateX(-90);
	sceneOffScreen->addChild(planeMesh);
	*/

	//----------------text

	
	//普通物品使用GL_SRGB_ALPHA
	// 添加地面
	auto groundMat = std::make_shared<GLframework::PhongPointShadowMaterial>();
	groundMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/land.jpg", 0, GL_SRGB_ALPHA);
	auto groundMatTex = std::make_shared<GLframework::PhongMaterial>();

	groundMatTex->mDiffuse = std::make_shared<GLframework::Texture>("Texture/land.jpg", 0, GL_SRGB_ALPHA);

	auto groundGeo = GLframework::Geometry::createPlane(renderer->getShader(groundMat->getMaterialType()), 10.0, 10.0);
	auto groundMeshA = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
	auto groundMeshB = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
	auto groundMeshC = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
	textD = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
	auto groundMeshE = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
	auto groundMeshF = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
	
	groundMeshA->setPosition({ 0.0f,-5.0f,0.0f });
	groundMeshA->rotateX(-90);

	groundMeshB->setPosition({ 0.0f,5.0f,0.0f });
	groundMeshB->rotateX(90);

	groundMeshC->setPosition({ 0.0f,0.0f,-5.0f });
	

	textD->setPosition({ 0.0f,0.0f,5.0f });
	textD->rotateX(180);

	
	groundMeshE->setPosition({ -5.0f,0.0f,0.0f });
	groundMeshE->rotateY(90);

	groundMeshF->setPosition({ 5.0f,0.0f,0.0f });
	groundMeshF->rotateY(-90);

	sceneOffScreen->addChild(groundMeshA);
	sceneOffScreen->addChild(groundMeshB);
	sceneOffScreen->addChild(groundMeshC);
	sceneOffScreen->addChild(textD);
	sceneOffScreen->addChild(groundMeshE);
	sceneOffScreen->addChild(groundMeshF);

	auto boxMat = std::make_shared<GLframework::PhongPointShadowMaterial>();
	boxMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/box.png", 0, GL_SRGB_ALPHA);
	auto boxGeo = GLframework::Geometry::createBox(renderer->getShader(boxMat->getMaterialType()), 3, 1, 1);
	auto boxMeshA = std::make_shared<GLframework::Mesh>(boxGeo, boxMat);
	sceneOffScreen->addChild(boxMeshA);


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
	ScreenMat = std::make_shared<GLframework::ScreenMaterial>();
	ScreenMat->mScreenTexture = framebufferResolve->getColorAttachment();
	//met->mScreenTexture = renderer->mShadowFBO->getDepthAttachment();
	//met->mDepthStencilTexture = framebuffer->getDepthStencilAttachment();
	auto geo = GLframework::Geometry::createScreenPlane(renderer->getShader(ScreenMat->getMaterialType()));
	auto mesh = std::make_shared<GLframework::Mesh>(geo, ScreenMat);
	sceneInScreen->addChild(mesh);
	//----------

	spotLight	= std::make_shared<GLframework::SpotLight>( 30.0f, 60.0f);
	spotLight	->	setPosition(glm::vec3(1.5f, 0.0f, 0.0f));
	spotLight	->	setColor(glm::vec3{0.0f});

	dirLight	= std::make_shared<GLframework::DirectionalLight>();
	dirLight->setPosition(glm::vec3(0.0f, 11.0f, 0.0f));
	dirLight->rotateX(-45.0f);
	dirLight->rotateY(45.0f);

	dirLight	->	setColor({ 0.0f,0.0f,0.0f });
	dirLight	->	setSpecularIntensity(0.5f);
	/*
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
	*/
	ambientLight	 = std::make_shared<GLframework::AmbientLight>();
	ambientLight->	setColor(glm::vec3(0.1f));
	
	for (int i = 0; i < 2; ++i) {
		auto pointLight = std::make_shared<GLframework::PointLight>();
		pointLight->setSpecularIntensity(0.9f);
		pointLight->setK(0.0f, 0.0f, 1.0f); // 减少衰减
		

		// 设置点光源位置
		float angle = i * (2 * glm::pi<float>() / 4);
		float radius = 5.0f;
		float x = cos(angle) * radius;
		float z = sin(angle) * radius;
		if(i==0)
		{
			pointLight->setPosition(glm::vec3(3.0, 3.0f, -1.0));
			pointLight->setColor(glm::vec3(0.8f, 0.8f, 0.9f));
		
		}
		if (i == 1)
		{
			pointLight->setPosition(glm::vec3(-3.0, 3.0f, -1.0));
			pointLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
		}
		pointLights.push_back(std::move(pointLight));
	}
	GLframework::PointLightShadow::setMAX_POINT_LIGHT(pointLights.size());

	/*
	auto textplan = std::make_shared<GLframework::PhongMaterial>();
	dirLight->getShadow()->mRenderTarget->getDepthAttachment()->setUnit(2);
	textplan->mDiffuse = dirLight->getShadow()->mRenderTarget->getDepthAttachment();

	auto textGeo = GLframework::Geometry::createPlane(renderer->getShader(textplan->getMaterialType()), 2, 2);
	auto meshit = std::make_shared<GLframework::Mesh>(textGeo, textplan);
	meshit->setPosition({ 3.0f,1.0f,0.0f });
	sceneOffScreen->addChild(meshit);
	*/
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
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void renderIMGUI()
{
	// 1. Initialize ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 2. setting ImGui UI
	
	ImGui::Begin("controller");
	
	auto pos = dirLight->getPosition();
	if(ImGui::SliderFloat("light.x", &pos.x, 0.0f, 50.f, "%.2f"))
	{
		dirLight->setPosition(pos);
	}
	float rotate = textD->getAngleX();
	if (ImGui::SliderFloat("Text Rotate:", &rotate, -360, 360))
	{
		textD->setAngleX(rotate);
	}
	ImGui::SliderFloat("tightness", &dirLight->getShadow()->mDiskTightness, 0.0f, 1.0f,"%.3f");
	ImGui::SliderFloat("pcfRadius", &dirLight->getShadow()->mPcfRadius, 0.0f, 10.0f, "%.3f");
	
	if(ImGui::SliderAngle("angle", &m_time))
	{
		double r = 3;
		double x = r * glm::sin(m_time);
		double y = r * glm::cos(m_time);
		pointLights[0]->setPosition({ x,3,y });
	}
	ImGui::SliderFloat("Exposure", &ScreenMat->mExposure, 0.0f, 1.0f);
	/*
	int width = dirLight->getShadow()->mRenderTarget->getWidth();
	int height = dirLight->getShadow()->mRenderTarget->getHeight();
	if(ImGui::SliderInt("FBO width:" , &width,1,4096)|| ImGui::SliderInt("FBO height:", &height, 1, 4096))
	{
		dirLight->getShadow()->setRenderTargetSize(width, height);
	}
	ImGui::SliderFloat("lightSize", &dirLight->getShadow()->mLightSize, 0.0f, 10.0f);
	float h = movePlane->getPosition().y;
	if(ImGui::SliderFloat("plane H",&h,1,10))
	{
		movePlane->setPosition({ 0.0f,h,0.0f });
	}
	*/
	//ImGui::SliderFloat("Parallax Scale", &parallaxMat->mHeightScale, 0.0f, 1.0f);
	//ImGui::SliderInt("layerNumber",&parallaxMat->mLayerNum, 1, 10000);

	/*
	ImGui::Text("Light");
	ImGui::InputFloat("intencity", dirLight->Control_Intensity(), 0.0f, 100.0f);

	ImGui::Text("MIX");
	ImGui::SliderFloat("CloudLerp", grassMaterial->Control_CloudLerp(), 0.0f, 1.0f);
	ImGui::Text("GrassColor");
	ImGui::SliderFloat("UVScale",grassMaterial->Control_UVScale(),0.0f,100.0f);
	
	ImGui::InputFloat("Brightness", grassMaterial->Control_Brightness());

	ImGui::Text("Wind");
	ImGui::SliderFloat("WindScale", grassMaterial->Control_WindScale(),-0.12f,0.12f);
	ImGui::InputFloat("PhaseScale", grassMaterial->Control_PhaseScale());
	ImGui::ColorEdit3("WindDirection",(float*)(grassMaterial->Control_WindDirection()));
	ImGui::Text("Cloud");
	ImGui::SliderFloat("CloudScale", grassMaterial->Control_CloudUVScale(), 0.0f, 100.0f);
	ImGui::SliderFloat("CloudSpeed", grassMaterial->Control_CloudSpeed(), 0.0f, 3.0f);
	ImGui::ColorEdit3("CloudWhiteColor", (float*)(grassMaterial->Control_CloudWhiteColor()));
	ImGui::ColorEdit3("CloudBlackColor", (float*)(grassMaterial->Control_CloudBlackColor()));
	

	ImGui::ColorEdit3("Clear Color",(float *)(&clearColor));
	*/
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	
	ImGui::End();

	// 3. ִ��UI��Ⱦ
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
	ImGui::CreateContext();		//����ImGui������
	ImGui::StyleColorsDark();	//ѡ��һ������

	//	imgui版本设置
	ImGui_ImplGlfw_InitForOpenGL(GL_APP->getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void rotatePlant()
{

#pragma region solorsystem
	
	/*
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


#pragma region 回调函数
//�����֣��������ص�����
void OnScroll(double offset)
{
	cameracontrol->onScroll(offset);
}

void OnResize(int width, int height)
{
	GL_CALL(glViewport(0, 0, width, height));
#ifdef _DEBUG
	std::cout << "OnResize" << std::endl;
#endif

}

void OnKeyboardCallback(int key, int action, int mods)
{
	GL_CALL(cameracontrol->onKey(key, action, mods));
#ifdef _DEBUG
	std::cout << "OnKeyboardCallback Pressed: " << key << " " << action << " " << mods << std::endl;
#endif
}

void OnMouseCallback(int button, int action, int mods)
{
	double x, y;
	GL_APP->getCursorPosition(&x, &y);
#ifdef _DEBUG
	std::cout << "OnMouseCallback : " << button << " " << action << " " << mods << std::endl;
#endif
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
#ifdef _DEBUG
	if (action == GLFW_PRESS)
		std::cout << "press the bottom" << std::endl;
	else if (action == GLFW_RELEASE)
		std::cout << "release the bottom" << std::endl;
	if (mods == GLFW_MOD_CONTROL)
		std::cout << "press the ctrl and key" << std::endl;
	std::cout << "Pressed: " << key << std::endl;
	std::cout << "Action: " << action << std::endl;
	std::cout << "Mods: " << mods << std::endl;
#endif

}

void OnCursor(double xpos, double ypos)
{
	//std::cout << "(" << xpos << ", " << ypos << ")" << std::endl;
	cameracontrol->onCursor(xpos, ypos);
}
#pragma endregion


void rotateLight()
{
	/*
	double r = 3;
	double Time = glfwGetTime();
	double x = r * glm::sin(Time);
	double y = r * glm::cos(Time);
	pointLights[0]->setPosition({ x,3,y });
	*/
}

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
#include "whiteMaterial.h"
#include "material.h"
#include "renderer.h"
#include "pointLight.h"
//imgui thirdparty
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_glfw.h"
#include "third_party/imgui/imgui_impl_opengl3.h"

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
void initIMGUI();
void prepareState();
void prepare();

//����IMGUI
void renderIMGUI();

//���Դ�ƶ�
void lightTransorm();

//����ȫ�ֱ���vao�Լ�shaderProram
//GLuint vao;
float angle = 0.0f;
std::shared_ptr < GLframework::Renderer> renderer = nullptr;
std::vector<std::shared_ptr < GLframework::Mesh>> meshes{};
std::shared_ptr<GLframework::Mesh> meshPointLight = nullptr;
std::shared_ptr <GLframework::AmbientLight> ambientLight = nullptr;
Camera* camera = nullptr;
CameraControl* cameracontrol = nullptr;
glm::vec3 clearColor{};
//�����ƹ�
std::shared_ptr < GLframework::DirectionalLight> dirLight = nullptr;
std::shared_ptr < GLframework::SpotLight> spotLight = nullptr;
std::vector<std::shared_ptr<GLframework::PointLight>> pointLights{};
float specularIntensity = 0.8f;
int main()
{
	std::cout << "Please set the window as x * y" << std::endl;
	int width = 800, height = 600;
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
		lightTransorm();
		meshes[0]->rotateZ(1.0f);
		meshes[1]->rotateY(1.0f);
		cameracontrol->update();
		renderer->setClearColor(clearColor);
		renderer->render(meshes,camera,dirLight,spotLight,pointLights,ambientLight);
		renderIMGUI();
	}

	GL_APP->destory();

	return 0;
}

void lightTransorm()
{
	float xPos = glm::sin(glfwGetTime()) + 2.3f;
	meshPointLight->setPosition(glm::vec3(xPos, 0.0f, 0.0f));
	spotLight->setPosition(glm::vec3(xPos, 0.0f, 0.0f));
}

void prepare()
{
	renderer = std::make_shared<GLframework::Renderer>();

	//1. ����material�������ò���
	auto materialBox			= std::make_shared<GLframework::PhongMaterial>();
	materialBox->mShiness		= 10.0f;
	materialBox->mDiffuse		= std::make_shared<GLframework::Texture>("Texture/box.png", 0);
	materialBox->mSpecularMask	= std::make_shared<GLframework::Texture>("Texture/sp_mask.png", 1);
	auto materialSphere			= std::make_shared<GLframework::PhongMaterial>();
	materialSphere->mShiness = 10.0f;
	materialSphere->mDiffuse = std::make_shared<GLframework::Texture>("Texture/box.png",0);
	materialSphere->mSpecularMask = std::make_shared<GLframework::Texture>("Texture/sp_mask.png", 1);
	//2. ����geogetry
	auto geometrySphere = GLframework::Geometry::createSphere(renderer->getShader(materialSphere->getMaterialType()),
	                                                          0.1f);
	auto geometryBox	= GLframework::Geometry::createBox(renderer->getShader(materialBox->getMaterialType()), 2.0f, 2.0f,
	                                                    2.0f);
	//3. ����mesh,

	auto meshBox	= std::make_shared<GLframework::Mesh>(geometryBox, materialBox);
	meshPointLight	= std::make_shared<GLframework::Mesh>(geometrySphere, materialSphere);
	meshPointLight->setPosition(glm::vec3(1.5f, 0.0f, 0.0f));

	//�������ӹ�ϵ
	meshBox->addChild(meshPointLight);

	meshes.push_back(meshBox);
	meshes.push_back(meshPointLight);

	spotLight	= std::make_shared<GLframework::SpotLight>(glm::vec3(-1.0f, 0.0f, 0.0f), 30.0f, 60.0f);
	spotLight	->	setPosition(glm::vec3(1.5f, 0.0f, 0.0f));
	spotLight	->	setColor(glm::vec3{0.0f});

	dirLight	= std::make_shared<GLframework::DirectionalLight>();
	dirLight	->	setDirection(glm::vec3(1.0f, 1.0f, 1.0f));
	dirLight	->	setColor(glm::vec3{0.0f});

	auto pointLight1 = std::make_shared<GLframework::PointLight>();
	pointLight1	->	setSpecularIntensity(0.8f);
	pointLight1	->	setK(0.017f, 0.07f, 1.0f);
	pointLight1	->	setColor(glm::vec3(1.0f, 0.0f, 0.0f));
	pointLight1	->	setPosition(glm::vec3(-1.5f, 0.0f, 0.0f));
	pointLights.push_back(std::move(pointLight1));

	auto pointLight2 = std::make_shared<GLframework::PointLight>();
	pointLight2	->	setSpecularIntensity(0.8f);
	pointLight2	->	setK(0.017f, 0.07f, 1.0f);
	pointLight2	->	setColor(glm::vec3(0.0f, 1.0f, 0.0f));
	pointLight2	->	setPosition(glm::vec3(1.5f, 0.0f, 0.0f));
	pointLights.push_back(std::move(pointLight2));

	auto pointLight3 = std::make_shared<GLframework::PointLight>();
	pointLight3	->	setSpecularIntensity(0.8f);
	pointLight3	->	setK(0.017f, 0.07f, 1.0f);
	pointLight3	->	setColor(glm::vec3(0.0f, 0.0f, 1.0f));
	pointLight3	->	setPosition(glm::vec3(0.0f, 1.5f, 0.0f));
	pointLights.push_back(std::move(pointLight3));

	auto pointLight4 = std::make_shared<GLframework::PointLight>();
	pointLight4	->	setSpecularIntensity(0.8f);
	pointLight4	->	setK(0.017f, 0.07f, 1.0f);
	pointLight4	->	setColor(glm::vec3(1.0f, 1.0f, 1.0f));
	pointLight4	->	setPosition(glm::vec3(0.0f, 0.0f, 1.5f));
	pointLights.push_back(std::move(pointLight4));

	ambientLight	 = std::make_shared<GLframework::AmbientLight>();
	ambientLight->	setColor(glm::vec3{0.0f});
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

void renderIMGUI()
{
	// 1. ������ǰIMGUI��Ⱦ
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 2. ������ǰ��GUI��������Щ�ؼ������ϵ���
	ImGui::Begin("Control");
	ImGui::Text("yeah!\n");
	ImGui::Button("Text Button", ImVec2(40, 20));
	ImGui::ColorEdit3("Clear Color",(float *)(&clearColor));
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
//void prepareOrtho()
//{
//	ortherMartix = glm::ortho(-2.0f, 2.0f, 2.0f, -2.0f, 2.0f, -2.0f);
//}

void initIMGUI()
{
	ImGui::CreateContext();		//����ImGui������
	ImGui::StyleColorsDark();	//ѡ��һ������

	//	����ImGui��GLFW��OpenGL�İ�
	ImGui_ImplGlfw_InitForOpenGL(GL_APP->getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");
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



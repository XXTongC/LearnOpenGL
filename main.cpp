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
#include "materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h"
#include "materials/phongPointShadowMaterial/phongPointShadowMaterial.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "materials/phongShadowMaterial/phongShadowMaterial.h"

#include "depthMaterial.h"
#include "material.h"
#include "framebuffer.h"
#include "scene.h"
#include <chrono>
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
#include "tools/Logger/Logger.h"
#include "tools/Logger/LogManager.h"
#include "tools/editor/EditorPanels.h"
#include "tools/legacyExperiments/LegacyExperimentRunner.h"
#include "tools/sceneSetup/SceneSetup.h"
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
bool initializeApplication();
void runFrame();
void printOpenGLCapabilities();
void cleanupRuntime();
GL_EXPERIMENTS::RuntimeContext makeLegacyExperimentContext();
GL_SCENE::SetupContext makeSceneSetupContext();
void prepareLegacyExperiments();
void updateLegacyExperiments();

//
void prepareCamera();
GL_EDITOR::EditorPanelContext makeEditorPanelContext();

//
void initIMGUI();
void prepareState();
void prepare();

//IMGUI
void renderIMGUI();

//grass texture attribute
int rNum = 30;
int cNum = 30;
float scale = 0.0f;
float brigtnesee = 1.0f;

//GLuint vao;
float angle = 0.0f;
struct AppRuntimeContext
{
	std::shared_ptr<GLframework::Renderer> renderer{ nullptr };
	std::shared_ptr<GLframework::Scene> sceneOffScreen{ nullptr };
	std::shared_ptr<GLframework::Scene> sceneInScreen{ nullptr };
	std::shared_ptr<GLframework::Mesh> meshPointLight{ nullptr };
	std::shared_ptr<GLframework::AmbientLight> ambientLight{ nullptr };
	std::shared_ptr<GLframework::Framebuffer> framebufferMultisample{ nullptr };
	std::shared_ptr<GLframework::Framebuffer> framebufferResolve{ nullptr };
	std::shared_ptr<GLframework::GrassInstanceMaterial> grassMaterial{ nullptr };
	std::shared_ptr<GLframework::Mesh> skyBoxMesh{ nullptr };
	std::shared_ptr<GLframework::Mesh> movePlane{ nullptr };
	std::shared_ptr<GLframework::Mesh> textD{ nullptr };
	std::shared_ptr<GLframework::ScreenMaterial> screenMaterial{ nullptr };
	std::shared_ptr<GLframework::PhongCSMShadowMaterial> csmShadowMaterial{ nullptr };
	Camera* camera{ nullptr };
	CameraControl* cameracontrol{ nullptr };
	glm::vec3 clearColor{};
	std::shared_ptr<GLframework::DirectionalLight> dirLight{ nullptr };
	std::shared_ptr<GLframework::SpotLight> spotLight{ nullptr };
	std::vector<std::shared_ptr<GLframework::PointLight>> pointLights{};
};

AppRuntimeContext gAppRuntime{};
GL_EDITOR::SelectionContext gEditorSelection{};

auto& renderer = gAppRuntime.renderer;
auto& sceneOffScreen = gAppRuntime.sceneOffScreen;
auto& sceneInScreen = gAppRuntime.sceneInScreen;
auto& meshPointLight = gAppRuntime.meshPointLight;
auto& ambientLight = gAppRuntime.ambientLight;
auto& framebufferMultisample = gAppRuntime.framebufferMultisample;
auto& framebufferResolve = gAppRuntime.framebufferResolve;
auto& grassMaterial = gAppRuntime.grassMaterial;
auto& skyBoxMesh = gAppRuntime.skyBoxMesh;
auto& movePlane = gAppRuntime.movePlane;
auto& textD = gAppRuntime.textD;
auto& ScreenMat = gAppRuntime.screenMaterial;
auto& csmShadowMaterial = gAppRuntime.csmShadowMaterial;
Camera*& camera = gAppRuntime.camera;
CameraControl*& cameracontrol = gAppRuntime.cameracontrol;
glm::vec3& clearColor = gAppRuntime.clearColor;
auto& dirLight = gAppRuntime.dirLight;
auto& spotLight = gAppRuntime.spotLight;
auto& pointLights = gAppRuntime.pointLights;
GL_EXPERIMENTS::LegacyExperimentRunner gLegacyExperiments{};

//----skyBox----
std::string TexturePath{ "Texture/bk.jpg" };
//---------------
int width = 1920, height = 1080;
float specularIntensity = 0.8f;

//--------text--------
void moveit()
{
	movePlane->setPosition({ 0.0f,(glm::sin(glfwGetTime()) + 1) * 5,0.0f });
}
float m_time = 0.0f;
//--------------------
int main()
{
	LogManager::getInstance().setMinLevel(LogManager::Level::info);
	if (!initializeApplication()) return -1;
	
	while (GL_APP->update())
	{
		runFrame();
	}

	cleanupRuntime();
	GL_APP->destroy();

	return 0;
}

bool initializeApplication()
{
	std::cout << "Please set the window as x * y" << std::endl;
	if (!setAndInitWindow(width, height)) return false;

	GL_CALL(glViewport(0, 0, width, height));
	GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

	prepareCamera();
	prepare();
	initIMGUI();
	printOpenGLCapabilities();

	return true;
}

void runFrame()
{
	cameracontrol->update();
	renderer->setClearColor(clearColor);
	updateLegacyExperiments();
	//moveit();

	// pass 1: off-screen color attachment
	renderer->render(sceneOffScreen, camera, dirLight, spotLight, pointLights, ambientLight, framebufferMultisample->getFBO());
	renderer->msaaResolve(framebufferMultisample, framebufferResolve);

	// pass 2: on-screen color attachment
	renderer->render(sceneInScreen, camera, dirLight, spotLight, pointLights, ambientLight);
	renderIMGUI();
}

void printOpenGLCapabilities()
{
	int nrAttributes = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
}

void cleanupRuntime()
{
	delete cameracontrol;
	cameracontrol = nullptr;

	delete camera;
	camera = nullptr;
}

GL_EXPERIMENTS::RuntimeContext makeLegacyExperimentContext()
{
	return {
		renderer,
		sceneOffScreen,
		grassMaterial,
		skyBoxMesh,
		movePlane,
		csmShadowMaterial,
		dirLight,
		pointLights
	};
}

GL_SCENE::SetupContext makeSceneSetupContext()
{
	return {
		renderer,
		sceneOffScreen,
		sceneInScreen,
		framebufferMultisample,
		framebufferResolve,
		skyBoxMesh,
		textD,
		ScreenMat,
		ambientLight,
		dirLight,
		spotLight,
		pointLights,
		width,
		height,
		TexturePath
	};
}

void prepare()
{
	auto sceneSetupContext = makeSceneSetupContext();
	GL_SCENE::prepareDefaultScene(sceneSetupContext);
	prepareLegacyExperiments();

	LogInfo(":\n Renderer Prepared\n SceneInScreen Prepared\n SceneOffScreen Prepared \n FramebufferMultisample Prepared\n FramebufferResolve Prepared\n PointLightShadow initialized\n Lights Ready \n Objects Ready");
}

void prepareLegacyExperiments()
{
	auto context = makeLegacyExperimentContext();

	// Re-enable legacy experiments here with a few focused calls.
	// gLegacyExperiments.enableSolarSystem(context);
	// gLegacyExperiments.enableGrassField(context, rNum, cNum);
	// gLegacyExperiments.enableEnvironmentSphere(context);
	// gLegacyExperiments.enableCsmPlane(context);
	// gLegacyExperiments.enableBackpackModel(context);
	// gLegacyExperiments.enableShadowPreview(context);
	// gLegacyExperiments.enableOrbitingPointLight(0, 3.0f, 3.0f);
}

void updateLegacyExperiments()
{
	auto context = makeLegacyExperimentContext();
	gLegacyExperiments.update(context);
}

GL_EDITOR::EditorPanelContext makeEditorPanelContext()
{
	GL_EDITOR::EditorPanelContext context{};
	context.sceneOffScreen = sceneOffScreen;
	context.sceneInScreen = sceneInScreen;
	context.directionalLight = dirLight;
	context.spotLight = spotLight;
	context.pointLights = &pointLights;
	context.mainCamera = camera;
	return context;
}

bool setAndInitWindow(int width, int height)
{
	LogInfo("Window Initializing...");
	if (!GL_APP->init(width,height)) return false;
	GL_APP->setResizeCallback(OnResize);
	GL_APP->setKeyboardCallback(OnKeyboardCallback);
	GL_APP->setMouseCallback(OnMouseCallback);
	GL_APP->setCursorCallback(OnCursor);
	GL_APP->setScrollCallback(OnScroll);
	
	//glClearDepth(0.0);
	LogInfo("Window Initialized");
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
		const float r = 3.0f;
		const float x = static_cast<float>(r * glm::sin(m_time));
		const float z = static_cast<float>(r * glm::cos(m_time));
		pointLights[0]->setPosition({ x, 3.0f, z });
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

	const auto editorContext = makeEditorPanelContext();
	GL_EDITOR::ensureSelectionIsInitialized(gEditorSelection, sceneOffScreen);
	GL_EDITOR::drawHierarchyPanel(editorContext, gEditorSelection);
	GL_EDITOR::drawSelectionInspectorPanel(editorContext, gEditorSelection);

	// 3. ִ��UI��Ⱦ
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(GL_APP->getWindow(), &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	

}

void prepareCamera() 
{
	LogInfo(static_cast<std::string>(__func__)+ "(): " + "Camera preparing...");
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
	LogInfo("Camera prepared");
}

void initIMGUI()
{
	LogInfo("GUI Initializing...");
	ImGui::CreateContext();		//����ImGui������
	ImGui::StyleColorsDark();	//ѡ��һ������

	//	imgui版本设置
	ImGui_ImplGlfw_InitForOpenGL(GL_APP->getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 460");
	LogInfo("GUI Initialized");
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



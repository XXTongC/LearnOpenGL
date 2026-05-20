#include "core.h"
#include <iostream>
#include <memory>
#include <vector>
#include <typeinfo>
#include "GL_ERROR_FIND.h"
#include "AppRuntimeContext.h"
#include "Application.h"
#include "RuntimeBootstrapper.h"
#include "RuntimeInputController.h"
#include "RuntimeProfileLoader.h"
#include "RuntimeScenePreparer.h"
#include "RuntimeViewport.h"
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
#include "scene.h"
#include <chrono>
#include "renderer.h"
#include "renderer/Bloom/Bloom.h"
#include "renderer/EnvironmentProfile.h"
#include "renderer/FrameRenderTargets.h"
#include "renderer/PostProcessPass.h"
#include "renderer/PostProcessSettings.h"
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
#include "tools/editor/DebugControllerPanel.h"
#include "tools/editor/EditorPanels.h"
#include "tools/legacyExperiments/LegacyExperimentRunner.h"
#include "tools/sceneSetup/PBRCameraRigProfile.h"
#include "tools/sceneSetup/PBRExperimentProfile.h"
#include "tools/sceneSetup/PBRLightRigProfile.h"
int GLframework::PointLightShadow::MAX_POINT_LIGHTS = 2;
/*
 * refer to ColorBlend, there are still some problem should be solve such as opacity order, look up OIT and Depth Peeling
*/

#pragma region ���ֻص�����
void OnScroll(double offset);
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void OnResize(int newWidth, int newHeight);
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
GL_RUNTIME::RuntimeScenePrepareConfig makeScenePrepareConfig();
GL_EDITOR::DebugControllerContext makeDebugControllerContext();

//
void prepareCamera();
GL_EDITOR::EditorPanelContext makeEditorPanelContext();

//
void initIMGUI();
void prepareState();

//IMGUI
void renderIMGUI();

//grass texture attribute
int rNum = 30;
int cNum = 30;
float scale = 0.0f;
float brigtnesee = 1.0f;

//GLuint vao;
float angle = 0.0f;
GLframework::AppRuntimeContext gAppRuntime{};
GL_EDITOR::SelectionContext gEditorSelection{};

auto& renderer = gAppRuntime.renderer;
auto& sceneOffScreen = gAppRuntime.sceneOffScreen;
auto& sceneInScreen = gAppRuntime.sceneInScreen;
auto& meshPointLight = gAppRuntime.meshPointLight;
auto& screenQuad = gAppRuntime.screenQuad;
auto& ambientLight = gAppRuntime.ambientLight;
auto& frameRenderTargets = gAppRuntime.frameRenderTargets;
auto& bloom = gAppRuntime.bloom;
auto& grassMaterial = gAppRuntime.grassMaterial;
auto& skyBoxMesh = gAppRuntime.skyBoxMesh;
auto& movePlane = gAppRuntime.movePlane;
auto& textD = gAppRuntime.textD;
auto& ScreenMat = gAppRuntime.screenMaterial;
auto& csmShadowMaterial = gAppRuntime.csmShadowMaterial;
auto& postProcessPass = gAppRuntime.postProcessPass;
auto& postProcessSettings = gAppRuntime.postProcessSettings;
auto& postProcessSettingsPath = gAppRuntime.postProcessSettingsPath;
auto& environmentProfile = gAppRuntime.environmentProfile;
auto& environmentProfilePath = gAppRuntime.environmentProfilePath;
auto& pbrPreviewProfile = gAppRuntime.pbrPreviewProfile;
auto& pbrLightRigProfile = gAppRuntime.pbrLightRigProfile;
auto& pbrCameraRigProfile = gAppRuntime.pbrCameraRigProfile;
auto& pbrPreviewProfilePath = gAppRuntime.pbrPreviewProfilePath;
auto& pbrExperimentProfilePath = gAppRuntime.pbrExperimentProfilePath;
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
	return GL_RUNTIME::RuntimeBootstrapper::run({
		[]() { return initializeApplication(); },
		[]() { return GL_APP->update(); },
		[]() { runFrame(); },
		[]() { cleanupRuntime(); },
		[]() { GL_APP->destroy(); }
	});
}

bool initializeApplication()
{
	std::cout << "Please set the window as x * y" << std::endl;
	if (!setAndInitWindow(width, height)) return false;

	GL_RUNTIME::RuntimeViewport::applyViewport(width, height);
	GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

	prepareCamera();
	GL_RUNTIME::RuntimeProfileLoader::loadAll(gAppRuntime);
	GL_RUNTIME::RuntimeScenePreparer::prepare(gAppRuntime, gLegacyExperiments, makeScenePrepareConfig());
	initIMGUI();
	printOpenGLCapabilities();

	return true;
}

void runFrame()
{
	cameracontrol->update();
	renderer->setClearColor(clearColor);
	GL_RUNTIME::RuntimeScenePreparer::updateLegacyExperiments(gAppRuntime, gLegacyExperiments);
	//moveit();

	// pass 1: off-screen color attachment
	renderer->render(sceneOffScreen, camera, dirLight, spotLight, pointLights, ambientLight, frameRenderTargets.getSceneFbo());
	postProcessPass.resolveMultisample(frameRenderTargets.getMultisample(), frameRenderTargets.getResolved());
	if (postProcessSettings.bloomEnabled)
	{
		postProcessPass.extractBloomBright(
			bloom,
			frameRenderTargets.getResolved(),
			frameRenderTargets.getBloomBright(),
			postProcessSettings.bloomThreshold
		);
		postProcessPass.blurBloom(
			bloom,
			frameRenderTargets.getBloomBright(),
			frameRenderTargets.getBloomPing(),
			frameRenderTargets.getBloomPong(),
			postProcessSettings.bloomIterations
		);
	}

	// pass 2: post-process composite to default framebuffer
	postProcessPass.renderScreenComposite(
		screenQuad,
		renderer->getShader(GLframework::MaterialType::ScreenMaterial),
		postProcessSettings,
		static_cast<unsigned int>(GL_APP->getWidth()),
		static_cast<unsigned int>(GL_APP->getHeight())
	);
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

GL_RUNTIME::RuntimeScenePrepareConfig makeScenePrepareConfig()
{
	return {
		width,
		height,
		TexturePath,
		rNum,
		cNum
	};
}

GL_EDITOR::DebugControllerContext makeDebugControllerContext()
{
	return {
		&dirLight,
		&ambientLight,
		&spotLight,
		&pointLights,
		textD,
		&postProcessSettings,
		&postProcessSettingsPath,
		renderer,
		&environmentProfile,
		&environmentProfilePath,
		&pbrPreviewProfile,
		&pbrPreviewProfilePath,
		&pbrExperimentProfilePath,
		&pbrLightRigProfile,
		&pbrCameraRigProfile,
		camera,
		&m_time
	};
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

	GL_EDITOR::drawDebugControllerPanel(makeDebugControllerContext());
	const auto editorContext = makeEditorPanelContext();
	GL_EDITOR::ensureSelectionIsInitialized(gEditorSelection, sceneOffScreen);
	GL_EDITOR::drawHierarchyPanel(editorContext, gEditorSelection);
	GL_EDITOR::drawSelectionInspectorPanel(editorContext, gEditorSelection);

	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(GL_APP->getWindow(), &display_w, &display_h);
	GL_RUNTIME::RuntimeViewport::applyViewport(display_w, display_h);
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
	GL_RUNTIME::RuntimeInputController::handleScroll(offset, { camera, cameracontrol });
}

void OnResize(int newWidth, int newHeight)
{
	const auto result = GL_RUNTIME::RuntimeViewport::applyResize(
		newWidth,
		newHeight,
		{ &width, &height, camera, &frameRenderTargets, ScreenMat }
	);

#ifdef _DEBUG
	if (result.accepted)
	{
		std::cout << "OnResize" << std::endl;
	}
#endif

}

void OnKeyboardCallback(int key, int action, int mods)
{
	GL_RUNTIME::RuntimeInputController::handleKey(key, action, mods, { camera, cameracontrol });
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
	GL_RUNTIME::RuntimeInputController::handleMouse(button, action, x, y, { camera, cameracontrol });
	
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
	GL_RUNTIME::RuntimeInputController::handleCursor(xpos, ypos, { camera, cameracontrol });
}
#pragma endregion



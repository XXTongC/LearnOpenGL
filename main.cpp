#include "core.h"
#include <iostream>
#include <memory>
#include <vector>
#include <typeinfo>
#include "GL_ERROR_FIND.h"
#include "AppRuntimeContext.h"
#include "Application.h"
#include "RuntimeBootstrapper.h"
#include "RuntimeCameraLifecycle.h"
#include "RuntimeEditorPanelCoordinator.h"
#include "RuntimeFrameRunner.h"
#include "RuntimeGuiHost.h"
#include "RuntimeProfileLoader.h"
#include "RuntimeScenePreparer.h"
#include "RuntimeViewport.h"
#include "RuntimeWindowLifecycle.h"
#include "tools/tools.h"
#include "shader.h"
#include "texture.h"
#include "trackBallCameraControl.h"
#include "opacityMaskMatetial.h"
#include "cubeMaterial.h"
#include "screenMaterial.h"
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
#include "assimpInstanceLoader.h"
#include "assimpLoader.h"
#include "phongEnvMaterial.h"
#include "materials/phongPointShadowMaterial/phongPointShadowMaterial.h"
#include "tools/Logger/Logger.h"
#include "tools/Logger/LogManager.h"
#include "tools/legacyExperiments/LegacyExperimentRunner.h"
#include "tools/sceneSetup/PBRCameraRigProfile.h"
#include "tools/sceneSetup/PBRExperimentProfile.h"
#include "tools/sceneSetup/PBRLightRigProfile.h"
int GLframework::PointLightShadow::MAX_POINT_LIGHTS = 2;
/*
 * refer to ColorBlend, there are still some problem should be solve such as opacity order, look up OIT and Depth Peeling
*/

bool initializeApplication();
void runFrame();
void printOpenGLCapabilities();
void cleanupRuntime();
GL_RUNTIME::RuntimeFrameConfig makeFrameConfig();
GL_RUNTIME::RuntimeCameraConfig makeCameraConfig();
GL_RUNTIME::RuntimeScenePrepareConfig makeScenePrepareConfig();

void renderFrameUi();
void drawEditorPanels();

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
	if (!GL_RUNTIME::RuntimeWindowLifecycle::initialize(
		{ width, height },
		{ &gAppRuntime, &width, &height }
	)) return false;

	GL_RUNTIME::RuntimeViewport::applyViewport(width, height);
	GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));

	GL_RUNTIME::RuntimeCameraLifecycle::initializeDefaultCamera(gAppRuntime, makeCameraConfig());
	GL_RUNTIME::RuntimeProfileLoader::loadAll(gAppRuntime);
	GL_RUNTIME::RuntimeScenePreparer::prepare(gAppRuntime, gLegacyExperiments, makeScenePrepareConfig());
	GL_RUNTIME::RuntimeGuiHost::initialize({ GL_APP->getWindow() });
	printOpenGLCapabilities();

	return true;
}

void runFrame()
{
	GL_RUNTIME::RuntimeFrameRunner::run(gAppRuntime, gLegacyExperiments, makeFrameConfig(), { renderFrameUi });
}

GL_RUNTIME::RuntimeFrameConfig makeFrameConfig()
{
	return {
		static_cast<unsigned int>(GL_APP->getWidth()),
		static_cast<unsigned int>(GL_APP->getHeight())
	};
}

void printOpenGLCapabilities()
{
	int nrAttributes = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
}

void cleanupRuntime()
{
	GL_RUNTIME::RuntimeCameraLifecycle::cleanup(gAppRuntime);
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

GL_RUNTIME::RuntimeCameraConfig makeCameraConfig()
{
	return {
		static_cast<int>(GL_APP->getWidth()),
		static_cast<int>(GL_APP->getHeight())
	};
}

void renderFrameUi()
{
	GL_RUNTIME::RuntimeGuiHost::renderFrame({ GL_APP->getWindow(), drawEditorPanels });
}

void drawEditorPanels()
{
	GL_RUNTIME::RuntimeEditorPanelCoordinator::drawPanels(gAppRuntime, gEditorSelection, &m_time);
}


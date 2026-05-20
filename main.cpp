#include "core.h"
#include <iostream>
#include <string>
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
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "tools/Logger/LogManager.h"
#include "tools/legacyExperiments/LegacyExperimentRunner.h"
int GLframework::PointLightShadow::MAX_POINT_LIGHTS = 2;
/*
 * refer to ColorBlend, there are still some problem should be solve such as opacity order, look up OIT and Depth Peeling
*/

namespace
{
	struct MainStartupConfig
	{
		GL_RUNTIME::RuntimeWindowConfig window{ 1920, 1080 };
		std::string skyboxTexturePath{ "Texture/bk.jpg" };
		int legacyGrassRows{ 30 };
		int legacyGrassColumns{ 30 };
		float editorOrbitAngle{ 0.0f };
	};

	GLframework::AppRuntimeContext gAppRuntime{};
	GL_EDITOR::SelectionContext gEditorSelection{};
	GL_EXPERIMENTS::LegacyExperimentRunner gLegacyExperiments{};
	MainStartupConfig gStartupConfig{};

	bool initializeApplication();
	void runFrame();
	void printOpenGLCapabilities();
	void cleanupRuntime();
	GL_RUNTIME::RuntimeFrameConfig makeFrameConfig();
	GL_RUNTIME::RuntimeCameraConfig makeCameraConfig();
	GL_RUNTIME::RuntimeScenePrepareConfig makeScenePrepareConfig();
	void renderFrameUi();
	void drawEditorPanels();
}

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

namespace
{
bool initializeApplication()
{
	std::cout << "Please set the window as x * y" << std::endl;
	if (!GL_RUNTIME::RuntimeWindowLifecycle::initialize(
		gStartupConfig.window,
		{ &gAppRuntime, &gStartupConfig.window.width, &gStartupConfig.window.height }
	)) return false;

	GL_RUNTIME::RuntimeViewport::applyViewport(gStartupConfig.window.width, gStartupConfig.window.height);
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
		gStartupConfig.window.width,
		gStartupConfig.window.height,
		gStartupConfig.skyboxTexturePath,
		gStartupConfig.legacyGrassRows,
		gStartupConfig.legacyGrassColumns
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
	GL_RUNTIME::RuntimeEditorPanelCoordinator::drawPanels(gAppRuntime, gEditorSelection, &gStartupConfig.editorOrbitAngle);
}
}


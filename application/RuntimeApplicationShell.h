#pragma once

#include <string>

#include "AppRuntimeContext.h"
#include "RuntimeBootstrapper.h"
#include "RuntimeCameraLifecycle.h"
#include "RuntimeFrameRunner.h"
#include "RuntimeScenePreparer.h"
#include "RuntimeWindowLifecycle.h"
#include "../tools/editor/EditorPanels.h"
#include "../tools/legacyExperiments/LegacyExperimentRunner.h"

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig
	{
		RuntimeWindowConfig window{ 1920, 1080 };
		std::string skyboxTexturePath{ "Texture/bk.jpg" };
		int legacyGrassRows{ 30 };
		int legacyGrassColumns{ 30 };
		float editorOrbitAngle{ 0.0f };
	};

	class RuntimeApplicationShell
	{
	public:
		RuntimeApplicationShell() = default;
		explicit RuntimeApplicationShell(RuntimeApplicationShellConfig config);

		RuntimeBootstrapperCallbacks makeCallbacks();

	private:
		bool initialize();
		bool shouldContinue();
		void runFrame();
		void cleanup();
		void destroy();

		RuntimeFrameConfig makeFrameConfig() const;
		RuntimeCameraConfig makeCameraConfig() const;
		RuntimeScenePrepareConfig makeScenePrepareConfig() const;
		void renderFrameUi();
		void drawEditorPanels();
		void printOpenGLCapabilities() const;

		GLframework::AppRuntimeContext mRuntime{};
		GL_EDITOR::SelectionContext mEditorSelection{};
		GL_EXPERIMENTS::LegacyExperimentRunner mLegacyExperiments{};
		RuntimeApplicationShellConfig mConfig{};
	};
}

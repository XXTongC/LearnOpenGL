#include "RuntimeContentConfigPolicy.h"

#include "RuntimeApplicationConfig.h"
#include "RuntimeContentLifecycleConfig.h"

namespace GL_RUNTIME
{
	RuntimeContentLifecycleConfig RuntimeContentConfigPolicy::makeContentLifecycleConfig(
		const RuntimeApplicationShellConfig& shellConfig,
		int framebufferWidth,
		int framebufferHeight
	)
	{
		RuntimeContentLifecycleConfig config{};
		config.camera = {
			framebufferWidth,
			framebufferHeight
		};
		config.scene.width = shellConfig.window.width;
		config.scene.height = shellConfig.window.height;
		config.scene.texturePath = shellConfig.skyboxTexturePath;
		config.scene.legacyGrassRows = shellConfig.legacyGrassRows;
		config.scene.legacyGrassColumns = shellConfig.legacyGrassColumns;
		config.scene.sceneSetupPipeline.useWorldDrivenMinimalScene =
			shellConfig.verification.enabled && shellConfig.verification.engineWorld.enableMinimalScene;
		if (config.scene.sceneSetupPipeline.useWorldDrivenMinimalScene)
		{
			config.scene.sceneSetupPipeline.useLegacyDefaultScene = false;
		}
		config.scene.sceneSetupPipeline.addWorldDrivenProbe =
			shellConfig.verification.enabled && shellConfig.verification.engineWorld.enableSceneProbe;
		config.verification = shellConfig.verification;
		config.rendererBackendKey = shellConfig.rendererBackendKey;
		return config;
	}
}

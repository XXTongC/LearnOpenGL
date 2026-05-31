#pragma once

#include <string>

#include "../tools/sceneSetup/SceneSetupPipelineConfig.h"

namespace GL_RUNTIME
{
	struct RuntimeScenePrepareConfig
	{
		int width{ 0 };
		int height{ 0 };
		std::string texturePath{};
		int legacyGrassRows{ 30 };
		int legacyGrassColumns{ 30 };
		GL_SCENE::SceneSetupPipelineConfig sceneSetupPipeline{};
	};
}

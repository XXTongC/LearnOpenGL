#pragma once

#include <string>

#include "AppRuntimeContext.h"
#include "../tools/legacyExperiments/LegacyExperimentRunner.h"
#include "../tools/sceneSetup/SceneSetup.h"

namespace GL_RUNTIME
{
	struct RuntimeScenePrepareConfig
	{
		int width{ 0 };
		int height{ 0 };
		std::string texturePath{};
		int legacyGrassRows{ 30 };
		int legacyGrassColumns{ 30 };
	};

	class RuntimeScenePreparer
	{
	public:
		static GL_SCENE::SetupContext makeSceneSetupContext(
			GLframework::AppRuntimeContext& context,
			const RuntimeScenePrepareConfig& config
		);

		static GL_EXPERIMENTS::RuntimeContext makeLegacyExperimentContext(
			GLframework::AppRuntimeContext& context
		);

		static void prepare(
			GLframework::AppRuntimeContext& context,
			GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
			const RuntimeScenePrepareConfig& config
		);

		static void prepareLegacyExperiments(
			GLframework::AppRuntimeContext& context,
			GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments,
			const RuntimeScenePrepareConfig& config
		);

		static void updateLegacyExperiments(
			GLframework::AppRuntimeContext& context,
			GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments
		);
	};
}

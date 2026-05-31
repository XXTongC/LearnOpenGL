#pragma once

#include <string>

#include "../../engine/LegacySceneWorldBuilder.h"
#include "SceneSetup.h"
#include "SceneSetupPipelineConfig.h"
#include "WorldDrivenSceneSetup.h"

namespace GL_SCENE
{
	struct SceneSetupPipelineResult
	{
		bool legacyDefaultPrepared{ false };
		bool legacyWorldMirrorPrepared{ false };
		bool worldDrivenMinimalScenePrepared{ false };
		bool worldDrivenProbeRequested{ false };
		GLengine::LegacySceneImportStats legacyWorldMirrorStats{};
		WorldDrivenMinimalSceneStats worldDrivenMinimalSceneStats{};
		WorldDrivenSceneProbeStats worldDrivenProbeStats{};
	};

	SceneSetupPipelineResult prepareScene(
		SetupContext& context,
		const SceneSetupPipelineConfig& config = {}
	);

	std::string formatLegacyWorldMirrorStats(const GLengine::LegacySceneImportStats& stats);
}

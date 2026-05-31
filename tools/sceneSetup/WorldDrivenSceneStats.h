#pragma once

#include "../../engine/LegacySceneWorldStats.h"

namespace GL_SCENE
{
	struct WorldDrivenSceneProbeStats
	{
		bool added{ false };
		int createdActors{ 0 };
		int createdMeshes{ 0 };
		GLengine::WorldLegacySceneExportStats exportStats{};
	};

	struct WorldDrivenMinimalSceneStats
	{
		bool added{ false };
		int createdActors{ 0 };
		int createdMeshes{ 0 };
		GLengine::WorldLegacySceneExportStats exportStats{};
	};
}

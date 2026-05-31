#pragma once

#include <memory>

#include "engine/Transform.h"

namespace GLframework
{
	class Object;
	class Scene;
}

namespace GLengine
{
	class Level;
	class World;

	struct WorldLegacySceneExportStats
	{
		int visitedActors{ 0 };
		int visitedSceneComponents{ 0 };
		int exportedObjects{ 0 };
		int exportedMeshes{ 0 };
		int exportedLights{ 0 };
		int exportedLegacyObjects{ 0 };
		int sceneRootObjects{ 0 };
		int objectAttachments{ 0 };
	};

	class WorldLegacySceneExporter
	{
	public:
		static WorldLegacySceneExportStats exportWorldToScene(
			const World& world,
			const std::shared_ptr<GLframework::Scene>& scene
		);

		static WorldLegacySceneExportStats exportLevelToScene(
			const Level& level,
			const std::shared_ptr<GLframework::Scene>& scene
		);

		static void applyTransform(GLframework::Object& object, const Transform& transform);
	};
}

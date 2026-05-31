#pragma once

#include <memory>

#include "engine/LegacySceneWorldStats.h"

namespace GLframework
{
	class Object;
	class Scene;
}

namespace GLengine
{
	class Level;
	class World;
	struct Transform;

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

#pragma once

#include <memory>
#include <string>

#include "engine/LegacySceneWorldStats.h"
#include "engine/PersistentIdPolicy.h"

namespace GLframework
{
	class Object;
	class Scene;
}

namespace GLengine
{
	class AssetRegistry;
	class Level;
	struct Transform;
	class World;

	struct LegacySceneImportOptions
	{
		PersistentIdSource persistentIdSource{ PersistentIdSource::LegacyMirrorDerived };
		std::string persistentIdScope{ "legacy-scene-mirror" };
		AssetRegistry* assetRegistry{ nullptr };
		std::string assetHandleSource{ "legacy-scene" };
		std::string assetHandleScope{};
	};

	class LegacySceneWorldBuilder
	{
	public:
		static LegacySceneImportStats importScene(
			World& world,
			const std::shared_ptr<GLframework::Scene>& scene
		);
		static LegacySceneImportStats importScene(
			World& world,
			const std::shared_ptr<GLframework::Scene>& scene,
			const LegacySceneImportOptions& options
		);

		static LegacySceneImportStats importObjectTree(
			Level& level,
			const std::shared_ptr<GLframework::Object>& root
		);
		static LegacySceneImportStats importObjectTree(
			Level& level,
			const std::shared_ptr<GLframework::Object>& root,
			const LegacySceneImportOptions& options
		);

		static Transform makeTransform(const GLframework::Object& object);
	};
}

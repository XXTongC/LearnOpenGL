#pragma once

#include <string>

namespace GLengine
{
	class World;
}

namespace GL_EDITOR
{
	struct SceneTransformSnapshotResult
	{
		bool saved{ false };
		int actorCount{ 0 };
		int sceneComponentCount{ 0 };
		std::string path{};
		std::string error{};
	};

	struct SceneTransformSnapshotApplyResult
	{
		bool applied{ false };
		int snapshotActorCount{ 0 };
		int snapshotSceneComponentCount{ 0 };
		int matchedSceneComponentCount{ 0 };
		int matchedByPersistentIdCount{ 0 };
		int matchedByStablePathCount{ 0 };
		int matchedByObjectIdCount{ 0 };
		int matchedByIndexCount{ 0 };
		int appliedSceneComponentCount{ 0 };
		int changedSceneComponentCount{ 0 };
		std::string path{};
		std::string error{};
	};

	std::string defaultSceneTransformSnapshotPath();
	SceneTransformSnapshotResult saveSceneTransformSnapshot(
		const GLengine::World& world,
		const std::string& path
	);
	SceneTransformSnapshotApplyResult applySceneTransformSnapshot(
		GLengine::World& world,
		const std::string& path
	);
}

#pragma once

namespace GLengine
{
	struct LegacySceneImportStats
	{
		int visitedObjects{ 0 };
		int meshActors{ 0 };
		int lightActors{ 0 };
		int legacyObjectActors{ 0 };
		int componentAttachments{ 0 };
		int actorsWithPersistentIds{ 0 };
		int sceneComponentsWithPersistentIds{ 0 };
		int assetHandles{ 0 };
		int meshAssetHandles{ 0 };
		int materialAssetHandles{ 0 };
		int textureAssetHandles{ 0 };

		int importedActorCount() const
		{
			return meshActors + lightActors + legacyObjectActors;
		}
	};

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
}

#pragma once

#include <string>

namespace GLengine
{
	struct ScenePackageLoadResult;
}

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState;

	struct RuntimeEngineWorldPreparedSceneStatsSnapshot
	{
		int engineWorldProbeMeshCount{ 0 };
		int engineWorldMinimalMeshCount{ 0 };
	};

	class RuntimeEngineWorldVerificationResourceAdapter
	{
	public:
		static RuntimeEngineWorldPreparedSceneStatsSnapshot collectPreparedSceneStats(
			const RuntimeRenderResourceState& renderResources
		);

		static GLengine::ScenePackageLoadResult loadRuntimeGeneratedScenePackage(
			const RuntimeRenderResourceState& renderResources,
			const std::string& path
		);
	};
}

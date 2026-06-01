#pragma once

namespace GLframework
{
	struct RendererFrameStats;
}

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState;

	struct RuntimePBRPreparedSceneStatsSnapshot
	{
		int objectCount{ 0 };
		int meshCount{ 0 };
		int pbrMeshCount{ 0 };
		int pbrPreviewMeshCount{ 0 };
		int transparentMeshCount{ 0 };
		int pbrTransparentMeshCount{ 0 };
		int pbrEmissiveMeshCount{ 0 };
		int pbrCustomIblMeshCount{ 0 };
		int pbrAlphaMaskedMeshCount{ 0 };
		int pbrImportedMeshCount{ 0 };
		int pbrTexturedMeshCount{ 0 };
		int pbrShowcaseSphereCount{ 0 };
		bool iblReady{ false };
	};

	class RuntimePBRStatsResourceAdapter
	{
	public:
		static const GLframework::RendererFrameStats* lastRendererFrameStats(
			const RuntimeRenderResourceState& renderResources
		);

		static RuntimePBRPreparedSceneStatsSnapshot collectPreparedSceneStats(
			const RuntimeRenderResourceState& renderResources
		);
	};
}

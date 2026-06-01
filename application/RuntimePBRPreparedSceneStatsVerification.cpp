#include "RuntimePBRPreparedSceneStatsVerification.h"

#include <iostream>
#include <string>

#include "../tools/Logger/LogManager.h"
#include "AppRuntimeContext.h"
#include "RuntimePBRStatsResourceAdapter.h"

namespace
{
	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}
}

namespace GL_RUNTIME
{
	void RuntimePBRPreparedSceneStatsVerification::reportPreparedScene(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig&
	)
	{
		const auto stats = RuntimePBRStatsResourceAdapter::collectPreparedSceneStats(context.renderResources);

		reportLine(
			"PBR verification scene stats: objects=" + std::to_string(stats.objectCount)
			+ ", meshes=" + std::to_string(stats.meshCount)
			+ ", pbrMeshes=" + std::to_string(stats.pbrMeshCount)
			+ ", pbrPreviewMeshes=" + std::to_string(stats.pbrPreviewMeshCount)
			+ ", transparentMeshes=" + std::to_string(stats.transparentMeshCount)
			+ ", pbrTransparentMeshes=" + std::to_string(stats.pbrTransparentMeshCount)
			+ ", pbrEmissiveMeshes=" + std::to_string(stats.pbrEmissiveMeshCount)
			+ ", pbrCustomIblMeshes=" + std::to_string(stats.pbrCustomIblMeshCount)
			+ ", pbrAlphaMaskedMeshes=" + std::to_string(stats.pbrAlphaMaskedMeshCount)
			+ ", pbrImportedMeshes=" + std::to_string(stats.pbrImportedMeshCount)
			+ ", pbrTexturedMeshes=" + std::to_string(stats.pbrTexturedMeshCount)
			+ ", pbrShowcaseSpheres=" + std::to_string(stats.pbrShowcaseSphereCount)
			+ ", iblReady=" + (stats.iblReady ? std::string{ "yes" } : std::string{ "no" })
		);
	}
}

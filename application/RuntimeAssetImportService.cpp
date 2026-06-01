#include "RuntimeAssetImportService.h"

#include "RuntimeRenderResourceState.h"
#include "assimpLoader.h"

std::shared_ptr<GLframework::Object> GL_RUNTIME::RuntimeAssetImportService::loadPbrAsset(
	const RuntimeRenderResourceState& renderResources,
	const std::string& path
)
{
	if (!renderResources.renderer())
	{
		return nullptr;
	}

	return GL_APPLICATION::AssimpLoader::loadPBR(path, renderResources.renderer());
}

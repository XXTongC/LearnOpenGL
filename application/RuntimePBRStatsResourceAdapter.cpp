#include "RuntimePBRStatsResourceAdapter.h"

#include <memory>

#include <glm/geometric.hpp>

#include "../framework/object.h"
#include "../framework/scene.h"
#include "../materials/material.h"
#include "../materials/pbrMaterial/PBRMaterial.h"
#include "../mesh/mesh.h"
#include "../renderer/EnvironmentRenderTargets.h"
#include "../renderer/RendererFrameStats.h"
#include "../renderer/renderer.h"
#include "RuntimeRenderResourceState.h"

namespace
{
	void collectSceneStats(
		const std::shared_ptr<GLframework::Object>& object,
		GL_RUNTIME::RuntimePBRPreparedSceneStatsSnapshot& stats,
		bool importedAssetSubtree = false
	)
	{
		if (!object)
		{
			return;
		}

		++stats.objectCount;
		const bool importedAsset = importedAssetSubtree || object->getName().find("PBR Imported") == 0;
		const bool texturedProbe = object->getName().find("PBR Texture Set") == 0;
		const bool showcaseSphere = object->getName().find("PBR Showcase Sphere") == 0;
		if (object->getType() == GLframework::ObjectType::Mesh)
		{
			++stats.meshCount;
		}

		const auto mesh = std::dynamic_pointer_cast<GLframework::Mesh>(object);
		if (mesh && mesh->getMaterial() && mesh->getMaterial()->getMaterialType() == GLframework::MaterialType::PBRMaterial)
		{
			++stats.pbrMeshCount;
			if (importedAsset)
			{
				++stats.pbrImportedMeshCount;
			}
			if (texturedProbe)
			{
				++stats.pbrTexturedMeshCount;
			}
			if (showcaseSphere)
			{
				++stats.pbrShowcaseSphereCount;
			}
			const auto pbrMaterial = std::dynamic_pointer_cast<GLframework::PBRMaterial>(mesh->getMaterial());
			if (pbrMaterial && pbrMaterial->mEmissiveIntensity > 0.0f && glm::length(pbrMaterial->mEmissiveColor) > 0.0001f)
			{
				++stats.pbrEmissiveMeshCount;
			}
			if (pbrMaterial && pbrMaterial->mUseAlphaMask)
			{
				++stats.pbrAlphaMaskedMeshCount;
			}
			if (pbrMaterial
				&& pbrMaterial->mUseIBL
				&& (pbrMaterial->mIblDiffuseStrength < 0.999f
					|| pbrMaterial->mIblDiffuseStrength > 1.001f
					|| pbrMaterial->mIblSpecularStrength < 0.999f
					|| pbrMaterial->mIblSpecularStrength > 1.001f))
			{
				++stats.pbrCustomIblMeshCount;
			}
		}
		if (mesh && mesh->getMaterial() && mesh->getMaterial()->getColorBlendState())
		{
			++stats.transparentMeshCount;
			if (mesh->getMaterial()->getMaterialType() == GLframework::MaterialType::PBRMaterial)
			{
				++stats.pbrTransparentMeshCount;
			}
		}

		if (object->getName().find("PBR Preview") == 0)
		{
			++stats.pbrPreviewMeshCount;
		}

		for (const auto& child : object->getChildren())
		{
			collectSceneStats(child, stats, importedAsset);
		}
	}
}

const GLframework::RendererFrameStats* GL_RUNTIME::RuntimePBRStatsResourceAdapter::lastRendererFrameStats(
	const RuntimeRenderResourceState& renderResources
)
{
	if (!renderResources.renderer())
	{
		return nullptr;
	}

	return &renderResources.renderer()->getLastFrameStats();
}

GL_RUNTIME::RuntimePBRPreparedSceneStatsSnapshot GL_RUNTIME::RuntimePBRStatsResourceAdapter::collectPreparedSceneStats(
	const RuntimeRenderResourceState& renderResources
)
{
	RuntimePBRPreparedSceneStatsSnapshot stats{};
	collectSceneStats(std::static_pointer_cast<GLframework::Object>(renderResources.sceneOffScreen()), stats);
	stats.iblReady = renderResources.renderer()
		&& renderResources.renderer()->getEnvironmentRenderTargets().hasPrecomputedEnvironment();
	return stats;
}

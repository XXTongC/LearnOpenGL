#include "RuntimePBRPreparedSceneStatsVerification.h"

#include <iostream>
#include <memory>
#include <string>

#include "../framework/object.h"
#include "../framework/scene.h"
#include "../materials/material.h"
#include "../materials/pbrMaterial/PBRMaterial.h"
#include "../mesh/mesh.h"
#include "../renderer/EnvironmentRenderTargets.h"
#include "../renderer/renderer.h"
#include "../tools/Logger/LogManager.h"
#include "AppRuntimeContext.h"

namespace
{
	struct PBRVerificationSceneStats
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
	};

	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}

	void collectSceneStats(
		const std::shared_ptr<GLframework::Object>& object,
		PBRVerificationSceneStats& stats,
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

namespace GL_RUNTIME
{
	void RuntimePBRPreparedSceneStatsVerification::reportPreparedScene(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig&
	)
	{
		PBRVerificationSceneStats stats{};
		collectSceneStats(std::static_pointer_cast<GLframework::Object>(context.renderResources.sceneOffScreen), stats);
		const bool environmentReady = context.renderResources.renderer
			&& context.renderResources.renderer->getEnvironmentRenderTargets().hasPrecomputedEnvironment();

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
			+ ", iblReady=" + (environmentReady ? std::string{ "yes" } : std::string{ "no" })
		);
	}

}

#include "RuntimeImportedAssetVerification.h"

#include <iostream>
#include <string>

#include "RuntimeVerificationConfig.h"
#include "assimpLoader.h"
#include "../engine/AssetRegistry.h"
#include "../engine/AssetSubsystem.h"
#include "../engine/LegacySceneWorldBuilder.h"
#include "../engine/Level.h"
#include "../engine/PersistentIdPolicy.h"
#include "../engine/ScenePackage.h"
#include "../engine/World.h"
#include "../tools/Logger/LogManager.h"
#include "AppRuntimeContext.h"

namespace
{
	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}

	std::string yesNo(bool value)
	{
		return value ? "yes" : "no";
	}

	std::string formatImportedAssetWorldImportStats(const GLengine::LegacySceneImportStats& stats)
	{
		return "PBR imported asset engine world import stats: visitedObjects=" + std::to_string(stats.visitedObjects)
			+ ", actors=" + std::to_string(stats.importedActorCount())
			+ ", meshActors=" + std::to_string(stats.meshActors)
			+ ", lightActors=" + std::to_string(stats.lightActors)
			+ ", legacyObjectActors=" + std::to_string(stats.legacyObjectActors)
			+ ", componentAttachments=" + std::to_string(stats.componentAttachments)
			+ ", persistentActors=" + std::to_string(stats.actorsWithPersistentIds)
			+ ", persistentSceneComponents=" + std::to_string(stats.sceneComponentsWithPersistentIds)
			+ ", assetHandles=" + std::to_string(stats.assetHandles)
			+ ", meshAssetHandles=" + std::to_string(stats.meshAssetHandles)
			+ ", materialAssetHandles=" + std::to_string(stats.materialAssetHandles)
			+ ", textureAssetHandles=" + std::to_string(stats.textureAssetHandles)
			+ ", assetHandleSource=imported-asset"
			+ ", persistentIdSource=" + std::string(GLengine::persistentIdSourceToken(GLengine::PersistentIdSource::ImportedAssetDerived));
	}

	std::string formatImportedAssetScenePackageStats(
		const GLengine::ScenePackageSaveResult& save,
		const GLengine::ScenePackageLoadResult& load
	)
	{
		return "PBR imported asset scene package stats: saved=" + yesNo(save.saved)
			+ ", loaded=" + yesNo(load.loaded)
			+ ", assetManifest=" + std::to_string(save.assetManifestCount)
			+ ", meshAssets=" + std::to_string(save.meshAssetManifestCount)
			+ ", materialAssets=" + std::to_string(save.materialAssetManifestCount)
			+ ", textureAssets=" + std::to_string(save.textureAssetManifestCount)
			+ ", loadedAssetManifest=" + std::to_string(load.loadedAssetManifestCount)
			+ ", loadedMeshAssets=" + std::to_string(load.loadedMeshAssetManifestCount)
			+ ", loadedMaterialAssets=" + std::to_string(load.loadedMaterialAssetManifestCount)
			+ ", loadedTextureAssets=" + std::to_string(load.loadedTextureAssetManifestCount)
			+ ", registeredAssetManifest=" + std::to_string(load.registeredAssetManifestCount)
			+ ", registeredMeshAssets=" + std::to_string(load.registeredMeshAssetManifestCount)
			+ ", registeredMaterialAssets=" + std::to_string(load.registeredMaterialAssetManifestCount)
			+ ", registeredTextureAssets=" + std::to_string(load.registeredTextureAssetManifestCount)
			+ ", path=" + save.path
			+ (!save.error.empty() ? ", saveError=" + save.error : std::string{})
			+ (!load.error.empty() ? ", loadError=" + load.error : std::string{});
	}

	std::string formatRuntimeAssetRegistryStats(const GLengine::AssetRegistry& registry)
	{
		int importedAssetHandles = 0;
		for (const auto& asset : registry.listAssets())
		{
			if (asset.source == "imported-asset")
			{
				++importedAssetHandles;
			}
		}

		return "Runtime asset registry stats: assets=" + std::to_string(registry.count())
			+ ", meshAssets=" + std::to_string(registry.countByKind(GLengine::AssetKind::Mesh))
			+ ", materialAssets=" + std::to_string(registry.countByKind(GLengine::AssetKind::Material))
			+ ", textureAssets=" + std::to_string(registry.countByKind(GLengine::AssetKind::Texture))
			+ ", importedAssetHandles=" + std::to_string(importedAssetHandles);
	}
}

namespace GL_RUNTIME
{
	void RuntimeImportedAssetVerification::addVerificationSceneProbes(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& verification
	)
	{
		if (!verification.pbr.probes.enablePbrImportedAssetProbe || !context.renderResources.sceneOffScreen || !context.renderResources.renderer)
		{
			return;
		}

		auto importedAsset = GL_APPLICATION::AssimpLoader::loadPBR("fbx/test/test.fbx", context.renderResources.renderer);
		if (!importedAsset)
		{
			reportLine("PBR imported asset probe failed: fbx/test/test.fbx");
			return;
		}

		importedAsset->setName("PBR Imported Asset Probe");
		importedAsset->setPosition({ -2.4f, -1.1f, 1.8f });
		importedAsset->setScale({ 0.65f, 0.65f, 0.65f });
		context.renderResources.sceneOffScreen->addChild(importedAsset);

		if (!context.engineAttachments.engineWorld || !context.engineAttachments.engineWorld->getPersistentLevel())
		{
			reportLine("PBR imported asset engine world import skipped: runtime engineWorld unavailable");
			return;
		}

		if (!context.engineAttachments.assetSubsystem)
		{
			reportLine("PBR imported asset engine world import skipped: runtime AssetSubsystem unavailable");
			return;
		}

		GLengine::LegacySceneImportOptions importOptions{};
		importOptions.persistentIdSource = GLengine::PersistentIdSource::ImportedAssetDerived;
		importOptions.persistentIdScope = "fbx-test-test-fbx";
		importOptions.assetHandleSource = "imported-asset";
		importOptions.assetHandleScope = "fbx-test-test-fbx";
		auto& assetRegistry = context.engineAttachments.assetSubsystem->getRegistry();
		assetRegistry.clear();
		importOptions.assetRegistry = &assetRegistry;
		const auto importStats = GLengine::LegacySceneWorldBuilder::importObjectTree(
			*context.engineAttachments.engineWorld->getPersistentLevel(),
			importedAsset,
			importOptions
		);
		reportLine(formatImportedAssetWorldImportStats(importStats));
		reportLine(formatRuntimeAssetRegistryStats(assetRegistry));

		const std::string packagePath = "out/pbr_import_asset_scene_package.verification.ini";
		const auto packageSave = GLengine::saveScenePackage(*context.engineAttachments.engineWorld, packagePath);
		assetRegistry.clear();
		GLengine::ScenePackageLoadOptions packageLoadOptions{};
		packageLoadOptions.assetRegistry = &assetRegistry;
		const auto packageLoad = GLengine::loadScenePackage(packagePath, packageLoadOptions);
		reportLine(formatImportedAssetScenePackageStats(packageSave, packageLoad));
		reportLine(formatRuntimeAssetRegistryStats(assetRegistry));
	}
}

#include "RuntimeEngineWorldVerification.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "RuntimeVerificationConfig.h"
#include "../engine/Actor.h"
#include "../engine/ActorAdapters.h"
#include "../engine/AssetRegistry.h"
#include "../engine/Level.h"
#include "../engine/PersistentIdPolicy.h"
#include "../engine/SceneComponent.h"
#include "../engine/ScenePackage.h"
#include "../engine/Transform.h"
#include "../engine/World.h"
#include "../engine/WorldLegacySceneExporter.h"
#include "../framework/geometry.h"
#include "../framework/object.h"
#include "../framework/scene.h"
#include "../light/pointLight.h"
#include "../materials/pbrMaterial/PBRMaterial.h"
#include "../mesh/mesh.h"
#include "../renderer/renderer.h"
#include "../tools/Logger/LogManager.h"
#include "../tools/editor/EditorWorldActions.h"
#include "../tools/editor/SceneTransformSnapshot.h"
#include "AppRuntimeContext.h"

namespace
{
	struct ScenePackageNegativeProbeStats
	{
		bool missingSchemaRejected{ false };
		bool invalidLineRejected{ false };
		bool missingCountRejected{ false };
		bool unknownTypesLoaded{ false };
		bool duplicateActorIdRejected{ false };
		bool duplicateSceneComponentIdRejected{ false };
		bool unresolvedParentRejected{ false };
		bool invalidParentIndexRejected{ false };
		bool selfParentRejected{ false };
		bool cycleRejected{ false };
		bool validCrossActorParentLoaded{ false };
		int unknownActorTypes{ 0 };
		int unknownSceneComponentTypes{ 0 };
		int invalidSceneComponentTransforms{ 0 };
		int duplicateActorPersistentIds{ 0 };
		int duplicateSceneComponentPersistentIds{ 0 };
		int unresolvedParentReferences{ 0 };
		int invalidParentSceneComponentIndices{ 0 };
		int selfParentReferences{ 0 };
		int cyclicParentReferences{ 0 };
		int restoredCrossActorParentReferences{ 0 };
		std::string directory{};
	};

	struct EngineWorldPreparedSceneStats
	{
		int engineWorldProbeMeshCount{ 0 };
		int engineWorldMinimalMeshCount{ 0 };
		int runtimeWorldActorCount{ 0 };
	};

	void collectEngineWorldPreparedSceneStats(
		const std::shared_ptr<GLframework::Object>& object,
		EngineWorldPreparedSceneStats& stats
	)
	{
		if (!object)
		{
			return;
		}

		const bool engineWorldProbe = object->getName().find("Engine World Scene Probe") == 0;
		const bool engineWorldMinimal = object->getName().find("Engine World Minimal") == 0;
		const auto mesh = std::dynamic_pointer_cast<GLframework::Mesh>(object);
		if (mesh
			&& mesh->getMaterial()
			&& mesh->getMaterial()->getMaterialType() == GLframework::MaterialType::PBRMaterial)
		{
			if (engineWorldProbe)
			{
				++stats.engineWorldProbeMeshCount;
			}
			if (engineWorldMinimal)
			{
				++stats.engineWorldMinimalMeshCount;
			}
		}

		for (const auto& child : object->getChildren())
		{
			collectEngineWorldPreparedSceneStats(child, stats);
		}
	}

	bool perturbFirstRuntimeSceneComponent(const GLframework::AppRuntimeContext& context)
	{
		if (!context.engineAttachments.engineWorld || !context.engineAttachments.engineWorld->getPersistentLevel())
		{
			return false;
		}

		for (const auto& actor : context.engineAttachments.engineWorld->getPersistentLevel()->getActors())
		{
			if (!actor)
			{
				continue;
			}

			for (const auto& component : actor->getComponents())
			{
				auto* sceneComponent = dynamic_cast<GLengine::SceneComponent*>(component.get());
				if (!sceneComponent)
				{
					continue;
				}

				auto transform = sceneComponent->getRelativeTransform();
				transform.location.x += 0.125f;
				sceneComponent->setRelativeTransform(transform);
				return true;
			}
		}

		return false;
	}

	GLengine::SceneComponent* findFirstRootSceneComponent(const GLframework::AppRuntimeContext& context)
	{
		if (!context.engineAttachments.engineWorld || !context.engineAttachments.engineWorld->getPersistentLevel())
		{
			return nullptr;
		}

		for (const auto& actor : context.engineAttachments.engineWorld->getPersistentLevel()->getActors())
		{
			if (!actor)
			{
				continue;
			}

			auto* rootComponent = actor->getRootComponent();
			if (rootComponent && rootComponent->getParent() == nullptr)
			{
				return rootComponent;
			}
		}

		return nullptr;
	}

	bool addScenePackageLightProbe(GLframework::AppRuntimeContext& context)
	{
		if (!context.engineAttachments.engineWorld || !context.engineAttachments.engineWorld->getPersistentLevel())
		{
			return false;
		}

		auto* parentComponent = findFirstRootSceneComponent(context);
		auto pointLight = std::make_shared<GLframework::PointLight>();
		pointLight->setName("Engine World Package Point Light");
		pointLight->setColor({ 1.0f, 0.78f, 0.45f });
		pointLight->setIntensity(1.8f);
		pointLight->setSpecularIntensity(1.0f);
		pointLight->setK(0.09f, 0.07f, 1.0f);

		auto& lightActor = context.engineAttachments.engineWorld->getPersistentLevel()->spawnActor<GLengine::LightActor>(
			pointLight,
			"Engine World Package Light Actor"
		);
		lightActor.setPersistentId(GLengine::makePersistentId(
			"actor",
			GLengine::PersistentIdSource::PresetAssigned,
			"engine-world-scene-package",
			{ "light" }
		));

		auto* lightComponent = lightActor.getRootComponent();
		if (!lightComponent)
		{
			return false;
		}

		lightComponent->setName("Engine World Package Light Component");
		lightComponent->setPersistentId(GLengine::makePersistentId(
			"component",
			GLengine::PersistentIdSource::PresetAssigned,
			"engine-world-scene-package",
			{ "light" }
		));
		GLengine::Transform lightTransform{};
		lightTransform.location = { 0.0f, 1.4f, 0.65f };
		lightComponent->setRelativeTransform(lightTransform);
		if (parentComponent)
		{
			lightComponent->attachTo(parentComponent);
		}
		return true;
	}

	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}

	std::string yesNo(bool value)
	{
		return value ? "yes" : "no";
	}

	bool writeVerificationTextFile(const std::string& path, const std::string& contents)
	{
		const std::filesystem::path filePath{ path };
		const auto parentPath = filePath.parent_path();
		if (!parentPath.empty())
		{
			std::error_code filesystemError{};
			std::filesystem::create_directories(parentPath, filesystemError);
			if (filesystemError)
			{
				return false;
			}
		}

		std::ofstream output(path, std::ios::trunc);
		if (!output)
		{
			return false;
		}

		output << contents;
		return static_cast<bool>(output);
	}

	bool scenePackageLoadRejected(const std::string& path)
	{
		const auto load = GLengine::loadScenePackage(path);
		return !load.loaded && !load.error.empty();
	}

	ScenePackageNegativeProbeStats runScenePackageNegativeProbes()
	{
		ScenePackageNegativeProbeStats stats{};
		stats.directory = "out/engine_world_scene_package_negative";

		const std::string missingSchemaPath = stats.directory + "/missing_schema.ini";
		if (writeVerificationTextFile(
			missingSchemaPath,
			"actorCount=0\n"
			"sceneComponentCount=0\n"
		))
		{
			stats.missingSchemaRejected = scenePackageLoadRejected(missingSchemaPath);
		}

		const std::string invalidLinePath = stats.directory + "/invalid_line.ini";
		if (writeVerificationTextFile(
			invalidLinePath,
			"schema=engine.world.scenePackage.v1\n"
			"this line has no separator\n"
			"actorCount=0\n"
			"sceneComponentCount=0\n"
		))
		{
			stats.invalidLineRejected = scenePackageLoadRejected(invalidLinePath);
		}

		const std::string missingCountPath = stats.directory + "/missing_count.ini";
		if (writeVerificationTextFile(
			missingCountPath,
			"schema=engine.world.scenePackage.v1\n"
			"actorCount=1\n"
		))
		{
			stats.missingCountRejected = scenePackageLoadRejected(missingCountPath);
		}

		const std::string unknownTypesPath = stats.directory + "/unknown_types.ini";
		if (writeVerificationTextFile(
			unknownTypesPath,
			"schema=engine.world.scenePackage.v1\n"
			"world.name=Negative Probe World\n"
			"level.name=Negative Probe Level\n"
			"actorCount=1\n"
			"sceneComponentCount=1\n"
			"actor.0.name=Unknown Actor\n"
			"actor.0.type=FutureActor\n"
			"actor.0.sceneComponentCount=1\n"
			"actor.0.rootSceneComponent=0\n"
			"actor.0.sceneComponent.0.name=Unknown Component\n"
			"actor.0.sceneComponent.0.type=FutureComponent\n"
			"actor.0.sceneComponent.0.adapter.kind=none\n"
			"actor.0.sceneComponent.0.adapter.assetHandle=none\n"
			"actor.0.sceneComponent.0.adapter.assetReference=none\n"
			"actor.0.sceneComponent.0.parentSceneComponent=-1\n"
			"actor.0.sceneComponent.0.locationX=not-a-number\n"
			"actor.0.sceneComponent.0.locationY=0\n"
			"actor.0.sceneComponent.0.locationZ=0\n"
			"actor.0.sceneComponent.0.rotationX=0\n"
			"actor.0.sceneComponent.0.rotationY=0\n"
			"actor.0.sceneComponent.0.rotationZ=0\n"
			"actor.0.sceneComponent.0.scaleX=1\n"
			"actor.0.sceneComponent.0.scaleY=1\n"
			"actor.0.sceneComponent.0.scaleZ=1\n"
		))
		{
			const auto load = GLengine::loadScenePackage(unknownTypesPath);
			stats.unknownTypesLoaded = load.loaded;
			stats.unknownActorTypes = load.unknownActorTypes;
			stats.unknownSceneComponentTypes = load.unknownSceneComponentTypes;
			stats.invalidSceneComponentTransforms = load.invalidSceneComponentTransforms;
		}

		const std::string duplicateActorIdsPath = stats.directory + "/duplicate_actor_ids.ini";
		if (writeVerificationTextFile(
			duplicateActorIdsPath,
			"schema=engine.world.scenePackage.v1\n"
			"actorCount=2\n"
			"sceneComponentCount=2\n"
			"actor.0.name=Actor A\n"
			"actor.0.type=Actor\n"
			"actor.0.persistentId=actor:preset:duplicate\n"
			"actor.0.sceneComponentCount=1\n"
			"actor.0.rootSceneComponent=0\n"
			"actor.0.sceneComponent.0.name=Root A\n"
			"actor.0.sceneComponent.0.type=SceneComponent\n"
			"actor.0.sceneComponent.0.persistentId=component:preset:a-root\n"
			"actor.0.sceneComponent.0.parentSceneComponent=-1\n"
			"actor.1.name=Actor B\n"
			"actor.1.type=Actor\n"
			"actor.1.persistentId=actor:preset:duplicate\n"
			"actor.1.sceneComponentCount=1\n"
			"actor.1.rootSceneComponent=0\n"
			"actor.1.sceneComponent.0.name=Root B\n"
			"actor.1.sceneComponent.0.type=SceneComponent\n"
			"actor.1.sceneComponent.0.persistentId=component:preset:b-root\n"
			"actor.1.sceneComponent.0.parentSceneComponent=-1\n"
		))
		{
			const auto load = GLengine::loadScenePackage(duplicateActorIdsPath);
			stats.duplicateActorIdRejected = !load.loaded && load.duplicateActorPersistentIds >= 1;
			stats.duplicateActorPersistentIds = load.duplicateActorPersistentIds;
		}

		const std::string duplicateComponentIdsPath = stats.directory + "/duplicate_component_ids.ini";
		if (writeVerificationTextFile(
			duplicateComponentIdsPath,
			"schema=engine.world.scenePackage.v1\n"
			"actorCount=1\n"
			"sceneComponentCount=2\n"
			"actor.0.name=Actor\n"
			"actor.0.type=Actor\n"
			"actor.0.persistentId=actor:preset:component-duplicate-owner\n"
			"actor.0.sceneComponentCount=2\n"
			"actor.0.rootSceneComponent=0\n"
			"actor.0.sceneComponent.0.name=Root A\n"
			"actor.0.sceneComponent.0.type=SceneComponent\n"
			"actor.0.sceneComponent.0.persistentId=component:preset:duplicate\n"
			"actor.0.sceneComponent.0.parentSceneComponent=-1\n"
			"actor.0.sceneComponent.1.name=Root B\n"
			"actor.0.sceneComponent.1.type=SceneComponent\n"
			"actor.0.sceneComponent.1.persistentId=component:preset:duplicate\n"
			"actor.0.sceneComponent.1.parentSceneComponent=-1\n"
		))
		{
			const auto load = GLengine::loadScenePackage(duplicateComponentIdsPath);
			stats.duplicateSceneComponentIdRejected = !load.loaded && load.duplicateSceneComponentPersistentIds >= 1;
			stats.duplicateSceneComponentPersistentIds = load.duplicateSceneComponentPersistentIds;
		}

		const std::string unresolvedParentPath = stats.directory + "/unresolved_parent.ini";
		if (writeVerificationTextFile(
			unresolvedParentPath,
			"schema=engine.world.scenePackage.v1\n"
			"actorCount=1\n"
			"sceneComponentCount=1\n"
			"actor.0.name=Actor\n"
			"actor.0.type=Actor\n"
			"actor.0.persistentId=actor:preset:unresolved-parent\n"
			"actor.0.sceneComponentCount=1\n"
			"actor.0.rootSceneComponent=0\n"
			"actor.0.sceneComponent.0.name=Root\n"
			"actor.0.sceneComponent.0.type=SceneComponent\n"
			"actor.0.sceneComponent.0.persistentId=component:preset:unresolved-parent-root\n"
			"actor.0.sceneComponent.0.parentPersistentId=component:preset:missing-parent\n"
			"actor.0.sceneComponent.0.parentSceneComponent=-1\n"
		))
		{
			const auto load = GLengine::loadScenePackage(unresolvedParentPath);
			stats.unresolvedParentRejected = !load.loaded && load.unresolvedParentReferences >= 1;
			stats.unresolvedParentReferences = load.unresolvedParentReferences;
		}

		const std::string invalidParentIndexPath = stats.directory + "/invalid_parent_index.ini";
		if (writeVerificationTextFile(
			invalidParentIndexPath,
			"schema=engine.world.scenePackage.v1\n"
			"actorCount=1\n"
			"sceneComponentCount=1\n"
			"actor.0.name=Actor\n"
			"actor.0.type=Actor\n"
			"actor.0.persistentId=actor:preset:invalid-parent-index\n"
			"actor.0.sceneComponentCount=1\n"
			"actor.0.rootSceneComponent=0\n"
			"actor.0.sceneComponent.0.name=Root\n"
			"actor.0.sceneComponent.0.type=SceneComponent\n"
			"actor.0.sceneComponent.0.persistentId=component:preset:invalid-parent-index-root\n"
			"actor.0.sceneComponent.0.parentSceneComponent=99\n"
		))
		{
			const auto load = GLengine::loadScenePackage(invalidParentIndexPath);
			stats.invalidParentIndexRejected = !load.loaded && load.invalidParentSceneComponentIndices >= 1;
			stats.invalidParentSceneComponentIndices = load.invalidParentSceneComponentIndices;
		}

		const std::string selfParentPath = stats.directory + "/self_parent.ini";
		if (writeVerificationTextFile(
			selfParentPath,
			"schema=engine.world.scenePackage.v1\n"
			"actorCount=1\n"
			"sceneComponentCount=1\n"
			"actor.0.name=Actor\n"
			"actor.0.type=Actor\n"
			"actor.0.persistentId=actor:preset:self-parent\n"
			"actor.0.sceneComponentCount=1\n"
			"actor.0.rootSceneComponent=0\n"
			"actor.0.sceneComponent.0.name=Root\n"
			"actor.0.sceneComponent.0.type=SceneComponent\n"
			"actor.0.sceneComponent.0.persistentId=component:preset:self-parent-root\n"
			"actor.0.sceneComponent.0.parentPersistentId=component:preset:self-parent-root\n"
			"actor.0.sceneComponent.0.parentSceneComponent=-1\n"
		))
		{
			const auto load = GLengine::loadScenePackage(selfParentPath);
			stats.selfParentRejected = !load.loaded && load.selfParentReferences >= 1;
			stats.selfParentReferences = load.selfParentReferences;
		}

		const std::string cyclePath = stats.directory + "/cycle.ini";
		if (writeVerificationTextFile(
			cyclePath,
			"schema=engine.world.scenePackage.v1\n"
			"actorCount=1\n"
			"sceneComponentCount=2\n"
			"actor.0.name=Actor\n"
			"actor.0.type=Actor\n"
			"actor.0.persistentId=actor:preset:cycle\n"
			"actor.0.sceneComponentCount=2\n"
			"actor.0.rootSceneComponent=0\n"
			"actor.0.sceneComponent.0.name=A\n"
			"actor.0.sceneComponent.0.type=SceneComponent\n"
			"actor.0.sceneComponent.0.persistentId=component:preset:cycle-a\n"
			"actor.0.sceneComponent.0.parentPersistentId=component:preset:cycle-b\n"
			"actor.0.sceneComponent.1.name=B\n"
			"actor.0.sceneComponent.1.type=SceneComponent\n"
			"actor.0.sceneComponent.1.persistentId=component:preset:cycle-b\n"
			"actor.0.sceneComponent.1.parentPersistentId=component:preset:cycle-a\n"
		))
		{
			const auto load = GLengine::loadScenePackage(cyclePath);
			stats.cycleRejected = !load.loaded && load.cyclicParentReferences >= 1;
			stats.cyclicParentReferences = load.cyclicParentReferences;
		}

		const std::string validCrossActorPath = stats.directory + "/valid_cross_actor_parent.ini";
		if (writeVerificationTextFile(
			validCrossActorPath,
			"schema=engine.world.scenePackage.v1\n"
			"actorCount=2\n"
			"sceneComponentCount=2\n"
			"actor.0.name=Parent Actor\n"
			"actor.0.type=Actor\n"
			"actor.0.persistentId=actor:preset:cross-parent\n"
			"actor.0.sceneComponentCount=1\n"
			"actor.0.rootSceneComponent=0\n"
			"actor.0.sceneComponent.0.name=Parent Root\n"
			"actor.0.sceneComponent.0.type=SceneComponent\n"
			"actor.0.sceneComponent.0.persistentId=component:preset:cross-parent-root\n"
			"actor.0.sceneComponent.0.parentSceneComponent=-1\n"
			"actor.1.name=Child Actor\n"
			"actor.1.type=Actor\n"
			"actor.1.persistentId=actor:preset:cross-child\n"
			"actor.1.sceneComponentCount=1\n"
			"actor.1.rootSceneComponent=0\n"
			"actor.1.sceneComponent.0.name=Child Root\n"
			"actor.1.sceneComponent.0.type=SceneComponent\n"
			"actor.1.sceneComponent.0.persistentId=component:preset:cross-child-root\n"
			"actor.1.sceneComponent.0.parentPersistentId=component:preset:cross-parent-root\n"
			"actor.1.sceneComponent.0.parentSceneComponent=-1\n"
		))
		{
			const auto load = GLengine::loadScenePackage(validCrossActorPath);
			stats.validCrossActorParentLoaded = load.loaded && load.restoredCrossActorParentReferences >= 1;
			stats.restoredCrossActorParentReferences = load.restoredCrossActorParentReferences;
		}

		return stats;
	}

	std::string formatScenePackageNegativeProbeStats(const ScenePackageNegativeProbeStats& stats)
	{
		return "Engine world scene package negative stats: missingSchemaRejected=" + yesNo(stats.missingSchemaRejected)
			+ ", invalidLineRejected=" + yesNo(stats.invalidLineRejected)
			+ ", missingCountRejected=" + yesNo(stats.missingCountRejected)
			+ ", unknownTypesLoaded=" + yesNo(stats.unknownTypesLoaded)
			+ ", duplicateActorIdRejected=" + yesNo(stats.duplicateActorIdRejected)
			+ ", duplicateSceneComponentIdRejected=" + yesNo(stats.duplicateSceneComponentIdRejected)
			+ ", unresolvedParentRejected=" + yesNo(stats.unresolvedParentRejected)
			+ ", invalidParentIndexRejected=" + yesNo(stats.invalidParentIndexRejected)
			+ ", selfParentRejected=" + yesNo(stats.selfParentRejected)
			+ ", cycleRejected=" + yesNo(stats.cycleRejected)
			+ ", validCrossActorParentLoaded=" + yesNo(stats.validCrossActorParentLoaded)
			+ ", unknownActorTypes=" + std::to_string(stats.unknownActorTypes)
			+ ", unknownSceneComponentTypes=" + std::to_string(stats.unknownSceneComponentTypes)
			+ ", invalidSceneComponentTransforms=" + std::to_string(stats.invalidSceneComponentTransforms)
			+ ", duplicateActorPersistentIds=" + std::to_string(stats.duplicateActorPersistentIds)
			+ ", duplicateSceneComponentPersistentIds=" + std::to_string(stats.duplicateSceneComponentPersistentIds)
			+ ", unresolvedParentReferences=" + std::to_string(stats.unresolvedParentReferences)
			+ ", invalidParentSceneComponentIndices=" + std::to_string(stats.invalidParentSceneComponentIndices)
			+ ", selfParentReferences=" + std::to_string(stats.selfParentReferences)
			+ ", cyclicParentReferences=" + std::to_string(stats.cyclicParentReferences)
			+ ", restoredCrossActorParentReferences=" + std::to_string(stats.restoredCrossActorParentReferences)
			+ ", path=" + stats.directory;
	}

	std::string formatScenePackageRoundTripStats(
		const GLengine::ScenePackageSaveResult& save,
		const GLengine::ScenePackageLoadResult& load,
		const GLengine::WorldLegacySceneExportStats& loadedWorldExport
	)
	{
		return "Engine world scene package stats: saved=" + yesNo(save.saved)
			+ ", loaded=" + yesNo(load.loaded)
			+ ", actors=" + std::to_string(save.actorCount)
			+ ", sceneComponents=" + std::to_string(save.sceneComponentCount)
			+ ", typedActors=" + std::to_string(save.typedActorCount)
			+ ", typedSceneComponents=" + std::to_string(save.typedSceneComponentCount)
			+ ", adapterDescriptors=" + std::to_string(save.adapterDescriptorCount)
			+ ", assetHandles=" + std::to_string(save.assetHandleCount)
			+ ", loadedActors=" + std::to_string(load.actorCount)
			+ ", loadedSceneComponents=" + std::to_string(load.sceneComponentCount)
			+ ", loadedTypedActors=" + std::to_string(load.loadedTypedActors)
			+ ", loadedActorsWithPersistentIds=" + std::to_string(load.actorsWithPersistentIds)
			+ ", loadedSceneComponentsWithPersistentIds=" + std::to_string(load.sceneComponentsWithPersistentIds)
			+ ", loadedTypedSceneComponents=" + std::to_string(load.loadedTypedSceneComponents)
			+ ", unknownActorTypes=" + std::to_string(load.unknownActorTypes)
			+ ", unknownSceneComponentTypes=" + std::to_string(load.unknownSceneComponentTypes)
			+ ", invalidSceneComponentTransforms=" + std::to_string(load.invalidSceneComponentTransforms)
			+ ", duplicateActorPersistentIds=" + std::to_string(load.duplicateActorPersistentIds)
			+ ", duplicateSceneComponentPersistentIds=" + std::to_string(load.duplicateSceneComponentPersistentIds)
			+ ", invalidRootSceneComponentIndices=" + std::to_string(load.invalidRootSceneComponentIndices)
			+ ", unresolvedParentReferences=" + std::to_string(load.unresolvedParentReferences)
			+ ", invalidParentSceneComponentIndices=" + std::to_string(load.invalidParentSceneComponentIndices)
			+ ", selfParentReferences=" + std::to_string(load.selfParentReferences)
			+ ", cyclicParentReferences=" + std::to_string(load.cyclicParentReferences)
			+ ", loadedAdapterReferences=" + std::to_string(load.loadedAdapterReferences)
			+ ", loadedAssetHandles=" + std::to_string(load.loadedAssetHandles)
			+ ", resolvedAdapterReferences=" + std::to_string(load.resolvedAdapterReferences)
			+ ", resolvedAssetHandles=" + std::to_string(load.resolvedAssetHandles)
			+ ", unresolvedAdapterReferences=" + std::to_string(load.unresolvedAdapterReferences)
			+ ", unresolvedAssetHandles=" + std::to_string(load.unresolvedAssetHandles)
			+ ", restoredRootComponents=" + std::to_string(load.restoredRootComponents)
			+ ", restoredSceneComponentParents=" + std::to_string(load.restoredSceneComponentParents)
			+ ", restoredCrossActorParentReferences=" + std::to_string(load.restoredCrossActorParentReferences)
			+ ", exportedLoadedWorldObjects=" + std::to_string(loadedWorldExport.exportedObjects)
			+ ", exportedLoadedWorldMeshes=" + std::to_string(loadedWorldExport.exportedMeshes)
			+ ", exportedLoadedWorldLights=" + std::to_string(loadedWorldExport.exportedLights)
			+ ", exportedLoadedWorldLegacyObjects=" + std::to_string(loadedWorldExport.exportedLegacyObjects)
			+ ", exportedLoadedWorldAttachments=" + std::to_string(loadedWorldExport.objectAttachments)
			+ ", path=" + save.path
			+ (!save.error.empty() ? ", saveError=" + save.error : std::string{})
			+ (!load.error.empty() ? ", loadError=" + load.error : std::string{});
	}

	std::string formatEditorCreatedActorStats(const GL_EDITOR::EditorCreatedActorResult& result)
	{
		return "Engine world editor create stats: created=" + yesNo(result.created)
			+ ", editorCreatedActors=" + std::to_string(result.editorCreatedActorCount)
			+ ", editorCreatedSceneComponents=" + std::to_string(result.editorCreatedSceneComponentCount)
			+ ", actorPersistentId=" + (result.actorPersistentId.empty() ? std::string{ "none" } : result.actorPersistentId)
			+ ", componentPersistentId=" + (result.rootComponentPersistentId.empty() ? std::string{ "none" } : result.rootComponentPersistentId)
			+ (result.error.empty() ? std::string{} : ", error=" + result.error);
	}

	std::string formatEngineWorldPreparedSceneStats(const EngineWorldPreparedSceneStats& stats)
	{
		return "Engine world prepared scene stats: engineWorldProbeMeshes="
			+ std::to_string(stats.engineWorldProbeMeshCount)
			+ ", engineWorldMinimalMeshes=" + std::to_string(stats.engineWorldMinimalMeshCount)
			+ ", runtimeWorldActors=" + std::to_string(stats.runtimeWorldActorCount);
	}

	std::shared_ptr<GLframework::PBRMaterial> createResolvedScenePackagePbrMaterial(
		const GLengine::ScenePackageAdapterDescriptor& descriptor
	)
	{
		auto material = std::make_shared<GLframework::PBRMaterial>();
		material->mAlbedo = { 0.85f, 0.85f, 0.85f };
		material->mMetallic = 0.0f;
		material->mRoughness = 0.5f;
		material->mAo = 1.0f;
		material->mUseIBL = true;

		if (descriptor.objectName.find("Matte") != std::string::npos)
		{
			material->mAlbedo = { 0.92f, 0.38f, 0.18f };
			material->mMetallic = 0.0f;
			material->mRoughness = 0.72f;
		}
		else if (descriptor.objectName.find("Metallic") != std::string::npos)
		{
			material->mAlbedo = { 1.0f, 0.78f, 0.28f };
			material->mMetallic = 1.0f;
			material->mRoughness = 0.22f;
			material->mIblSpecularStrength = 1.25f;
		}
		else if (descriptor.objectName.find("Gloss") != std::string::npos)
		{
			material->mAlbedo = { 0.16f, 0.62f, 1.0f };
			material->mMetallic = 0.0f;
			material->mRoughness = 0.16f;
			material->mIblSpecularStrength = 1.3f;
		}
		else if (descriptor.objectName.find("Emissive") != std::string::npos)
		{
			material->mAlbedo = { 0.02f, 0.02f, 0.02f };
			material->mMetallic = 0.0f;
			material->mRoughness = 0.95f;
			material->mUseIBL = false;
			material->mIblDiffuseStrength = 0.0f;
			material->mIblSpecularStrength = 0.0f;
			material->mEmissiveColor = { 0.25f, 0.95f, 0.8f };
			material->mEmissiveIntensity = 2.4f;
		}

		return material;
	}

	class RuntimeScenePackageAssetResolver : public GLengine::ScenePackageAssetResolver
	{
	public:
		explicit RuntimeScenePackageAssetResolver(std::shared_ptr<GLframework::Renderer> renderer)
			: mRenderer(std::move(renderer))
		{
		}

		std::shared_ptr<GLframework::Mesh> resolveMesh(
			const GLengine::ScenePackageAdapterDescriptor& descriptor
		) override
		{
			if (!mRenderer
				|| descriptor.adapterKind != "mesh"
				|| (!GLengine::assetHandleMatches(
						descriptor.assetHandle,
						GLengine::AssetKind::Mesh,
						"runtime-generated"
					)
					&& !GLengine::assetHandleMatches(
						descriptor.assetReference,
						GLengine::AssetKind::Mesh,
						"runtime-generated"
					)
					&& descriptor.assetReference.find("runtime-generated:mesh:") != 0))
			{
				return nullptr;
			}

			auto geometry = GLframework::Geometry::createSphere(
				mRenderer->getShader(GLframework::MaterialType::PBRMaterial),
				0.36f,
				40,
				20
			);
			auto mesh = std::make_shared<GLframework::Mesh>(
				geometry,
				createResolvedScenePackagePbrMaterial(descriptor)
			);
			mesh->setName(descriptor.objectName.empty() ? descriptor.assetReference : descriptor.objectName);
			return mesh;
		}

		std::shared_ptr<GLframework::Object> resolveLegacyObject(
			const GLengine::ScenePackageAdapterDescriptor& descriptor
		) override
		{
			if (descriptor.adapterKind != "legacy-object"
				|| (!GLengine::assetHandleMatches(
						descriptor.assetHandle,
						GLengine::AssetKind::LegacyObject,
						"runtime-generated"
					)
					&& !GLengine::assetHandleMatches(
						descriptor.assetReference,
						GLengine::AssetKind::LegacyObject,
						"runtime-generated"
					)
					&& descriptor.assetReference.find("runtime-generated:legacy-object:") != 0))
			{
				return nullptr;
			}

			auto object = std::make_shared<GLframework::Object>();
			object->setName(descriptor.objectName.empty() ? "Resolved Legacy Object" : descriptor.objectName);
			return object;
		}

		std::shared_ptr<GLframework::Light> resolveLight(
			const GLengine::ScenePackageAdapterDescriptor& descriptor
		) override
		{
			if (descriptor.adapterKind != "light"
				|| (!GLengine::assetHandleMatches(
						descriptor.assetHandle,
						GLengine::AssetKind::Light,
						"runtime-generated"
					)
					&& !GLengine::assetHandleMatches(
						descriptor.assetReference,
						GLengine::AssetKind::Light,
						"runtime-generated"
					)
					&& descriptor.assetReference.find("runtime-generated:light:") != 0))
			{
				return nullptr;
			}

			auto pointLight = std::make_shared<GLframework::PointLight>();
			pointLight->setName(descriptor.objectName.empty() ? "Resolved Package Point Light" : descriptor.objectName);
			pointLight->setColor({ 1.0f, 0.78f, 0.45f });
			pointLight->setIntensity(1.8f);
			pointLight->setSpecularIntensity(1.0f);
			pointLight->setK(0.09f, 0.07f, 1.0f);
			return pointLight;
		}

	private:
		std::shared_ptr<GLframework::Renderer> mRenderer{ nullptr };
	};
}

namespace GL_RUNTIME
{
	int RuntimeEngineWorldVerification::countRuntimeWorldActors(const GLframework::AppRuntimeContext& context)
	{
		if (!context.engineAttachments.engineWorld || !context.engineAttachments.engineWorld->getPersistentLevel())
		{
			return 0;
		}

		return static_cast<int>(context.engineAttachments.engineWorld->getPersistentLevel()->getActors().size());
	}

	void RuntimeEngineWorldVerification::addVerificationSceneProbes(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& verification
	)
	{
		const auto& engineWorld = verification.engineWorld;
		if (!engineWorld.enableEditorCreate)
		{
			return;
		}

		if (context.engineAttachments.engineWorld && context.engineAttachments.engineWorldEditable)
		{
			GL_EDITOR::EditorCreateActorOptions options{};
			options.actorName = "Editor Created Verification Actor";
			options.rootComponentName = "Editor Created Verification Root";
			options.transform.location = { 0.0f, 0.55f, 0.0f };
			const auto result = GL_EDITOR::createEditorEmptyActor(*context.engineAttachments.engineWorld, options);
			reportLine(formatEditorCreatedActorStats(result));
		}
		else
		{
			GL_EDITOR::EditorCreatedActorResult result{};
			result.error = "runtime engineWorld is not editable";
			reportLine(formatEditorCreatedActorStats(result));
		}
	}

	void RuntimeEngineWorldVerification::reportPreparedScene(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& verification
	)
	{
		const auto& engineWorld = verification.engineWorld;
		EngineWorldPreparedSceneStats stats{};
		collectEngineWorldPreparedSceneStats(
			std::static_pointer_cast<GLframework::Object>(context.renderResources.sceneOffScreen()),
			stats
		);
		stats.runtimeWorldActorCount = countRuntimeWorldActors(context);
		reportLine(formatEngineWorldPreparedSceneStats(stats));

		if (context.engineAttachments.engineWorld && context.engineAttachments.engineWorldEditable)
		{
			if (engineWorld.enableScenePackageRoundTrip)
			{
				addScenePackageLightProbe(context);
			}

			const std::string snapshotPath = "out/engine_world_transform_snapshot.verification.ini";
			const auto snapshot = GL_EDITOR::saveSceneTransformSnapshot(
				*context.engineAttachments.engineWorld,
				snapshotPath
			);
			reportLine(
				"Engine world transform snapshot stats: saved="
				+ yesNo(snapshot.saved)
				+ ", actors=" + std::to_string(snapshot.actorCount)
				+ ", sceneComponents=" + std::to_string(snapshot.sceneComponentCount)
				+ ", path=" + snapshot.path
				+ (snapshot.error.empty() ? std::string{} : ", error=" + snapshot.error)
			);

			const bool perturbed = perturbFirstRuntimeSceneComponent(context);
			const auto apply = GL_EDITOR::applySceneTransformSnapshot(*context.engineAttachments.engineWorld, snapshotPath);
			reportLine(
				"Engine world transform snapshot apply stats: applied="
				+ yesNo(apply.applied)
				+ ", snapshotActors=" + std::to_string(apply.snapshotActorCount)
				+ ", snapshotSceneComponents=" + std::to_string(apply.snapshotSceneComponentCount)
				+ ", matchedSceneComponents=" + std::to_string(apply.matchedSceneComponentCount)
				+ ", matchedByPersistentId=" + std::to_string(apply.matchedByPersistentIdCount)
				+ ", matchedByStablePath=" + std::to_string(apply.matchedByStablePathCount)
				+ ", matchedByObjectId=" + std::to_string(apply.matchedByObjectIdCount)
				+ ", matchedByIndex=" + std::to_string(apply.matchedByIndexCount)
				+ ", appliedSceneComponents=" + std::to_string(apply.appliedSceneComponentCount)
				+ ", changedSceneComponents=" + std::to_string(apply.changedSceneComponentCount)
				+ ", verificationPerturbed=" + yesNo(perturbed)
				+ ", path=" + apply.path
				+ (apply.error.empty() ? std::string{} : ", error=" + apply.error)
			);

			if (engineWorld.enableScenePackageRoundTrip)
			{
				const std::string packagePath = "out/engine_world_scene_package.verification.ini";
				const auto packageSave = GLengine::saveScenePackage(*context.engineAttachments.engineWorld, packagePath);
				RuntimeScenePackageAssetResolver resolver(context.renderResources.renderer());
				GLengine::ScenePackageLoadOptions loadOptions{};
				loadOptions.assetResolver = &resolver;
				const auto packageLoad = GLengine::loadScenePackage(packagePath, loadOptions);
				GLengine::WorldLegacySceneExportStats loadedWorldExport{};
				if (packageLoad.world)
				{
					auto loadedScene = std::make_shared<GLframework::Scene>();
					loadedWorldExport = GLengine::WorldLegacySceneExporter::exportWorldToScene(
						*packageLoad.world,
						loadedScene
					);
				}
				reportLine(formatScenePackageRoundTripStats(packageSave, packageLoad, loadedWorldExport));
				reportLine(formatScenePackageNegativeProbeStats(runScenePackageNegativeProbes()));
			}
		}
		else if (engineWorld.enableScenePackageRoundTrip)
		{
			GLengine::ScenePackageSaveResult packageSave{};
			packageSave.error = "runtime engineWorld is not editable";
			GLengine::ScenePackageLoadResult packageLoad{};
			packageLoad.error = "runtime engineWorld is not editable";
			GLengine::WorldLegacySceneExportStats loadedWorldExport{};
			reportLine(formatScenePackageRoundTripStats(packageSave, packageLoad, loadedWorldExport));
			reportLine(formatScenePackageNegativeProbeStats(runScenePackageNegativeProbes()));
		}
	}
}

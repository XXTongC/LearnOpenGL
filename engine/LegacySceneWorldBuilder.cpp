#include "engine/LegacySceneWorldBuilder.h"

#include <memory>
#include <string>
#include <unordered_map>

#include "engine/Actor.h"
#include "engine/ActorAdapters.h"
#include "engine/AssetRegistry.h"
#include "engine/Level.h"
#include "engine/PersistentIdPolicy.h"
#include "engine/SceneComponent.h"
#include "engine/Transform.h"
#include "engine/World.h"
#include "framework/object.h"
#include "framework/scene.h"
#include "light/light.h"
#include "materials/material.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "mesh/mesh.h"

using namespace GLengine;

namespace
{
	std::string objectTypeName(GLframework::ObjectType type)
	{
		switch (type)
		{
		case GLframework::ObjectType::Mesh:
			return "mesh";
		case GLframework::ObjectType::Scene:
			return "scene";
		case GLframework::ObjectType::InstancedMesh:
			return "instanced-mesh";
		case GLframework::ObjectType::Light:
			return "light";
		case GLframework::ObjectType::Object:
		default:
			return "object";
		}
	}

	std::string makeLegacyObjectKey(const std::shared_ptr<GLframework::Object>& object)
	{
		if (!object)
		{
			return "null";
		}

		return objectTypeName(object->getType()) + ":" + object->getName();
	}

	std::string makeLegacyObjectPathSegment(
		const std::shared_ptr<GLframework::Object>& object,
		int occurrence
	)
	{
		const std::string type = object ? objectTypeName(object->getType()) : "object";
		const std::string name = object ? object->getName() : std::string{};
		return makeIndexedPersistentIdPathSegment(type, name, occurrence);
	}

	std::string makeActorName(const std::shared_ptr<GLframework::Object>& object, const char* fallback)
	{
		if (!object)
		{
			return fallback;
		}

		const std::string name = object->getName();
		return name.empty() ? fallback : name;
	}

	std::string materialTypeName(GLframework::MaterialType type)
	{
		switch (type)
		{
		case GLframework::MaterialType::PhongMaterial:
			return "PhongMaterial";
		case GLframework::MaterialType::PBRMaterial:
			return "PBRMaterial";
		case GLframework::MaterialType::WhiteMaterial:
			return "WhiteMaterial";
		case GLframework::MaterialType::DepthMaterial:
			return "DepthMaterial";
		case GLframework::MaterialType::OpacityMaskMaterial:
			return "OpacityMaskMaterial";
		case GLframework::MaterialType::ScreenMaterial:
			return "ScreenMaterial";
		case GLframework::MaterialType::CubeMaterial:
			return "CubeMaterial";
		case GLframework::MaterialType::CubeSphereMaterial:
			return "CubeSphereMaterial";
		case GLframework::MaterialType::PhongEnvMaterial:
			return "PhongEnvMaterial";
		case GLframework::MaterialType::PhongEnvSphereMaterial:
			return "PhongEnvSphereMaterial";
		case GLframework::MaterialType::PhongInstanceMaterial:
			return "PhongInstanceMaterial";
		case GLframework::MaterialType::PhongNormalMaterial:
			return "PhongNormalMaterial";
		case GLframework::MaterialType::GrassInstanceMaterial:
			return "GrassInstanceMaterial";
		case GLframework::MaterialType::PhongParallaxMaterial:
			return "PhongParallaxMaterial";
		case GLframework::MaterialType::PhongShadowMaterial:
			return "PhongShadowMaterial";
		case GLframework::MaterialType::PhongCSMShadowMaterial:
			return "PhongCSMShadowMaterial";
		case GLframework::MaterialType::PhongPointShadowMaterial:
			return "PhongPointShadowMaterial";
		}
		return "Unknown";
	}

	std::string assetScopeOrPersistentScope(const LegacySceneImportOptions& options)
	{
		return options.assetHandleScope.empty()
			? options.persistentIdScope
			: options.assetHandleScope;
	}

	bool registerAssetHandle(
		const LegacySceneImportOptions& options,
		LegacySceneImportStats& stats,
		AssetKind kind,
		std::initializer_list<std::string_view> pathSegments,
		const std::string& name,
		const std::string& path,
		const std::string& materialType = {}
	)
	{
		if (!options.assetRegistry)
		{
			return false;
		}

		AssetDescriptor descriptor{};
		descriptor.kind = kind;
		descriptor.source = options.assetHandleSource;
		descriptor.name = name;
		descriptor.path = path;
		descriptor.materialType = materialType;
		descriptor.handle = makeAssetHandle(kind, options.assetHandleSource, pathSegments);

		const bool alreadyRegistered = options.assetRegistry->contains(descriptor.handle);
		if (!options.assetRegistry->registerAsset(descriptor) || alreadyRegistered)
		{
			return false;
		}

		++stats.assetHandles;
		if (kind == AssetKind::Mesh)
		{
			++stats.meshAssetHandles;
		}
		else if (kind == AssetKind::Material)
		{
			++stats.materialAssetHandles;
		}
		else if (kind == AssetKind::Texture)
		{
			++stats.textureAssetHandles;
		}
		return true;
	}

	void registerMeshAssetHandles(
		const std::shared_ptr<GLframework::Mesh>& mesh,
		const std::string& objectPath,
		const LegacySceneImportOptions& options,
		LegacySceneImportStats& stats
	)
	{
		if (!mesh || !options.assetRegistry)
		{
			return;
		}

		const std::string scope = assetScopeOrPersistentScope(options);
		registerAssetHandle(
			options,
			stats,
			AssetKind::Mesh,
			{ scope, objectPath },
			makeActorName(mesh, "Imported Mesh"),
			objectPath
		);

		const auto material = mesh->getMaterial();
		if (!material)
		{
			return;
		}

		const std::string materialType = materialTypeName(material->getMaterialType());
		registerAssetHandle(
			options,
			stats,
			AssetKind::Material,
			{ scope, objectPath, materialType },
			materialType,
			objectPath,
			materialType
		);

		const auto pbrMaterial = std::dynamic_pointer_cast<GLframework::PBRMaterial>(material);
		if (!pbrMaterial)
		{
			return;
		}

		for (const auto& slot : pbrMaterial->getTextureSlots())
		{
			if (!slot.texture || !*slot.texture)
			{
				continue;
			}

			const auto& texture = *slot.texture;
			const std::string slotLabel = slot.label ? slot.label : "Texture";
			const std::string texturePath =
				objectPath
				+ "/"
				+ sanitizeAssetHandleSegment(slotLabel, "texture")
				+ "-"
				+ std::to_string(texture->getWidth())
				+ "x"
				+ std::to_string(texture->getHeight())
				+ "-unit"
				+ std::to_string(texture->getUnit());
			registerAssetHandle(
				options,
				stats,
				AssetKind::Texture,
				{ scope, texturePath },
				slotLabel,
				texturePath,
				materialType
			);
		}
	}

	Actor* spawnActorForLegacyObject(
		Level& level,
		const std::shared_ptr<GLframework::Object>& object,
		LegacySceneImportStats& stats
	)
	{
		if (!object)
		{
			return nullptr;
		}

		if (object->getType() == GLframework::ObjectType::Mesh || object->getType() == GLframework::ObjectType::InstancedMesh)
		{
			if (auto mesh = std::dynamic_pointer_cast<GLframework::Mesh>(object))
			{
				++stats.meshActors;
				return &level.spawnActor<MeshActor>(mesh, makeActorName(object, "Legacy Mesh Actor"));
			}
		}

		if (object->getType() == GLframework::ObjectType::Light)
		{
			if (auto light = std::dynamic_pointer_cast<GLframework::Light>(object))
			{
				++stats.lightActors;
				return &level.spawnActor<LightActor>(light, makeActorName(object, "Legacy Light Actor"));
			}
		}

		++stats.legacyObjectActors;
		return &level.spawnActor<LegacyObjectActor>(object, makeActorName(object, "Legacy Object Actor"));
	}

	void importObjectRecursive(
		Level& level,
		const std::shared_ptr<GLframework::Object>& object,
		SceneComponent* parentComponent,
		const std::string& objectPath,
		const LegacySceneImportOptions& options,
		LegacySceneImportStats& stats
	)
	{
		if (!object)
		{
			return;
		}

		++stats.visitedObjects;
		Actor* actor = spawnActorForLegacyObject(level, object, stats);
		SceneComponent* rootComponent = actor ? actor->getRootComponent() : nullptr;
		if (actor)
		{
			actor->setPersistentId(makePersistentId(
				"actor",
				options.persistentIdSource,
				options.persistentIdScope,
				{ objectPath }
			));
			++stats.actorsWithPersistentIds;
		}
		if (rootComponent)
		{
			rootComponent->setPersistentId(makePersistentId(
				"component",
				options.persistentIdSource,
				options.persistentIdScope,
				{ objectPath, "root" }
			));
			++stats.sceneComponentsWithPersistentIds;
			rootComponent->setRelativeTransform(LegacySceneWorldBuilder::makeTransform(*object));
			if (parentComponent)
			{
				rootComponent->attachTo(parentComponent);
				++stats.componentAttachments;
			}
		}
		if (auto mesh = std::dynamic_pointer_cast<GLframework::Mesh>(object))
		{
			registerMeshAssetHandles(mesh, objectPath, options, stats);
		}

		std::unordered_map<std::string, int> childOccurrences{};
		for (const auto& child : object->getChildren())
		{
			const std::string key = makeLegacyObjectKey(child);
			const int occurrence = childOccurrences[key]++;
			const std::string childPath = objectPath + "/" + makeLegacyObjectPathSegment(child, occurrence);
			importObjectRecursive(level, child, rootComponent, childPath, options, stats);
		}
	}
}

LegacySceneImportStats LegacySceneWorldBuilder::importScene(
	World& world,
	const std::shared_ptr<GLframework::Scene>& scene
)
{
	return importScene(world, scene, LegacySceneImportOptions{});
}

LegacySceneImportStats LegacySceneWorldBuilder::importScene(
	World& world,
	const std::shared_ptr<GLframework::Scene>& scene,
	const LegacySceneImportOptions& options
)
{
	Level* level = world.getPersistentLevel();
	if (!level)
	{
		level = &world.createPersistentLevel();
	}
	if (world.getPersistentId().empty())
	{
		world.setPersistentId(makePersistentId(
			"world",
			options.persistentIdSource,
			options.persistentIdScope
		));
	}
	if (level->getPersistentId().empty())
	{
		level->setPersistentId(makePersistentId(
			"level",
			options.persistentIdSource,
			options.persistentIdScope,
			{ "persistent-level" }
		));
	}

	return importObjectTree(*level, scene, options);
}

LegacySceneImportStats LegacySceneWorldBuilder::importObjectTree(
	Level& level,
	const std::shared_ptr<GLframework::Object>& root
)
{
	LegacySceneImportOptions options{};
	options.persistentIdScope = "legacy-object-tree";
	return importObjectTree(level, root, options);
}

LegacySceneImportStats LegacySceneWorldBuilder::importObjectTree(
	Level& level,
	const std::shared_ptr<GLframework::Object>& root,
	const LegacySceneImportOptions& options
)
{
	LegacySceneImportStats stats{};
	if (level.getPersistentId().empty())
	{
		level.setPersistentId(makePersistentId(
			"level",
			options.persistentIdSource,
			options.persistentIdScope,
			{ "persistent-level" }
		));
	}
	if (World* world = level.getWorld(); world && world->getPersistentId().empty())
	{
		world->setPersistentId(makePersistentId(
			"world",
			options.persistentIdSource,
			options.persistentIdScope
		));
	}

	const std::string rootPath = makeLegacyObjectPathSegment(root, 0);
	importObjectRecursive(level, root, nullptr, rootPath, options, stats);
	return stats;
}

Transform LegacySceneWorldBuilder::makeTransform(const GLframework::Object& object)
{
	const glm::vec3 position = object.getPosition();
	const glm::vec3 scale = object.getScale();

	Transform transform{};
	transform.location = Vector3{ position.x, position.y, position.z };
	transform.rotation = Vector3{ object.getAngleX(), object.getAngleY(), object.getAngleZ() };
	transform.scale = Vector3{ scale.x, scale.y, scale.z };
	return transform;
}

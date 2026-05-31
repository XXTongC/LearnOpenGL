#include "engine/WorldLegacySceneExporter.h"

#include <memory>
#include <unordered_set>

#include "engine/Actor.h"
#include "engine/ActorAdapters.h"
#include "engine/ActorComponent.h"
#include "engine/Level.h"
#include "engine/SceneComponent.h"
#include "engine/World.h"
#include "framework/object.h"
#include "framework/scene.h"
#include "light/light.h"
#include "mesh/mesh.h"

using namespace GLengine;

namespace
{
	std::shared_ptr<GLframework::Object> getExportObject(const SceneComponent& component)
	{
		if (const auto* meshComponent = dynamic_cast<const MeshComponent*>(&component))
		{
			return std::static_pointer_cast<GLframework::Object>(meshComponent->getMesh());
		}
		if (const auto* lightComponent = dynamic_cast<const LightComponent*>(&component))
		{
			return std::static_pointer_cast<GLframework::Object>(lightComponent->getLight());
		}
		if (const auto* legacyComponent = dynamic_cast<const LegacyObjectComponent*>(&component))
		{
			return legacyComponent->getObject();
		}

		return nullptr;
	}

	void classifyExportedObject(const std::shared_ptr<GLframework::Object>& object, WorldLegacySceneExportStats& stats)
	{
		if (!object)
		{
			return;
		}

		switch (object->getType())
		{
		case GLframework::ObjectType::Mesh:
		case GLframework::ObjectType::InstancedMesh:
			++stats.exportedMeshes;
			break;
		case GLframework::ObjectType::Light:
			++stats.exportedLights;
			break;
		default:
			++stats.exportedLegacyObjects;
			break;
		}
	}

	void exportComponentRecursive(
		const SceneComponent& component,
		const std::shared_ptr<GLframework::Object>& parentObject,
		const std::shared_ptr<GLframework::Scene>& scene,
		std::unordered_set<const SceneComponent*>& visitedComponents,
		std::unordered_set<GLframework::Object*>& exportedObjects,
		WorldLegacySceneExportStats& stats
	)
	{
		if (!visitedComponents.insert(&component).second)
		{
			return;
		}

		++stats.visitedSceneComponents;
		std::shared_ptr<GLframework::Object> currentObject = getExportObject(component);
		if (currentObject && exportedObjects.insert(currentObject.get()).second)
		{
			WorldLegacySceneExporter::applyTransform(*currentObject, component.getRelativeTransform());
			if (parentObject)
			{
				parentObject->addChild(currentObject);
				++stats.objectAttachments;
			}
			else if (scene)
			{
				scene->addChild(currentObject);
				++stats.sceneRootObjects;
			}

			++stats.exportedObjects;
			classifyExportedObject(currentObject, stats);
		}

		const auto& children = component.getChildren();
		for (const SceneComponent* child : children)
		{
			if (!child)
			{
				continue;
			}

			exportComponentRecursive(
				*child,
				currentObject ? currentObject : parentObject,
				scene,
				visitedComponents,
				exportedObjects,
				stats
			);
		}
	}
}

WorldLegacySceneExportStats WorldLegacySceneExporter::exportWorldToScene(
	const World& world,
	const std::shared_ptr<GLframework::Scene>& scene
)
{
	const Level* level = world.getPersistentLevel();
	if (!level)
	{
		return {};
	}

	return exportLevelToScene(*level, scene);
}

WorldLegacySceneExportStats WorldLegacySceneExporter::exportLevelToScene(
	const Level& level,
	const std::shared_ptr<GLframework::Scene>& scene
)
{
	WorldLegacySceneExportStats stats{};
	if (!scene)
	{
		return stats;
	}

	std::unordered_set<const SceneComponent*> visitedComponents{};
	std::unordered_set<GLframework::Object*> exportedObjects{};
	for (const auto& actor : level.getActors())
	{
		if (!actor)
		{
			continue;
		}

		++stats.visitedActors;
		const SceneComponent* rootComponent = actor->getRootComponent();
		if (!rootComponent || rootComponent->getParent())
		{
			continue;
		}

		exportComponentRecursive(*rootComponent, nullptr, scene, visitedComponents, exportedObjects, stats);
	}

	return stats;
}

void WorldLegacySceneExporter::applyTransform(GLframework::Object& object, const Transform& transform)
{
	object.setPosition({ transform.location.x, transform.location.y, transform.location.z });
	object.setAngleX(transform.rotation.x);
	object.setAngleY(transform.rotation.y);
	object.setAngleZ(transform.rotation.z);
	object.setScale({ transform.scale.x, transform.scale.y, transform.scale.z });
}

#include "EngineWorldInspector.h"

#include <cstdint>
#include <memory>

#include "../../engine/Actor.h"
#include "../../engine/ActorAdapters.h"
#include "../../engine/ActorComponent.h"
#include "../../engine/EngineObject.h"
#include "../../engine/Level.h"
#include "../../engine/SceneComponent.h"
#include "../../engine/World.h"
#include "../../framework/object.h"
#include "../../light/light.h"
#include "../../mesh/mesh.h"
#include "../editor/EditorSelectionState.h"

namespace
{
	glm::vec3 toGlmVec3(const GLengine::Vector3& value)
	{
		return { value.x, value.y, value.z };
	}

	GLengine::Vector3 toEngineVector3(const glm::vec3& value)
	{
		return { value.x, value.y, value.z };
	}

	bool areVec3Equal(const glm::vec3& left, const glm::vec3& right)
	{
		return left.x == right.x && left.y == right.y && left.z == right.z;
	}

	std::string getObjectTypeName(GLframework::ObjectType type)
	{
		switch (type)
		{
		case GLframework::ObjectType::Object: return "Object";
		case GLframework::ObjectType::Mesh: return "Mesh";
		case GLframework::ObjectType::Scene: return "Scene";
		case GLframework::ObjectType::InstancedMesh: return "InstancedMesh";
		case GLframework::ObjectType::Light: return "Light";
		default: return "Unknown";
		}
	}

	std::string getLegacyObjectDisplayName(const std::shared_ptr<GLframework::Object>& object)
	{
		if (!object) return "None";

		const std::string explicitName = object->getName();
		if (!explicitName.empty()) return explicitName;

		return getObjectTypeName(object->getType());
	}

	std::shared_ptr<GLframework::Object> getLegacyObjectForSceneComponent(GLengine::SceneComponent& component)
	{
		if (auto* meshComponent = dynamic_cast<GLengine::MeshComponent*>(&component))
		{
			return std::static_pointer_cast<GLframework::Object>(meshComponent->getMesh());
		}
		if (auto* lightComponent = dynamic_cast<GLengine::LightComponent*>(&component))
		{
			return std::static_pointer_cast<GLframework::Object>(lightComponent->getLight());
		}
		if (auto* legacyComponent = dynamic_cast<GLengine::LegacyObjectComponent*>(&component))
		{
			return legacyComponent->getObject();
		}

		return nullptr;
	}

	void syncLegacyObjectTransform(GLengine::SceneComponent& component, const GLengine::Transform& transform)
	{
		const auto object = getLegacyObjectForSceneComponent(component);
		if (!object)
		{
			return;
		}

		object->setPosition(toGlmVec3(transform.location));
		object->setAngleX(transform.rotation.x);
		object->setAngleY(transform.rotation.y);
		object->setAngleZ(transform.rotation.z);
		object->setScale(toGlmVec3(transform.scale));
	}

	void recordSceneComponentVec3Edit(
		GL_EDITOR::EditTransactionLog* editTransactions,
		GLengine::SceneComponent& component,
		const std::string& field,
		const glm::vec3& beforeValue,
		const glm::vec3& afterValue
	)
	{
		if (!editTransactions || areVec3Equal(beforeValue, afterValue))
		{
			return;
		}

		editTransactions->recordVec3(
			GL_EDITOR::getEngineObjectDisplayName(component, "SceneComponent"),
			GL_EDITOR::getComponentTypeName(component),
			component.getObjectId(),
			reinterpret_cast<std::uintptr_t>(&component),
			&component,
			field,
			beforeValue,
			afterValue
		);
	}

	void setSceneComponentLocation(GLengine::SceneComponent& component, glm::vec3 location, GL_EDITOR::EditTransactionLog* editTransactions)
	{
		auto transform = component.getRelativeTransform();
		const glm::vec3 beforeValue = toGlmVec3(transform.location);
		transform.location = toEngineVector3(location);
		component.setRelativeTransform(transform);
		syncLegacyObjectTransform(component, transform);
		recordSceneComponentVec3Edit(editTransactions, component, "Relative Location", beforeValue, location);
	}

	void setSceneComponentRotation(GLengine::SceneComponent& component, glm::vec3 rotation, GL_EDITOR::EditTransactionLog* editTransactions)
	{
		auto transform = component.getRelativeTransform();
		const glm::vec3 beforeValue = toGlmVec3(transform.rotation);
		transform.rotation = toEngineVector3(rotation);
		component.setRelativeTransform(transform);
		syncLegacyObjectTransform(component, transform);
		recordSceneComponentVec3Edit(editTransactions, component, "Relative Rotation", beforeValue, rotation);
	}

	void setSceneComponentScale(GLengine::SceneComponent& component, glm::vec3 scale, GL_EDITOR::EditTransactionLog* editTransactions)
	{
		auto transform = component.getRelativeTransform();
		const glm::vec3 beforeValue = toGlmVec3(transform.scale);
		transform.scale = toEngineVector3(scale);
		component.setRelativeTransform(transform);
		syncLegacyObjectTransform(component, transform);
		recordSceneComponentVec3Edit(editTransactions, component, "Relative Scale", beforeValue, scale);
	}
}

namespace GL_EDITOR
{
	std::string getEngineObjectDisplayName(const GLengine::EngineObject& object, const std::string& fallback)
	{
		if (!object.getName().empty()) return object.getName();
		return fallback;
	}

	std::string getActorTypeName(const GLengine::Actor& actor)
	{
		if (dynamic_cast<const GLengine::MeshActor*>(&actor)) return "MeshActor";
		if (dynamic_cast<const GLengine::LightActor*>(&actor)) return "LightActor";
		if (dynamic_cast<const GLengine::CameraActor*>(&actor)) return "CameraActor";
		if (dynamic_cast<const GLengine::LegacyObjectActor*>(&actor)) return "LegacyObjectActor";
		return "Actor";
	}

	std::string getComponentTypeName(const GLengine::ActorComponent& component)
	{
		if (dynamic_cast<const GLengine::MeshComponent*>(&component)) return "MeshComponent";
		if (dynamic_cast<const GLengine::LightComponent*>(&component)) return "LightComponent";
		if (dynamic_cast<const GLengine::CameraComponent*>(&component)) return "CameraComponent";
		if (dynamic_cast<const GLengine::LegacyObjectComponent*>(&component)) return "LegacyObjectComponent";
		if (dynamic_cast<const GLengine::SceneComponent*>(&component)) return "SceneComponent";
		return "ActorComponent";
	}

	bool undoLatestSceneComponentVec3Edit(EditTransactionLog& editTransactions)
	{
		const auto* latest = editTransactions.getLatestRecord();
		if (!latest
			|| latest->kind != EditTransactionRecordKind::TransformVec3
			|| !latest->sceneComponent)
		{
			return false;
		}

		auto* sceneComponent = latest->sceneComponent;
		const std::string field = latest->field;
		const glm::vec3 beforeValue = latest->beforeValue;

		if (field == "Relative Location")
		{
			setSceneComponentLocation(*sceneComponent, beforeValue, nullptr);
		}
		else if (field == "Relative Rotation")
		{
			setSceneComponentRotation(*sceneComponent, beforeValue, nullptr);
		}
		else if (field == "Relative Scale")
		{
			setSceneComponentScale(*sceneComponent, beforeValue, nullptr);
		}
		else
		{
			return false;
		}

		editTransactions.popLatestRecord();
		editTransactions.markDirty();
		return true;
	}

	PropertyBuilder buildActorPropertySchema(GLengine::Actor& actor, bool engineWorldEditable)
	{
		const auto* level = actor.getLevel();
		const auto* world = actor.getWorld();

		PropertyBuilder builder{};
		builder.addSection("Actor");
		builder.addReadOnlyString("Name", getEngineObjectDisplayName(actor, "Actor"));
		builder.addReadOnlyString("Type", getActorTypeName(actor));
		builder.addReadOnlyString("Object ID", std::to_string(actor.getObjectId()));
		builder.addReadOnlyString("Persistent ID", actor.getPersistentId().empty() ? "None" : actor.getPersistentId());
		builder.addReadOnlyString("World", world ? getEngineObjectDisplayName(*world, "World") : "None");
		builder.addReadOnlyString("Level", level ? getEngineObjectDisplayName(*level, "Level") : "None");
		builder.addReadOnlyInt("Components", static_cast<int>(actor.getComponents().size()));
		builder.addReadOnlyBool("Editable World", engineWorldEditable);

		if (auto* root = actor.getRootComponent())
		{
			const auto& transform = root->getRelativeTransform();
			builder.addSection("Root SceneComponent");
			builder.addReadOnlyString("Name", getEngineObjectDisplayName(*root, "SceneComponent"));
			builder.addReadOnlyString("Type", getComponentTypeName(*root));
			builder.addReadOnlyVec3("Location", toGlmVec3(transform.location));
			builder.addReadOnlyVec3("Rotation", toGlmVec3(transform.rotation));
			builder.addReadOnlyVec3("Scale", toGlmVec3(transform.scale));
			builder.addReadOnlyInt("Attached Children", static_cast<int>(root->getChildren().size()));
		}

		return builder;
	}

	PropertyBuilder buildComponentPropertySchema(
		GLengine::ActorComponent& component,
		bool engineWorldEditable,
		EditTransactionLog* editTransactions
	)
	{
		PropertyBuilder builder{};
		builder.addSection("Component");
		builder.addReadOnlyString("Name", getEngineObjectDisplayName(component, "Component"));
		builder.addReadOnlyString("Type", getComponentTypeName(component));
		builder.addReadOnlyString("Persistent ID", component.getPersistentId().empty() ? "None" : component.getPersistentId());
		builder.addReadOnlyBool("Active", component.isActive());
		builder.addReadOnlyBool("Can Tick", component.canTick());
		if (const auto* owner = component.getOwner())
		{
			builder.addReadOnlyString("Owner", getEngineObjectDisplayName(*owner, "Actor"));
		}
		else
		{
			builder.addReadOnlyString("Owner", "None");
		}

		if (auto* sceneComponent = dynamic_cast<GLengine::SceneComponent*>(&component))
		{
			const auto& transform = sceneComponent->getRelativeTransform();
			const auto* parent = sceneComponent->getParent();
			const bool canEditTransform = engineWorldEditable && getLegacyObjectForSceneComponent(*sceneComponent) != nullptr;
			builder.addSection("SceneComponent");
			builder.addReadOnlyBool("Transform Editable", canEditTransform);
			if (canEditTransform)
			{
				builder.addVec3(
					"Relative Location",
					[sceneComponent]() { return toGlmVec3(sceneComponent->getRelativeTransform().location); },
					[sceneComponent, editTransactions](glm::vec3 value) { setSceneComponentLocation(*sceneComponent, value, editTransactions); }
				);
				builder.addVec3(
					"Relative Rotation",
					[sceneComponent]() { return toGlmVec3(sceneComponent->getRelativeTransform().rotation); },
					[sceneComponent, editTransactions](glm::vec3 value) { setSceneComponentRotation(*sceneComponent, value, editTransactions); }
				);
				builder.addVec3(
					"Relative Scale",
					[sceneComponent]() { return toGlmVec3(sceneComponent->getRelativeTransform().scale); },
					[sceneComponent, editTransactions](glm::vec3 value) { setSceneComponentScale(*sceneComponent, value, editTransactions); }
				);
			}
			else
			{
				builder.addReadOnlyVec3("Relative Location", toGlmVec3(transform.location));
				builder.addReadOnlyVec3("Relative Rotation", toGlmVec3(transform.rotation));
				builder.addReadOnlyVec3("Relative Scale", toGlmVec3(transform.scale));
			}
			builder.addReadOnlyString("Parent", parent ? getEngineObjectDisplayName(*parent, "SceneComponent") : "None");
			builder.addReadOnlyInt("Children", static_cast<int>(sceneComponent->getChildren().size()));
		}

		if (auto* meshComponent = dynamic_cast<GLengine::MeshComponent*>(&component))
		{
			builder.addSection("Mesh Adapter");
			builder.addReadOnlyString("Mesh", getLegacyObjectDisplayName(meshComponent->getMesh()));
		}
		else if (auto* lightComponent = dynamic_cast<GLengine::LightComponent*>(&component))
		{
			builder.addSection("Light Adapter");
			builder.addReadOnlyString("Light", getLegacyObjectDisplayName(lightComponent->getLight()));
		}
		else if (auto* cameraComponent = dynamic_cast<GLengine::CameraComponent*>(&component))
		{
			builder.addSection("Camera Adapter");
			builder.addReadOnlyBool("Bound Camera", cameraComponent->getCamera() != nullptr);
		}
		else if (auto* legacyComponent = dynamic_cast<GLengine::LegacyObjectComponent*>(&component))
		{
			builder.addSection("Legacy Object Adapter");
			builder.addReadOnlyString("Object", getLegacyObjectDisplayName(legacyComponent->getObject()));
		}

		return builder;
	}
}

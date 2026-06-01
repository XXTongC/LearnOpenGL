#include "ActorComponentPropertyProviders.h"

#include <cstdint>
#include <memory>
#include <string>

#include "../../engine/ActorAdapters.h"
#include "../../engine/ActorComponent.h"
#include "../../engine/SceneComponent.h"
#include "../../framework/object.h"
#include "../../light/light.h"
#include "../../mesh/mesh.h"
#include "../editor/EditorSelectionState.h"
#include "EngineWorldInspector.h"

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

	bool isSceneComponent(const GL_EDITOR::ActorComponentPropertyProviderContext& context)
	{
		return dynamic_cast<GLengine::SceneComponent*>(&context.component) != nullptr;
	}

	void buildSceneComponentProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::ActorComponentPropertyProviderContext& context
	)
	{
		auto* sceneComponent = dynamic_cast<GLengine::SceneComponent*>(&context.component);
		if (!sceneComponent)
		{
			return;
		}

		const auto& transform = sceneComponent->getRelativeTransform();
		const auto* parent = sceneComponent->getParent();
		const bool canEditTransform = context.engineWorldEditable && getLegacyObjectForSceneComponent(*sceneComponent) != nullptr;
		builder.addSection("SceneComponent");
		builder.addReadOnlyBool("Transform Editable", canEditTransform);
		if (canEditTransform)
		{
			builder.addVec3(
				"Relative Location",
				[sceneComponent]() { return toGlmVec3(sceneComponent->getRelativeTransform().location); },
				[sceneComponent, editTransactions = context.editTransactions](glm::vec3 value) {
					setSceneComponentLocation(*sceneComponent, value, editTransactions);
				}
			);
			builder.addVec3(
				"Relative Rotation",
				[sceneComponent]() { return toGlmVec3(sceneComponent->getRelativeTransform().rotation); },
				[sceneComponent, editTransactions = context.editTransactions](glm::vec3 value) {
					setSceneComponentRotation(*sceneComponent, value, editTransactions);
				}
			);
			builder.addVec3(
				"Relative Scale",
				[sceneComponent]() { return toGlmVec3(sceneComponent->getRelativeTransform().scale); },
				[sceneComponent, editTransactions = context.editTransactions](glm::vec3 value) {
					setSceneComponentScale(*sceneComponent, value, editTransactions);
				}
			);
		}
		else
		{
			builder.addReadOnlyVec3("Relative Location", toGlmVec3(transform.location));
			builder.addReadOnlyVec3("Relative Rotation", toGlmVec3(transform.rotation));
			builder.addReadOnlyVec3("Relative Scale", toGlmVec3(transform.scale));
		}
		builder.addReadOnlyString("Parent", parent ? GL_EDITOR::getEngineObjectDisplayName(*parent, "SceneComponent") : "None");
		builder.addReadOnlyInt("Children", static_cast<int>(sceneComponent->getChildren().size()));
	}

	bool isMeshComponent(const GL_EDITOR::ActorComponentPropertyProviderContext& context)
	{
		return dynamic_cast<GLengine::MeshComponent*>(&context.component) != nullptr;
	}

	void buildMeshComponentProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::ActorComponentPropertyProviderContext& context
	)
	{
		auto* meshComponent = dynamic_cast<GLengine::MeshComponent*>(&context.component);
		if (!meshComponent)
		{
			return;
		}

		builder.addSection("Mesh Adapter");
		builder.addReadOnlyString("Mesh", getLegacyObjectDisplayName(meshComponent->getMesh()));
	}

	bool isLightComponent(const GL_EDITOR::ActorComponentPropertyProviderContext& context)
	{
		return dynamic_cast<GLengine::LightComponent*>(&context.component) != nullptr;
	}

	void buildLightComponentProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::ActorComponentPropertyProviderContext& context
	)
	{
		auto* lightComponent = dynamic_cast<GLengine::LightComponent*>(&context.component);
		if (!lightComponent)
		{
			return;
		}

		builder.addSection("Light Adapter");
		builder.addReadOnlyString("Light", getLegacyObjectDisplayName(lightComponent->getLight()));
	}

	bool isCameraComponent(const GL_EDITOR::ActorComponentPropertyProviderContext& context)
	{
		return dynamic_cast<GLengine::CameraComponent*>(&context.component) != nullptr;
	}

	void buildCameraComponentProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::ActorComponentPropertyProviderContext& context
	)
	{
		auto* cameraComponent = dynamic_cast<GLengine::CameraComponent*>(&context.component);
		if (!cameraComponent)
		{
			return;
		}

		builder.addSection("Camera Adapter");
		builder.addReadOnlyBool("Bound Camera", cameraComponent->getCamera() != nullptr);
	}

	bool isLegacyObjectComponent(const GL_EDITOR::ActorComponentPropertyProviderContext& context)
	{
		return dynamic_cast<GLengine::LegacyObjectComponent*>(&context.component) != nullptr;
	}

	void buildLegacyObjectComponentProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::ActorComponentPropertyProviderContext& context
	)
	{
		auto* legacyComponent = dynamic_cast<GLengine::LegacyObjectComponent*>(&context.component);
		if (!legacyComponent)
		{
			return;
		}

		builder.addSection("Legacy Object Adapter");
		builder.addReadOnlyString("Object", getLegacyObjectDisplayName(legacyComponent->getObject()));
	}

	GL_EDITOR::ActorComponentPropertyProviderRegistry buildDefaultActorComponentPropertyProviderRegistry()
	{
		GL_EDITOR::ActorComponentPropertyProviderRegistry registry{};
		registry.registerProvider({ "scene-component", isSceneComponent, buildSceneComponentProperties });
		registry.registerProvider({ "mesh-component", isMeshComponent, buildMeshComponentProperties });
		registry.registerProvider({ "light-component", isLightComponent, buildLightComponentProperties });
		registry.registerProvider({ "camera-component", isCameraComponent, buildCameraComponentProperties });
		registry.registerProvider({ "legacy-object-component", isLegacyObjectComponent, buildLegacyObjectComponentProperties });
		return registry;
	}
}

namespace GL_EDITOR
{
	const ActorComponentPropertyProviderRegistry& getDefaultActorComponentPropertyProviderRegistry()
	{
		static const auto registry = buildDefaultActorComponentPropertyProviderRegistry();
		return registry;
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
}

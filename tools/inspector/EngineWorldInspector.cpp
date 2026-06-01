#include "EngineWorldInspector.h"

#include "../../engine/Actor.h"
#include "../../engine/ActorAdapters.h"
#include "../../engine/ActorComponent.h"
#include "../../engine/EngineObject.h"
#include "../../engine/Level.h"
#include "../../engine/SceneComponent.h"
#include "../../engine/World.h"
#include "ActorComponentPropertyProviders.h"

namespace
{
	glm::vec3 toGlmVec3(const GLengine::Vector3& value)
	{
		return { value.x, value.y, value.z };
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

		ActorComponentPropertyProviderContext providerContext{ component, engineWorldEditable, editTransactions };
		getDefaultActorComponentPropertyProviderRegistry().buildMatching(builder, providerContext);

		return builder;
	}
}

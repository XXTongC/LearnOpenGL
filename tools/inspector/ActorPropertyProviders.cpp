#include "ActorPropertyProviders.h"

#include "../../engine/Actor.h"
#include "../../engine/SceneComponent.h"
#include "EngineWorldInspector.h"

namespace
{
	glm::vec3 toGlmVec3(const GLengine::Vector3& value)
	{
		return { value.x, value.y, value.z };
	}

	bool hasRootSceneComponent(const GL_EDITOR::ActorPropertyProviderContext& context)
	{
		return context.actor.getRootComponent() != nullptr;
	}

	void buildRootSceneComponentProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::ActorPropertyProviderContext& context
	)
	{
		auto* root = context.actor.getRootComponent();
		if (!root)
		{
			return;
		}

		const auto& transform = root->getRelativeTransform();
		builder.addSection("Root SceneComponent");
		builder.addReadOnlyString("Name", GL_EDITOR::getEngineObjectDisplayName(*root, "SceneComponent"));
		builder.addReadOnlyString("Type", GL_EDITOR::getComponentTypeName(*root));
		builder.addReadOnlyVec3("Location", toGlmVec3(transform.location));
		builder.addReadOnlyVec3("Rotation", toGlmVec3(transform.rotation));
		builder.addReadOnlyVec3("Scale", toGlmVec3(transform.scale));
		builder.addReadOnlyInt("Attached Children", static_cast<int>(root->getChildren().size()));
	}

	GL_EDITOR::ActorPropertyProviderRegistry buildDefaultActorPropertyProviderRegistry()
	{
		GL_EDITOR::ActorPropertyProviderRegistry registry{};
		registry.registerProvider({ "root-scene-component", hasRootSceneComponent, buildRootSceneComponentProperties });
		return registry;
	}
}

namespace GL_EDITOR
{
	const ActorPropertyProviderRegistry& getDefaultActorPropertyProviderRegistry()
	{
		static const auto registry = buildDefaultActorPropertyProviderRegistry();
		return registry;
	}
}

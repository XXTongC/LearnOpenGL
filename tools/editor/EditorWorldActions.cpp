#include "EditorWorldActions.h"

#include <string>

#include "../../engine/Actor.h"
#include "../../engine/Level.h"
#include "../../engine/PersistentIdPolicy.h"
#include "../../engine/SceneComponent.h"
#include "../../engine/World.h"

using namespace GL_EDITOR;

namespace
{
	constexpr const char* kDefaultEditorCreatedScope = "runtime-editor";

	bool startsWith(const std::string& value, const std::string& prefix)
	{
		return value.rfind(prefix, 0) == 0;
	}

	std::string normalizedScope(const std::string& scope)
	{
		return scope.empty() ? kDefaultEditorCreatedScope : scope;
	}

	std::string editorCreatedPersistentIdPrefix(const char* objectKind, const std::string& scope)
	{
		return GLengine::makePersistentId(
			objectKind,
			GLengine::PersistentIdSource::EditorCreatedGenerated,
			normalizedScope(scope)
		) + ":";
	}

	bool actorPersistentIdExists(const GLengine::World& world, const std::string& persistentId)
	{
		const auto* level = world.getPersistentLevel();
		if (!level)
		{
			return false;
		}

		for (const auto& actor : level->getActors())
		{
			if (actor && actor->getPersistentId() == persistentId)
			{
				return true;
			}
		}
		return false;
	}

	std::string makeEditorCreatedActorSegment(int actorIndex)
	{
		return "actor-" + std::to_string(actorIndex);
	}

	int findNextEditorCreatedActorIndex(const GLengine::World& world, const std::string& scope)
	{
		int candidate = countEditorCreatedActors(world);
		for (;;)
		{
			const std::string actorSegment = makeEditorCreatedActorSegment(candidate);
			const std::string persistentId = GLengine::makePersistentId(
				"actor",
				GLengine::PersistentIdSource::EditorCreatedGenerated,
				normalizedScope(scope),
				{ actorSegment }
			);
			if (!actorPersistentIdExists(world, persistentId))
			{
				return candidate;
			}
			++candidate;
		}
	}
}

EditorCreatedActorResult GL_EDITOR::createEditorEmptyActor(
	GLengine::World& world,
	const EditorCreateActorOptions& options
)
{
	EditorCreatedActorResult result{};
	GLengine::Level* level = world.getPersistentLevel();
	if (!level)
	{
		level = &world.createPersistentLevel();
	}
	if (!level)
	{
		result.error = "missing persistent level";
		return result;
	}

	const std::string scope = normalizedScope(options.persistentIdScope);
	const int actorIndex = findNextEditorCreatedActorIndex(world, scope);
	const std::string actorSegment = makeEditorCreatedActorSegment(actorIndex);
	const std::string actorName = options.actorName.empty()
		? "Editor Created Empty Actor"
		: options.actorName;
	const std::string rootName = options.rootComponentName.empty()
		? "Editor Created Root"
		: options.rootComponentName;

	auto& actor = level->spawnActor<GLengine::Actor>(actorName);
	actor.setPersistentId(GLengine::makePersistentId(
		"actor",
		GLengine::PersistentIdSource::EditorCreatedGenerated,
		scope,
		{ actorSegment }
	));

	auto& rootComponent = actor.createComponent<GLengine::SceneComponent>(rootName);
	rootComponent.setPersistentId(GLengine::makePersistentId(
		"component",
		GLengine::PersistentIdSource::EditorCreatedGenerated,
		scope,
		{ actorSegment, "root" }
	));
	rootComponent.setRelativeTransform(options.transform);
	actor.setRootComponent(&rootComponent);

	result.created = true;
	result.actor = &actor;
	result.rootComponent = &rootComponent;
	result.actorPersistentId = actor.getPersistentId();
	result.rootComponentPersistentId = rootComponent.getPersistentId();
	result.editorCreatedActorCount = countEditorCreatedActors(world);
	result.editorCreatedSceneComponentCount = countEditorCreatedSceneComponents(world);
	return result;
}

int GL_EDITOR::countEditorCreatedActors(const GLengine::World& world)
{
	const auto* level = world.getPersistentLevel();
	if (!level)
	{
		return 0;
	}

	int count = 0;
	const std::string prefix = editorCreatedPersistentIdPrefix("actor", kDefaultEditorCreatedScope);
	for (const auto& actor : level->getActors())
	{
		if (actor && startsWith(actor->getPersistentId(), prefix))
		{
			++count;
		}
	}
	return count;
}

int GL_EDITOR::countEditorCreatedSceneComponents(const GLengine::World& world)
{
	const auto* level = world.getPersistentLevel();
	if (!level)
	{
		return 0;
	}

	int count = 0;
	const std::string prefix = editorCreatedPersistentIdPrefix("component", kDefaultEditorCreatedScope);
	for (const auto& actor : level->getActors())
	{
		if (!actor)
		{
			continue;
		}
		for (const auto& component : actor->getComponents())
		{
			const auto* sceneComponent = dynamic_cast<const GLengine::SceneComponent*>(component.get());
			if (sceneComponent && startsWith(sceneComponent->getPersistentId(), prefix))
			{
				++count;
			}
		}
	}
	return count;
}

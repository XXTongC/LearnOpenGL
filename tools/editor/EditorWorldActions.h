#pragma once

#include <string>

#include "../../engine/Transform.h"

namespace GLengine
{
	class Actor;
	class SceneComponent;
	class World;
}

namespace GL_EDITOR
{
	struct EditorCreateActorOptions
	{
		std::string actorName{ "Editor Created Empty Actor" };
		std::string rootComponentName{ "Editor Created Root" };
		std::string persistentIdScope{ "runtime-editor" };
		GLengine::Transform transform{};
	};

	struct EditorCreatedActorResult
	{
		bool created{ false };
		GLengine::Actor* actor{ nullptr };
		GLengine::SceneComponent* rootComponent{ nullptr };
		int editorCreatedActorCount{ 0 };
		int editorCreatedSceneComponentCount{ 0 };
		std::string actorPersistentId{};
		std::string rootComponentPersistentId{};
		std::string error{};
	};

	EditorCreatedActorResult createEditorEmptyActor(
		GLengine::World& world,
		const EditorCreateActorOptions& options = {}
	);

	int countEditorCreatedActors(const GLengine::World& world);
	int countEditorCreatedSceneComponents(const GLengine::World& world);
}

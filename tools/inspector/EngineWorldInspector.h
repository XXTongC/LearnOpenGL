#pragma once

#include <string>

#include "PropertySchema.h"

namespace GLengine
{
	class Actor;
	class ActorComponent;
	class EngineObject;
}

namespace GL_EDITOR
{
	class EditTransactionLog;

	std::string getActorTypeName(const GLengine::Actor& actor);
	std::string getComponentTypeName(const GLengine::ActorComponent& component);
	std::string getEngineObjectDisplayName(const GLengine::EngineObject& object, const std::string& fallback);

	PropertyBuilder buildActorPropertySchema(GLengine::Actor& actor, bool engineWorldEditable);
	PropertyBuilder buildComponentPropertySchema(
		GLengine::ActorComponent& component,
		bool engineWorldEditable,
		EditTransactionLog* editTransactions
	);

	bool undoLatestSceneComponentVec3Edit(EditTransactionLog& editTransactions);
}

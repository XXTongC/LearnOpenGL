#include "SceneTransformSnapshot.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../engine/Actor.h"
#include "../../engine/ActorAdapters.h"
#include "../../engine/ActorComponent.h"
#include "../../engine/Level.h"
#include "../../engine/SceneComponent.h"
#include "../../engine/Transform.h"
#include "../../engine/World.h"
#include "../../framework/object.h"
#include "../../light/light.h"
#include "../../mesh/mesh.h"

namespace
{
	std::string sanitizeConfigValue(std::string value)
	{
		for (auto& character : value)
		{
			if (character == '\r' || character == '\n')
			{
				character = ' ';
			}
		}
		return value;
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

	std::string getObjectNameOrFallback(const GLengine::EngineObject& object, const std::string& fallback)
	{
		return object.getName().empty() ? fallback : object.getName();
	}

	std::string sanitizeStablePathSegment(std::string value)
	{
		for (auto& character : value)
		{
			if (character == '\r'
				|| character == '\n'
				|| character == '/'
				|| character == '\\'
				|| character == '|'
				|| character == '='
				|| character == ':')
			{
				character = '_';
			}
		}
		return value;
	}

	std::string makeStableToken(const std::string& type, const std::string& name, int occurrence)
	{
		return sanitizeStablePathSegment(type)
			+ ":"
			+ sanitizeStablePathSegment(name)
			+ "#"
			+ std::to_string(occurrence);
	}

	std::string makeLevelStablePath(const GLengine::Level& level)
	{
		return "world/level/"
			+ makeStableToken("Level", getObjectNameOrFallback(level, "Persistent Level"), 0);
	}

	void writeVec3(std::ostream& output, const std::string& prefix, const GLengine::Vector3& value)
	{
		output << prefix << "X=" << value.x << '\n';
		output << prefix << "Y=" << value.y << '\n';
		output << prefix << "Z=" << value.z << '\n';
	}

	bool areClose(float left, float right)
	{
		constexpr float epsilon = 0.0001f;
		const float delta = left - right;
		return delta >= -epsilon && delta <= epsilon;
	}

	bool areTransformsClose(const GLengine::Transform& left, const GLengine::Transform& right)
	{
		return areClose(left.location.x, right.location.x)
			&& areClose(left.location.y, right.location.y)
			&& areClose(left.location.z, right.location.z)
			&& areClose(left.rotation.x, right.rotation.x)
			&& areClose(left.rotation.y, right.rotation.y)
			&& areClose(left.rotation.z, right.rotation.z)
			&& areClose(left.scale.x, right.scale.x)
			&& areClose(left.scale.y, right.scale.y)
			&& areClose(left.scale.z, right.scale.z);
	}

	std::string trim(std::string value)
	{
		const auto first = value.find_first_not_of(" \t\r\n");
		if (first == std::string::npos)
		{
			return {};
		}

		const auto last = value.find_last_not_of(" \t\r\n");
		return value.substr(first, last - first + 1);
	}

	bool readSnapshotKeyValues(
		const std::string& path,
		std::map<std::string, std::string>& values,
		std::string& error
	)
	{
		std::ifstream input(path);
		if (!input)
		{
			error = "cannot read " + path;
			return false;
		}

		std::string line{};
		int lineNumber = 0;
		while (std::getline(input, line))
		{
			++lineNumber;
			const std::string trimmedLine = trim(line);
			if (trimmedLine.empty() || trimmedLine[0] == '#')
			{
				continue;
			}

			const auto separator = trimmedLine.find('=');
			if (separator == std::string::npos)
			{
				error = "invalid snapshot line " + std::to_string(lineNumber);
				return false;
			}

			values[trim(trimmedLine.substr(0, separator))] = trim(trimmedLine.substr(separator + 1));
		}

		return true;
	}

	bool getStringValue(
		const std::map<std::string, std::string>& values,
		const std::string& key,
		std::string& value
	)
	{
		const auto found = values.find(key);
		if (found == values.end())
		{
			return false;
		}

		value = found->second;
		return true;
	}

	bool getIntValue(const std::map<std::string, std::string>& values, const std::string& key, int& value)
	{
		std::string rawValue{};
		if (!getStringValue(values, key, rawValue))
		{
			return false;
		}

		try
		{
			value = std::stoi(rawValue);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool getObjectIdValue(const std::map<std::string, std::string>& values, const std::string& key, GLengine::ObjectId& value)
	{
		std::string rawValue{};
		if (!getStringValue(values, key, rawValue))
		{
			return false;
		}

		try
		{
			value = static_cast<GLengine::ObjectId>(std::stoull(rawValue));
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool getFloatValue(const std::map<std::string, std::string>& values, const std::string& key, float& value)
	{
		std::string rawValue{};
		if (!getStringValue(values, key, rawValue))
		{
			return false;
		}

		try
		{
			value = std::stof(rawValue);
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	bool getVec3Value(
		const std::map<std::string, std::string>& values,
		const std::string& prefix,
		GLengine::Vector3& value
	)
	{
		return getFloatValue(values, prefix + "X", value.x)
			&& getFloatValue(values, prefix + "Y", value.y)
			&& getFloatValue(values, prefix + "Z", value.z);
	}

	bool getTransformValue(
		const std::map<std::string, std::string>& values,
		const std::string& prefix,
		GLengine::Transform& transform
	)
	{
		return getVec3Value(values, prefix + "location", transform.location)
			&& getVec3Value(values, prefix + "rotation", transform.rotation)
			&& getVec3Value(values, prefix + "scale", transform.scale);
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

		object->setPosition({ transform.location.x, transform.location.y, transform.location.z });
		object->setAngleX(transform.rotation.x);
		object->setAngleY(transform.rotation.y);
		object->setAngleZ(transform.rotation.z);
		object->setScale({ transform.scale.x, transform.scale.y, transform.scale.z });
	}

	struct ActorSnapshotStats
	{
		int sceneComponentCount{ 0 };
		std::string stablePath{};
		std::vector<std::string> sceneComponentStablePaths{};
	};

	std::vector<ActorSnapshotStats> collectActorSnapshotStats(const GLengine::Level& level, int& sceneComponentCount)
	{
		std::vector<ActorSnapshotStats> actorStats{};
		sceneComponentCount = 0;
		actorStats.reserve(level.getActors().size());
		std::unordered_map<std::string, int> actorOccurrenceCounts{};
		const std::string levelStablePath = makeLevelStablePath(level);
		for (const auto& actor : level.getActors())
		{
			ActorSnapshotStats stats{};
			if (actor)
			{
				const std::string actorName = getObjectNameOrFallback(*actor, "Actor");
				const std::string actorType = getActorTypeName(*actor);
				const std::string actorOccurrenceKey = actorType + "|" + actorName;
				const int actorOccurrence = actorOccurrenceCounts[actorOccurrenceKey]++;
				stats.stablePath =
					levelStablePath
					+ "/actor/"
					+ makeStableToken(actorType, actorName, actorOccurrence);

				std::unordered_map<std::string, int> componentOccurrenceCounts{};
				for (const auto& component : actor->getComponents())
				{
					const auto* sceneComponent = component
						? dynamic_cast<GLengine::SceneComponent*>(component.get())
						: nullptr;
					if (sceneComponent)
					{
						const std::string componentName = getObjectNameOrFallback(*sceneComponent, "SceneComponent");
						const std::string componentType = getComponentTypeName(*sceneComponent);
						const std::string componentOccurrenceKey = componentType + "|" + componentName;
						const int componentOccurrence = componentOccurrenceCounts[componentOccurrenceKey]++;
						stats.sceneComponentStablePaths.push_back(
							stats.stablePath
							+ "/component/"
							+ makeStableToken(componentType, componentName, componentOccurrence)
						);
						++stats.sceneComponentCount;
						++sceneComponentCount;
					}
				}
			}
			actorStats.push_back(stats);
		}
		return actorStats;
	}

	std::vector<std::vector<GLengine::SceneComponent*>> collectIndexedSceneComponents(
		GLengine::Level& level,
		std::unordered_map<std::string, GLengine::SceneComponent*>& sceneComponentsByPersistentId,
		std::unordered_map<GLengine::ObjectId, GLengine::SceneComponent*>& sceneComponentsById,
		std::unordered_map<std::string, GLengine::SceneComponent*>& sceneComponentsByStablePath
	)
	{
		std::vector<std::vector<GLengine::SceneComponent*>> indexedSceneComponents{};
		indexedSceneComponents.reserve(level.getActors().size());
		int sceneComponentCount = 0;
		const auto actorStats = collectActorSnapshotStats(level, sceneComponentCount);
		const auto& actors = level.getActors();
		for (std::size_t actorIndex = 0; actorIndex < actors.size(); ++actorIndex)
		{
			const auto& actor = actors[actorIndex];
			std::vector<GLengine::SceneComponent*> actorComponents{};
			if (actor)
			{
				std::size_t sceneComponentIndex = 0;
				for (const auto& component : actor->getComponents())
				{
					auto* sceneComponent = dynamic_cast<GLengine::SceneComponent*>(component.get());
					if (!sceneComponent)
					{
						continue;
					}

					actorComponents.push_back(sceneComponent);
					if (!sceneComponent->getPersistentId().empty())
					{
						sceneComponentsByPersistentId[sceneComponent->getPersistentId()] = sceneComponent;
					}
					sceneComponentsById[sceneComponent->getObjectId()] = sceneComponent;
					if (actorIndex < actorStats.size()
						&& sceneComponentIndex < actorStats[actorIndex].sceneComponentStablePaths.size())
					{
						sceneComponentsByStablePath[
							actorStats[actorIndex].sceneComponentStablePaths[sceneComponentIndex]
						] = sceneComponent;
					}
					++sceneComponentIndex;
				}
			}
			indexedSceneComponents.push_back(actorComponents);
		}
		return indexedSceneComponents;
	}

	enum class SceneComponentMatchMethod
	{
		None,
		PersistentId,
		StablePath,
		ObjectId,
		Index,
	};

	struct SceneComponentMatch
	{
		GLengine::SceneComponent* target{ nullptr };
		SceneComponentMatchMethod method{ SceneComponentMatchMethod::None };
	};

	SceneComponentMatch findSceneComponentTarget(
		const std::unordered_map<std::string, GLengine::SceneComponent*>& sceneComponentsByPersistentId,
		const std::unordered_map<std::string, GLengine::SceneComponent*>& sceneComponentsByStablePath,
		const std::unordered_map<GLengine::ObjectId, GLengine::SceneComponent*>& sceneComponentsById,
		const std::vector<std::vector<GLengine::SceneComponent*>>& indexedSceneComponents,
		const std::string& persistentId,
		const std::string& stablePath,
		GLengine::ObjectId objectId,
		int actorIndex,
		int sceneComponentIndex
	)
	{
		if (!persistentId.empty())
		{
			const auto persistentIdFound = sceneComponentsByPersistentId.find(persistentId);
			if (persistentIdFound != sceneComponentsByPersistentId.end())
			{
				return { persistentIdFound->second, SceneComponentMatchMethod::PersistentId };
			}
		}

		if (!stablePath.empty())
		{
			const auto stablePathFound = sceneComponentsByStablePath.find(stablePath);
			if (stablePathFound != sceneComponentsByStablePath.end())
			{
				return { stablePathFound->second, SceneComponentMatchMethod::StablePath };
			}
		}

		const auto found = sceneComponentsById.find(objectId);
		if (found != sceneComponentsById.end())
		{
			return { found->second, SceneComponentMatchMethod::ObjectId };
		}

		if (actorIndex < 0 || sceneComponentIndex < 0)
		{
			return {};
		}

		const auto actorOffset = static_cast<std::size_t>(actorIndex);
		if (actorOffset >= indexedSceneComponents.size())
		{
			return {};
		}

		const auto componentOffset = static_cast<std::size_t>(sceneComponentIndex);
		if (componentOffset >= indexedSceneComponents[actorOffset].size())
		{
			return {};
		}

		return { indexedSceneComponents[actorOffset][componentOffset], SceneComponentMatchMethod::Index };
	}

	bool ensureParentDirectory(const std::string& path, std::string& error)
	{
		const std::filesystem::path filePath{ path };
		const auto parentPath = filePath.parent_path();
		if (parentPath.empty())
		{
			return true;
		}

		std::error_code filesystemError{};
		std::filesystem::create_directories(parentPath, filesystemError);
		if (!filesystemError)
		{
			return true;
		}

		error = "cannot create " + parentPath.string();
		return false;
	}
}

std::string GL_EDITOR::defaultSceneTransformSnapshotPath()
{
	return "out/engine_world_transform_snapshot.ini";
}

GL_EDITOR::SceneTransformSnapshotResult GL_EDITOR::saveSceneTransformSnapshot(
	const GLengine::World& world,
	const std::string& path
)
{
	SceneTransformSnapshotResult result{};
	result.path = path;
	if (path.empty())
	{
		result.error = "empty snapshot path";
		return result;
	}

	const auto* level = world.getPersistentLevel();
	if (!level)
	{
		result.error = "world has no persistent level";
		return result;
	}

	std::string error{};
	if (!ensureParentDirectory(path, error))
	{
		result.error = error;
		return result;
	}

	int sceneComponentCount = 0;
	const auto actorStats = collectActorSnapshotStats(*level, sceneComponentCount);
	result.actorCount = static_cast<int>(level->getActors().size());
	result.sceneComponentCount = sceneComponentCount;

	std::ofstream output(path, std::ios::trunc);
	if (!output)
	{
		result.error = "cannot write " + path;
		return result;
	}

	output << "# Engine World SceneComponent transform snapshot\n";
	output << "schema=engine.world.sceneComponentTransform.v1\n";
	output << "world.name=" << sanitizeConfigValue(getObjectNameOrFallback(world, "World")) << '\n';
	output << "world.persistentId=" << world.getPersistentId() << '\n';
	output << "world.objectId=" << world.getObjectId() << '\n';
	output << "level.name=" << sanitizeConfigValue(getObjectNameOrFallback(*level, "Persistent Level")) << '\n';
	output << "level.persistentId=" << level->getPersistentId() << '\n';
	output << "level.objectId=" << level->getObjectId() << '\n';
	output << "actorCount=" << result.actorCount << '\n';
	output << "sceneComponentCount=" << result.sceneComponentCount << '\n';
	output << std::fixed << std::setprecision(6);

	const auto& actors = level->getActors();
	for (std::size_t actorIndex = 0; actorIndex < actors.size(); ++actorIndex)
	{
		const auto& actor = actors[actorIndex];
		if (!actor)
		{
			continue;
		}

		const std::string actorPrefix = "actor." + std::to_string(actorIndex) + ".";
		output << actorPrefix << "name=" << sanitizeConfigValue(getObjectNameOrFallback(*actor, "Actor")) << '\n';
		output << actorPrefix << "type=" << getActorTypeName(*actor) << '\n';
		output << actorPrefix << "persistentId=" << actor->getPersistentId() << '\n';
		output << actorPrefix << "stablePath=" << actorStats[actorIndex].stablePath << '\n';
		output << actorPrefix << "objectId=" << actor->getObjectId() << '\n';
		output << actorPrefix << "sceneComponentCount=" << actorStats[actorIndex].sceneComponentCount << '\n';

		int sceneComponentIndex = 0;
		for (const auto& component : actor->getComponents())
		{
			auto* sceneComponent = dynamic_cast<GLengine::SceneComponent*>(component.get());
			if (!sceneComponent)
			{
				continue;
			}

			const auto& transform = sceneComponent->getRelativeTransform();
			const std::string componentPrefix =
				actorPrefix + "sceneComponent." + std::to_string(sceneComponentIndex) + ".";
			output << componentPrefix << "name="
				<< sanitizeConfigValue(getObjectNameOrFallback(*sceneComponent, "SceneComponent")) << '\n';
			output << componentPrefix << "type=" << getComponentTypeName(*sceneComponent) << '\n';
			output << componentPrefix << "persistentId=" << sceneComponent->getPersistentId() << '\n';
			if (static_cast<std::size_t>(sceneComponentIndex) < actorStats[actorIndex].sceneComponentStablePaths.size())
			{
				output << componentPrefix << "stablePath="
					<< actorStats[actorIndex].sceneComponentStablePaths[static_cast<std::size_t>(sceneComponentIndex)]
					<< '\n';
			}
			output << componentPrefix << "objectId=" << sceneComponent->getObjectId() << '\n';
			output << componentPrefix << "parentObjectId="
				<< (sceneComponent->getParent() ? sceneComponent->getParent()->getObjectId() : 0) << '\n';
			writeVec3(output, componentPrefix + "location", transform.location);
			writeVec3(output, componentPrefix + "rotation", transform.rotation);
			writeVec3(output, componentPrefix + "scale", transform.scale);
			++sceneComponentIndex;
		}
	}

	result.saved = true;
	return result;
}

GL_EDITOR::SceneTransformSnapshotApplyResult GL_EDITOR::applySceneTransformSnapshot(
	GLengine::World& world,
	const std::string& path
)
{
	SceneTransformSnapshotApplyResult result{};
	result.path = path;
	if (path.empty())
	{
		result.error = "empty snapshot path";
		return result;
	}

	auto* level = world.getPersistentLevel();
	if (!level)
	{
		result.error = "world has no persistent level";
		return result;
	}

	std::map<std::string, std::string> values{};
	std::string error{};
	if (!readSnapshotKeyValues(path, values, error))
	{
		result.error = error;
		return result;
	}

	std::string schema{};
	if (!getStringValue(values, "schema", schema) || schema != "engine.world.sceneComponentTransform.v1")
	{
		result.error = "unsupported or missing snapshot schema";
		return result;
	}

	if (!getIntValue(values, "actorCount", result.snapshotActorCount))
	{
		result.error = "missing actorCount";
		return result;
	}
	if (!getIntValue(values, "sceneComponentCount", result.snapshotSceneComponentCount))
	{
		result.error = "missing sceneComponentCount";
		return result;
	}

	std::unordered_map<std::string, GLengine::SceneComponent*> sceneComponentsByPersistentId{};
	std::unordered_map<GLengine::ObjectId, GLengine::SceneComponent*> sceneComponentsById{};
	std::unordered_map<std::string, GLengine::SceneComponent*> sceneComponentsByStablePath{};
	const auto indexedSceneComponents = collectIndexedSceneComponents(
		*level,
		sceneComponentsByPersistentId,
		sceneComponentsById,
		sceneComponentsByStablePath
	);

	for (int actorIndex = 0; actorIndex < result.snapshotActorCount; ++actorIndex)
	{
		const std::string actorPrefix = "actor." + std::to_string(actorIndex) + ".";
		int actorSceneComponentCount = 0;
		if (!getIntValue(values, actorPrefix + "sceneComponentCount", actorSceneComponentCount))
		{
			continue;
		}

		for (int sceneComponentIndex = 0; sceneComponentIndex < actorSceneComponentCount; ++sceneComponentIndex)
		{
			const std::string componentPrefix =
				actorPrefix + "sceneComponent." + std::to_string(sceneComponentIndex) + ".";

			GLengine::ObjectId sceneComponentObjectId = 0;
			getObjectIdValue(values, componentPrefix + "objectId", sceneComponentObjectId);
			std::string sceneComponentPersistentId{};
			getStringValue(values, componentPrefix + "persistentId", sceneComponentPersistentId);
			std::string sceneComponentStablePath{};
			getStringValue(values, componentPrefix + "stablePath", sceneComponentStablePath);

			GLengine::Transform transform{};
			if (!getTransformValue(values, componentPrefix, transform))
			{
				continue;
			}

			const auto match = findSceneComponentTarget(
				sceneComponentsByPersistentId,
				sceneComponentsByStablePath,
				sceneComponentsById,
				indexedSceneComponents,
				sceneComponentPersistentId,
				sceneComponentStablePath,
				sceneComponentObjectId,
				actorIndex,
				sceneComponentIndex
			);
			if (!match.target)
			{
				continue;
			}

			++result.matchedSceneComponentCount;
			if (match.method == SceneComponentMatchMethod::PersistentId)
			{
				++result.matchedByPersistentIdCount;
			}
			else if (match.method == SceneComponentMatchMethod::StablePath)
			{
				++result.matchedByStablePathCount;
			}
			else if (match.method == SceneComponentMatchMethod::ObjectId)
			{
				++result.matchedByObjectIdCount;
			}
			else if (match.method == SceneComponentMatchMethod::Index)
			{
				++result.matchedByIndexCount;
			}
			if (!areTransformsClose(match.target->getRelativeTransform(), transform))
			{
				++result.changedSceneComponentCount;
			}

			match.target->setRelativeTransform(transform);
			syncLegacyObjectTransform(*match.target, transform);
			++result.appliedSceneComponentCount;
		}
	}

	result.applied = result.appliedSceneComponentCount > 0;
	if (!result.applied)
	{
		result.error = "no SceneComponent transforms were applied";
	}
	return result;
}

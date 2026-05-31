#include "engine/ScenePackage.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "engine/Actor.h"
#include "engine/ActorAdapters.h"
#include "engine/AssetRegistry.h"
#include "engine/Level.h"
#include "engine/SceneComponent.h"
#include "engine/Transform.h"
#include "engine/World.h"
#include "framework/object.h"
#include "light/light.h"
#include "materials/material.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "mesh/mesh.h"

using namespace GLengine;

namespace
{
	constexpr const char* kScenePackageSchema = "engine.world.scenePackage.v1";

	struct SavedSceneComponentRef
	{
		SceneComponent* component{ nullptr };
		int index{ -1 };
	};

	struct ScenePackageAssetManifestEntry
	{
		AssetKind kind{ AssetKind::Unknown };
		std::string handle{};
		std::string source{};
		std::string name{};
		std::string path{};
		std::string materialType{};
	};

	struct PendingSceneComponentAttachment
	{
		SceneComponent* child{ nullptr };
		SceneComponent* parent{ nullptr };
		int childActorIndex{ -1 };
		int parentActorIndex{ -1 };
	};

	int countCyclicSceneComponentParentReferences(const std::vector<PendingSceneComponentAttachment>& attachments)
	{
		std::unordered_map<SceneComponent*, SceneComponent*> parentByChild{};
		for (const auto& attachment : attachments)
		{
			if (attachment.child && attachment.parent)
			{
				parentByChild[attachment.child] = attachment.parent;
			}
		}

		std::unordered_set<SceneComponent*> cyclicChildren{};
		for (const auto& entry : parentByChild)
		{
			auto* const origin = entry.first;
			std::unordered_set<SceneComponent*> visited{};
			auto* current = origin;
			while (current)
			{
				const auto foundParent = parentByChild.find(current);
				if (foundParent == parentByChild.end())
				{
					break;
				}

				current = foundParent->second;
				if (current == origin)
				{
					cyclicChildren.insert(origin);
					break;
				}
				if (current && !visited.insert(current).second)
				{
					cyclicChildren.insert(origin);
					break;
				}
			}
		}
		return static_cast<int>(cyclicChildren.size());
	}

	std::string sanitizeConfigValue(std::string value)
	{
		for (auto& character : value)
		{
			if (character == '\r' || character == '\n' || character == '=')
			{
				character = ' ';
			}
		}
		return value;
	}

	std::string objectNameOrFallback(const EngineObject& object, const char* fallback)
	{
		return object.getName().empty() ? std::string{ fallback } : object.getName();
	}

	std::string frameworkObjectNameOrFallback(const GLframework::Object& object, const char* fallback)
	{
		const std::string name = object.getName();
		return name.empty() ? std::string{ fallback } : name;
	}

	std::string makeRuntimeGeneratedReference(std::string_view kind, const std::string& objectName)
	{
		return "runtime-generated:"
			+ std::string{ kind }
			+ ":"
			+ sanitizeAssetHandleSegment(objectName, "unnamed");
	}

	std::string objectTypeName(GLframework::ObjectType type)
	{
		switch (type)
		{
		case GLframework::ObjectType::Object:
			return "Object";
		case GLframework::ObjectType::Mesh:
			return "Mesh";
		case GLframework::ObjectType::Scene:
			return "Scene";
		case GLframework::ObjectType::InstancedMesh:
			return "InstancedMesh";
		case GLframework::ObjectType::Light:
			return "Light";
		}
		return "Unknown";
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

	bool assetManifestEntryExists(
		const std::vector<ScenePackageAssetManifestEntry>& entries,
		const std::string& handle
	)
	{
		return std::any_of(
			entries.begin(),
			entries.end(),
			[&handle](const auto& entry)
			{
				return entry.handle == handle;
			}
		);
	}

	void addAssetManifestEntry(
		std::vector<ScenePackageAssetManifestEntry>& entries,
		ScenePackageSaveResult& result,
		AssetKind kind,
		const std::string& source,
		std::initializer_list<std::string_view> pathSegments,
		const std::string& name,
		const std::string& path,
		const std::string& materialType = {}
	)
	{
		const AssetHandle handle = makeAssetHandle(kind, source, pathSegments);
		if (!handle.isValid() || assetManifestEntryExists(entries, handle.value))
		{
			return;
		}

		ScenePackageAssetManifestEntry entry{};
		entry.kind = kind;
		entry.handle = handle.value;
		entry.source = source;
		entry.name = name;
		entry.path = path;
		entry.materialType = materialType;
		entries.push_back(entry);

		++result.assetManifestCount;
		if (kind == AssetKind::Mesh)
		{
			++result.meshAssetManifestCount;
		}
		else if (kind == AssetKind::Material)
		{
			++result.materialAssetManifestCount;
		}
		else if (kind == AssetKind::Texture)
		{
			++result.textureAssetManifestCount;
		}
	}

	bool extractImportedAssetPath(
		const std::string& persistentId,
		std::string& scope,
		std::string& objectPath
	)
	{
		const std::string prefix = "component:imported-asset:";
		if (persistentId.find(prefix) != 0)
		{
			return false;
		}

		std::string remainder = persistentId.substr(prefix.size());
		const auto scopeEnd = remainder.find(':');
		if (scopeEnd == std::string::npos)
		{
			return false;
		}

		scope = remainder.substr(0, scopeEnd);
		objectPath = remainder.substr(scopeEnd + 1);
		const std::string rootSuffix = ":root";
		if (objectPath.size() > rootSuffix.size()
			&& objectPath.compare(objectPath.size() - rootSuffix.size(), rootSuffix.size(), rootSuffix) == 0)
		{
			objectPath.resize(objectPath.size() - rootSuffix.size());
		}
		return !scope.empty() && !objectPath.empty();
	}

	void collectImportedAssetManifestEntries(
		const SceneComponent& sceneComponent,
		std::vector<ScenePackageAssetManifestEntry>& entries,
		ScenePackageSaveResult& result
	)
	{
		std::string scope{};
		std::string objectPath{};
		if (!extractImportedAssetPath(sceneComponent.getPersistentId(), scope, objectPath))
		{
			return;
		}

		const auto* meshComponent = dynamic_cast<const MeshComponent*>(&sceneComponent);
		const auto mesh = meshComponent ? meshComponent->getMesh() : nullptr;
		if (!mesh)
		{
			return;
		}

		const std::string source = "imported-asset";
		const std::string meshName = frameworkObjectNameOrFallback(*mesh, "Imported Mesh");
		addAssetManifestEntry(
			entries,
			result,
			AssetKind::Mesh,
			source,
			{ scope, objectPath },
			meshName,
			objectPath
		);

		const auto material = mesh->getMaterial();
		if (!material)
		{
			return;
		}

		const std::string materialType = materialTypeName(material->getMaterialType());
		addAssetManifestEntry(
			entries,
			result,
			AssetKind::Material,
			source,
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
			addAssetManifestEntry(
				entries,
				result,
				AssetKind::Texture,
				source,
				{ scope, texturePath },
				slotLabel,
				texturePath,
				materialType
			);
		}
	}

	std::string actorTypeName(const Actor& actor)
	{
		if (dynamic_cast<const MeshActor*>(&actor)) return "MeshActor";
		if (dynamic_cast<const LightActor*>(&actor)) return "LightActor";
		if (dynamic_cast<const CameraActor*>(&actor)) return "CameraActor";
		if (dynamic_cast<const LegacyObjectActor*>(&actor)) return "LegacyObjectActor";
		return "Actor";
	}

	bool isTypedActorType(const std::string& actorType)
	{
		return actorType == "MeshActor"
			|| actorType == "LightActor"
			|| actorType == "CameraActor"
			|| actorType == "LegacyObjectActor";
	}

	bool isKnownActorType(const std::string& actorType)
	{
		return actorType == "Actor" || isTypedActorType(actorType);
	}

	Actor& createActorFromType(Level& level, const std::string& actorType, const std::string& actorName)
	{
		if (actorType == "MeshActor")
		{
			return level.spawnActor<MeshActor>(std::shared_ptr<GLframework::Mesh>{}, actorName);
		}
		if (actorType == "LightActor")
		{
			return level.spawnActor<LightActor>(std::shared_ptr<GLframework::Light>{}, actorName);
		}
		if (actorType == "CameraActor")
		{
			return level.spawnActor<CameraActor>(nullptr, actorName);
		}
		if (actorType == "LegacyObjectActor")
		{
			return level.spawnActor<LegacyObjectActor>(std::shared_ptr<GLframework::Object>{}, actorName);
		}
		return level.spawnActor<Actor>(actorName);
	}

	bool sceneComponentMatchesType(const SceneComponent& component, const std::string& componentType)
	{
		if (componentType == "MeshComponent")
		{
			return dynamic_cast<const MeshComponent*>(&component) != nullptr;
		}
		if (componentType == "LightComponent")
		{
			return dynamic_cast<const LightComponent*>(&component) != nullptr;
		}
		if (componentType == "CameraComponent")
		{
			return dynamic_cast<const CameraComponent*>(&component) != nullptr;
		}
		if (componentType == "LegacyObjectComponent")
		{
			return dynamic_cast<const LegacyObjectComponent*>(&component) != nullptr;
		}
		return componentType == "SceneComponent";
	}

	bool isKnownSceneComponentType(const std::string& componentType)
	{
		return componentType == "SceneComponent"
			|| componentType == "MeshComponent"
			|| componentType == "LightComponent"
			|| componentType == "CameraComponent"
			|| componentType == "LegacyObjectComponent";
	}

	SceneComponent* tryReuseActorRootComponent(
		Actor& actor,
		const std::string& componentType,
		const std::string& componentName,
		int sceneComponentIndex
	)
	{
		if (sceneComponentIndex != 0)
		{
			return nullptr;
		}

		auto* rootComponent = actor.getRootComponent();
		if (!rootComponent || !sceneComponentMatchesType(*rootComponent, componentType))
		{
			return nullptr;
		}

		rootComponent->setName(componentName);
		return rootComponent;
	}

	ScenePackageAdapterDescriptor describeSceneComponentAdapter(const SceneComponent& component)
	{
		ScenePackageAdapterDescriptor descriptor{};
		if (const auto* meshComponent = dynamic_cast<const MeshComponent*>(&component))
		{
			descriptor.componentType = "MeshComponent";
			descriptor.adapterKind = "mesh";
			if (const auto& mesh = meshComponent->getMesh())
			{
				descriptor.hasAdapterReference = true;
				descriptor.objectName = frameworkObjectNameOrFallback(*mesh, "Mesh");
				descriptor.objectType = objectTypeName(mesh->getType());
				descriptor.assetHandle = makeAssetHandle(
					AssetKind::Mesh,
					"runtime-generated",
					{ descriptor.objectName }
				).value;
				descriptor.assetReference = makeRuntimeGeneratedReference("mesh", descriptor.objectName);
				if (const auto material = mesh->getMaterial())
				{
					descriptor.materialType = materialTypeName(material->getMaterialType());
				}
			}
			return descriptor;
		}
		if (const auto* lightComponent = dynamic_cast<const LightComponent*>(&component))
		{
			descriptor.componentType = "LightComponent";
			descriptor.adapterKind = "light";
			if (const auto& light = lightComponent->getLight())
			{
				descriptor.hasAdapterReference = true;
				descriptor.objectName = frameworkObjectNameOrFallback(*light, "Light");
				descriptor.objectType = objectTypeName(light->getType());
				descriptor.assetHandle = makeAssetHandle(
					AssetKind::Light,
					"runtime-generated",
					{ descriptor.objectName }
				).value;
				descriptor.assetReference = makeRuntimeGeneratedReference("light", descriptor.objectName);
			}
			return descriptor;
		}
		if (const auto* cameraComponent = dynamic_cast<const CameraComponent*>(&component))
		{
			descriptor.componentType = "CameraComponent";
			descriptor.adapterKind = "camera";
			if (cameraComponent->getCamera())
			{
				descriptor.objectName = "Runtime Camera";
			}
			return descriptor;
		}
		if (const auto* legacyComponent = dynamic_cast<const LegacyObjectComponent*>(&component))
		{
			descriptor.componentType = "LegacyObjectComponent";
			descriptor.adapterKind = "legacy-object";
			if (const auto& object = legacyComponent->getObject())
			{
				descriptor.hasAdapterReference = true;
				descriptor.objectName = frameworkObjectNameOrFallback(*object, "Object");
				descriptor.objectType = objectTypeName(object->getType());
				descriptor.assetHandle = makeAssetHandle(
					AssetKind::LegacyObject,
					"runtime-generated",
					{ descriptor.objectName }
				).value;
				descriptor.assetReference = makeRuntimeGeneratedReference("legacy-object", descriptor.objectName);
			}
			return descriptor;
		}
		return descriptor;
	}

	SceneComponent& createSceneComponentFromType(
		Actor& actor,
		const std::string& componentType,
		const std::string& componentName
	)
	{
		if (componentType == "MeshComponent")
		{
			return actor.createComponent<MeshComponent>(std::shared_ptr<GLframework::Mesh>{}, componentName);
		}
		if (componentType == "LightComponent")
		{
			return actor.createComponent<LightComponent>(std::shared_ptr<GLframework::Light>{}, componentName);
		}
		if (componentType == "CameraComponent")
		{
			return actor.createComponent<CameraComponent>(nullptr, componentName);
		}
		if (componentType == "LegacyObjectComponent")
		{
			return actor.createComponent<LegacyObjectComponent>(std::shared_ptr<GLframework::Object>{}, componentName);
		}
		return actor.createComponent<SceneComponent>(componentName);
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

	void writeVec3(std::ostream& output, const std::string& prefix, const Vector3& value)
	{
		output << prefix << "X=" << value.x << '\n';
		output << prefix << "Y=" << value.y << '\n';
		output << prefix << "Z=" << value.z << '\n';
	}

	bool readScenePackageKeyValues(
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
				error = "invalid scene package line " + std::to_string(lineNumber);
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
		Vector3& value
	)
	{
		return getFloatValue(values, prefix + "X", value.x)
			&& getFloatValue(values, prefix + "Y", value.y)
			&& getFloatValue(values, prefix + "Z", value.z);
	}

	bool getTransformValue(
		const std::map<std::string, std::string>& values,
		const std::string& prefix,
		Transform& transform
	)
	{
		return getVec3Value(values, prefix + "location", transform.location)
			&& getVec3Value(values, prefix + "rotation", transform.rotation)
			&& getVec3Value(values, prefix + "scale", transform.scale);
	}

	bool hasAnyTransformValue(
		const std::map<std::string, std::string>& values,
		const std::string& prefix
	)
	{
		const char* suffixes[] = {
			"locationX", "locationY", "locationZ",
			"rotationX", "rotationY", "rotationZ",
			"scaleX", "scaleY", "scaleZ"
		};
		for (const auto* suffix : suffixes)
		{
			if (values.find(prefix + suffix) != values.end())
			{
				return true;
			}
		}
		return false;
	}

	std::vector<SavedSceneComponentRef> collectSceneComponents(const Actor& actor)
	{
		std::vector<SavedSceneComponentRef> sceneComponents{};
		int sceneComponentIndex = 0;
		for (const auto& component : actor.getComponents())
		{
			auto* sceneComponent = component
				? dynamic_cast<SceneComponent*>(component.get())
				: nullptr;
			if (!sceneComponent)
			{
				continue;
			}

			sceneComponents.push_back({ sceneComponent, sceneComponentIndex });
			++sceneComponentIndex;
		}
		return sceneComponents;
	}

	int findSceneComponentIndex(
		const std::vector<SavedSceneComponentRef>& sceneComponents,
		const SceneComponent* target
	)
	{
		if (!target)
		{
			return -1;
		}

		for (const auto& ref : sceneComponents)
		{
			if (ref.component == target)
			{
				return ref.index;
			}
		}
		return -1;
	}

	int countSceneComponents(const Level& level)
	{
		int count = 0;
		for (const auto& actor : level.getActors())
		{
			if (!actor)
			{
				continue;
			}

			count += static_cast<int>(collectSceneComponents(*actor).size());
		}
		return count;
	}

	void writeAdapterDescriptor(
		std::ostream& output,
		const std::string& componentPrefix,
		const ScenePackageAdapterDescriptor& descriptor
	)
	{
		output << componentPrefix << "type=" << descriptor.componentType << '\n';
		output << componentPrefix << "adapter.kind=" << descriptor.adapterKind << '\n';
		output << componentPrefix << "adapter.assetHandle=" << descriptor.assetHandle << '\n';
		output << componentPrefix << "adapter.assetReference=" << descriptor.assetReference << '\n';
		output << componentPrefix << "adapter.objectName=" << sanitizeConfigValue(descriptor.objectName) << '\n';
		output << componentPrefix << "adapter.objectType=" << descriptor.objectType << '\n';
		output << componentPrefix << "adapter.materialType=" << descriptor.materialType << '\n';
	}

	void writeAssetManifest(
		std::ostream& output,
		const std::vector<ScenePackageAssetManifestEntry>& entries
	)
	{
		output << "assetManifestCount=" << entries.size() << '\n';
		for (std::size_t index = 0; index < entries.size(); ++index)
		{
			const auto& entry = entries[index];
			const std::string prefix = "assetManifest." + std::to_string(index) + ".";
			output << prefix << "handle=" << entry.handle << '\n';
			output << prefix << "kind=" << assetKindToken(entry.kind) << '\n';
			output << prefix << "source=" << sanitizeConfigValue(entry.source) << '\n';
			output << prefix << "name=" << sanitizeConfigValue(entry.name) << '\n';
			output << prefix << "path=" << sanitizeConfigValue(entry.path) << '\n';
			output << prefix << "materialType=" << sanitizeConfigValue(entry.materialType) << '\n';
		}
	}

	AssetKind parseAssetKind(const std::string& kind)
	{
		if (kind == "mesh") return AssetKind::Mesh;
		if (kind == "material") return AssetKind::Material;
		if (kind == "texture") return AssetKind::Texture;
		if (kind == "object") return AssetKind::Object;
		if (kind == "legacy-object") return AssetKind::LegacyObject;
		if (kind == "light") return AssetKind::Light;
		if (kind == "camera") return AssetKind::Camera;
		if (kind == "scene") return AssetKind::Scene;
		return AssetKind::Unknown;
	}

	void readAssetManifestStats(
		const std::map<std::string, std::string>& values,
		const ScenePackageLoadOptions& options,
		ScenePackageLoadResult& result
	)
	{
		int assetManifestCount = 0;
		if (!getIntValue(values, "assetManifestCount", assetManifestCount))
		{
			return;
		}

		for (int index = 0; index < assetManifestCount; ++index)
		{
			const std::string prefix = "assetManifest." + std::to_string(index) + ".";
			std::string handle{};
			if (!getStringValue(values, prefix + "handle", handle) || !isAssetHandle(handle))
			{
				continue;
			}

			++result.loadedAssetManifestCount;
			std::string kindToken{};
			getStringValue(values, prefix + "kind", kindToken);
			const AssetKind kind = parseAssetKind(kindToken);
			std::string source{};
			std::string name{};
			std::string path{};
			std::string materialType{};
			getStringValue(values, prefix + "source", source);
			getStringValue(values, prefix + "name", name);
			getStringValue(values, prefix + "path", path);
			getStringValue(values, prefix + "materialType", materialType);
			if (kind == AssetKind::Mesh)
			{
				++result.loadedMeshAssetManifestCount;
			}
			else if (kind == AssetKind::Material)
			{
				++result.loadedMaterialAssetManifestCount;
			}
			else if (kind == AssetKind::Texture)
			{
				++result.loadedTextureAssetManifestCount;
			}

			if (options.assetRegistry)
			{
				GLengine::AssetDescriptor descriptor{};
				descriptor.handle = GLengine::AssetHandle{ handle };
				descriptor.kind = kind;
				descriptor.source = source;
				descriptor.name = name;
				descriptor.path = path;
				descriptor.materialType = materialType;
				if (options.assetRegistry->registerAsset(descriptor))
				{
					++result.registeredAssetManifestCount;
					if (kind == AssetKind::Mesh)
					{
						++result.registeredMeshAssetManifestCount;
					}
					else if (kind == AssetKind::Material)
					{
						++result.registeredMaterialAssetManifestCount;
					}
					else if (kind == AssetKind::Texture)
					{
						++result.registeredTextureAssetManifestCount;
					}
				}
			}
		}
	}

	ScenePackageAdapterDescriptor readAdapterDescriptor(
		const std::map<std::string, std::string>& values,
		const std::string& componentPrefix,
		const std::string& componentType
	)
	{
		ScenePackageAdapterDescriptor descriptor{};
		descriptor.componentType = componentType.empty() ? "SceneComponent" : componentType;
		getStringValue(values, componentPrefix + "adapter.kind", descriptor.adapterKind);
		getStringValue(values, componentPrefix + "adapter.assetHandle", descriptor.assetHandle);
		getStringValue(values, componentPrefix + "adapter.assetReference", descriptor.assetReference);
		getStringValue(values, componentPrefix + "adapter.objectName", descriptor.objectName);
		getStringValue(values, componentPrefix + "adapter.objectType", descriptor.objectType);
		getStringValue(values, componentPrefix + "adapter.materialType", descriptor.materialType);
		descriptor.hasAdapterReference =
			((!descriptor.assetHandle.empty() && descriptor.assetHandle != "none")
				|| (!descriptor.assetReference.empty() && descriptor.assetReference != "none"))
			&& !descriptor.adapterKind.empty()
			&& descriptor.adapterKind != "none";
		return descriptor;
	}

	bool resolveAdapterReference(
		SceneComponent& sceneComponent,
		const ScenePackageAdapterDescriptor& descriptor,
		const ScenePackageLoadOptions& options
	)
	{
		if (!descriptor.hasAdapterReference || options.assetResolver == nullptr)
		{
			return false;
		}

		if (descriptor.adapterKind == "mesh")
		{
			auto* meshComponent = dynamic_cast<MeshComponent*>(&sceneComponent);
			if (!meshComponent)
			{
				return false;
			}

			auto mesh = options.assetResolver->resolveMesh(descriptor);
			if (!mesh)
			{
				return false;
			}

			meshComponent->setMesh(mesh);
			return true;
		}

		if (descriptor.adapterKind == "light")
		{
			auto* lightComponent = dynamic_cast<LightComponent*>(&sceneComponent);
			if (!lightComponent)
			{
				return false;
			}

			auto light = options.assetResolver->resolveLight(descriptor);
			if (!light)
			{
				return false;
			}

			lightComponent->setLight(light);
			return true;
		}

		if (descriptor.adapterKind == "legacy-object")
		{
			auto* legacyComponent = dynamic_cast<LegacyObjectComponent*>(&sceneComponent);
			if (!legacyComponent)
			{
				return false;
			}

			auto object = options.assetResolver->resolveLegacyObject(descriptor);
			if (!object)
			{
				return false;
			}

			legacyComponent->setObject(object);
			return true;
		}

		return false;
	}
}

std::string GLengine::defaultScenePackagePath()
{
	return "out/engine_world_scene_package.ini";
}

std::shared_ptr<GLframework::Mesh> GLengine::ScenePackageAssetResolver::resolveMesh(
	const ScenePackageAdapterDescriptor&
)
{
	return nullptr;
}

std::shared_ptr<GLframework::Light> GLengine::ScenePackageAssetResolver::resolveLight(
	const ScenePackageAdapterDescriptor&
)
{
	return nullptr;
}

std::shared_ptr<GLframework::Object> GLengine::ScenePackageAssetResolver::resolveLegacyObject(
	const ScenePackageAdapterDescriptor&
)
{
	return nullptr;
}

ScenePackageSaveResult GLengine::saveScenePackage(const World& world, const std::string& path)
{
	ScenePackageSaveResult result{};
	result.path = path;
	if (path.empty())
	{
		result.error = "empty scene package path";
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

	result.actorCount = static_cast<int>(level->getActors().size());
	result.sceneComponentCount = countSceneComponents(*level);

	std::ofstream output(path, std::ios::trunc);
	if (!output)
	{
		result.error = "cannot write " + path;
		return result;
	}

	output << "# Engine World scene package\n";
	output << "schema=" << kScenePackageSchema << '\n';
	output << "world.name=" << sanitizeConfigValue(objectNameOrFallback(world, "World")) << '\n';
	output << "world.persistentId=" << world.getPersistentId() << '\n';
	output << "level.name=" << sanitizeConfigValue(objectNameOrFallback(*level, "Persistent Level")) << '\n';
	output << "level.persistentId=" << level->getPersistentId() << '\n';
	output << "actorCount=" << result.actorCount << '\n';
	output << "sceneComponentCount=" << result.sceneComponentCount << '\n';
	output << std::fixed << std::setprecision(6);

	std::vector<ScenePackageAssetManifestEntry> assetManifestEntries{};
	const auto& actors = level->getActors();
	for (std::size_t actorIndex = 0; actorIndex < actors.size(); ++actorIndex)
	{
		const auto& actor = actors[actorIndex];
		if (!actor)
		{
			continue;
		}

		const auto sceneComponents = collectSceneComponents(*actor);
		const int rootSceneComponentIndex = findSceneComponentIndex(sceneComponents, actor->getRootComponent());
		const std::string actorPrefix = "actor." + std::to_string(actorIndex) + ".";
		const std::string actorType = actorTypeName(*actor);
		if (isTypedActorType(actorType))
		{
			++result.typedActorCount;
		}
		output << actorPrefix << "name=" << sanitizeConfigValue(objectNameOrFallback(*actor, "Actor")) << '\n';
		output << actorPrefix << "type=" << actorType << '\n';
		output << actorPrefix << "persistentId=" << actor->getPersistentId() << '\n';
		output << actorPrefix << "sceneComponentCount=" << sceneComponents.size() << '\n';
		output << actorPrefix << "rootSceneComponent=" << rootSceneComponentIndex << '\n';

		for (const auto& sceneComponentRef : sceneComponents)
		{
			const auto* sceneComponent = sceneComponentRef.component;
			if (!sceneComponent)
			{
				continue;
			}

			const auto& transform = sceneComponent->getRelativeTransform();
			const int parentIndex = findSceneComponentIndex(sceneComponents, sceneComponent->getParent());
			const std::string componentPrefix =
				actorPrefix + "sceneComponent." + std::to_string(sceneComponentRef.index) + ".";
			const ScenePackageAdapterDescriptor adapterDescriptor = describeSceneComponentAdapter(*sceneComponent);
			if (adapterDescriptor.componentType != "SceneComponent")
			{
				++result.typedSceneComponentCount;
			}
			if (adapterDescriptor.hasAdapterReference)
			{
				++result.adapterDescriptorCount;
			}
			if (!adapterDescriptor.assetHandle.empty() && adapterDescriptor.assetHandle != "none")
			{
				++result.assetHandleCount;
			}
			collectImportedAssetManifestEntries(*sceneComponent, assetManifestEntries, result);

			output << componentPrefix << "name="
				<< sanitizeConfigValue(objectNameOrFallback(*sceneComponent, "SceneComponent")) << '\n';
			writeAdapterDescriptor(output, componentPrefix, adapterDescriptor);
			output << componentPrefix << "persistentId=" << sceneComponent->getPersistentId() << '\n';
			output << componentPrefix << "parentSceneComponent=" << parentIndex << '\n';
			output << componentPrefix << "parentPersistentId="
				<< (sceneComponent->getParent() ? sceneComponent->getParent()->getPersistentId() : std::string{}) << '\n';
			writeVec3(output, componentPrefix + "location", transform.location);
			writeVec3(output, componentPrefix + "rotation", transform.rotation);
			writeVec3(output, componentPrefix + "scale", transform.scale);
		}
	}
	writeAssetManifest(output, assetManifestEntries);

	result.saved = true;
	return result;
}

ScenePackageLoadResult GLengine::loadScenePackage(const std::string& path)
{
	ScenePackageLoadOptions options{};
	return loadScenePackage(path, options);
}

ScenePackageLoadResult GLengine::loadScenePackage(
	const std::string& path,
	const ScenePackageLoadOptions& options
)
{
	ScenePackageLoadResult result{};
	result.path = path;
	if (path.empty())
	{
		result.error = "empty scene package path";
		return result;
	}

	std::map<std::string, std::string> values{};
	std::string error{};
	if (!readScenePackageKeyValues(path, values, error))
	{
		result.error = error;
		return result;
	}

	std::string schema{};
	if (!getStringValue(values, "schema", schema) || schema != kScenePackageSchema)
	{
		result.error = "unsupported or missing scene package schema";
		return result;
	}

	int actorCount = 0;
	int sceneComponentCount = 0;
	if (!getIntValue(values, "actorCount", actorCount))
	{
		result.error = "missing actorCount";
		return result;
	}
	if (!getIntValue(values, "sceneComponentCount", sceneComponentCount))
	{
		result.error = "missing sceneComponentCount";
		return result;
	}
	readAssetManifestStats(values, options, result);

	std::string worldName{};
	getStringValue(values, "world.name", worldName);
	auto loadedWorld = std::make_unique<World>(worldName.empty() ? "World" : worldName);
	std::string worldPersistentId{};
	if (getStringValue(values, "world.persistentId", worldPersistentId))
	{
		loadedWorld->setPersistentId(worldPersistentId);
	}

	auto* level = loadedWorld->getPersistentLevel();
	if (!level)
	{
		result.error = "loaded world has no persistent level";
		return result;
	}

	std::string levelName{};
	if (getStringValue(values, "level.name", levelName) && !levelName.empty())
	{
		level->setName(levelName);
	}
	std::string levelPersistentId{};
	if (getStringValue(values, "level.persistentId", levelPersistentId))
	{
		level->setPersistentId(levelPersistentId);
	}

	std::vector<std::vector<SceneComponent*>> loadedSceneComponents{};
	loadedSceneComponents.resize(static_cast<std::size_t>(std::max(actorCount, 0)));
	std::unordered_map<std::string, SceneComponent*> sceneComponentsByPersistentId{};
	std::unordered_set<std::string> actorPersistentIds{};
	std::unordered_set<std::string> sceneComponentPersistentIds{};
	std::unordered_map<SceneComponent*, int> sceneComponentOwnerActorIndex{};

	for (int actorIndex = 0; actorIndex < actorCount; ++actorIndex)
	{
		const std::string actorPrefix = "actor." + std::to_string(actorIndex) + ".";
		std::string actorName{};
		getStringValue(values, actorPrefix + "name", actorName);
		std::string actorType{};
		getStringValue(values, actorPrefix + "type", actorType);
		if (actorType.empty())
		{
			actorType = "Actor";
		}
		if (!isKnownActorType(actorType))
		{
			++result.unknownActorTypes;
		}
		auto& actor = createActorFromType(
			*level,
			actorType,
			actorName.empty() ? "Actor" : actorName
		);
		if (isTypedActorType(actorType) && actorTypeName(actor) == actorType)
		{
			++result.loadedTypedActors;
		}

		std::string actorPersistentId{};
		if (getStringValue(values, actorPrefix + "persistentId", actorPersistentId))
		{
			actor.setPersistentId(actorPersistentId);
			if (!actorPersistentId.empty())
			{
				if (!actorPersistentIds.insert(actorPersistentId).second)
				{
					++result.duplicateActorPersistentIds;
				}
				++result.actorsWithPersistentIds;
			}
		}

		int actorSceneComponentCount = 0;
		getIntValue(values, actorPrefix + "sceneComponentCount", actorSceneComponentCount);
		auto& actorSceneComponents = loadedSceneComponents[static_cast<std::size_t>(actorIndex)];
		actorSceneComponents.resize(static_cast<std::size_t>(std::max(actorSceneComponentCount, 0)), nullptr);

		for (int sceneComponentIndex = 0; sceneComponentIndex < actorSceneComponentCount; ++sceneComponentIndex)
		{
			const std::string componentPrefix =
				actorPrefix + "sceneComponent." + std::to_string(sceneComponentIndex) + ".";
			std::string componentName{};
			getStringValue(values, componentPrefix + "name", componentName);
			std::string componentType{};
			getStringValue(values, componentPrefix + "type", componentType);
			if (componentType.empty())
			{
				componentType = "SceneComponent";
			}
			const bool knownComponentType = isKnownSceneComponentType(componentType);
			if (!knownComponentType)
			{
				++result.unknownSceneComponentTypes;
			}
			const std::string resolvedComponentName = componentName.empty() ? "SceneComponent" : componentName;
			auto* reusedSceneComponent = tryReuseActorRootComponent(
				actor,
				componentType,
				resolvedComponentName,
				sceneComponentIndex
			);
			auto& sceneComponent = reusedSceneComponent
				? *reusedSceneComponent
				: createSceneComponentFromType(
					actor,
					componentType,
					resolvedComponentName
				);
			if (componentType != "SceneComponent" && knownComponentType && sceneComponentMatchesType(sceneComponent, componentType))
			{
				++result.loadedTypedSceneComponents;
			}

			const ScenePackageAdapterDescriptor adapterDescriptor = readAdapterDescriptor(
				values,
				componentPrefix,
				componentType
			);
			if (adapterDescriptor.hasAdapterReference)
			{
				++result.loadedAdapterReferences;
				const bool hasStableAssetHandle =
					!adapterDescriptor.assetHandle.empty()
					&& adapterDescriptor.assetHandle != "none";
				if (hasStableAssetHandle)
				{
					++result.loadedAssetHandles;
				}
				if (resolveAdapterReference(sceneComponent, adapterDescriptor, options))
				{
					++result.resolvedAdapterReferences;
					if (hasStableAssetHandle)
					{
						++result.resolvedAssetHandles;
					}
				}
				else
				{
					++result.unresolvedAdapterReferences;
					if (hasStableAssetHandle)
					{
						++result.unresolvedAssetHandles;
					}
				}
			}

			std::string sceneComponentPersistentId{};
			if (getStringValue(values, componentPrefix + "persistentId", sceneComponentPersistentId))
			{
				sceneComponent.setPersistentId(sceneComponentPersistentId);
				if (!sceneComponentPersistentId.empty())
				{
					if (sceneComponentPersistentIds.insert(sceneComponentPersistentId).second)
					{
						sceneComponentsByPersistentId[sceneComponentPersistentId] = &sceneComponent;
					}
					else
					{
						++result.duplicateSceneComponentPersistentIds;
					}
					++result.sceneComponentsWithPersistentIds;
				}
			}

			Transform transform{};
			if (getTransformValue(values, componentPrefix, transform))
			{
				sceneComponent.setRelativeTransform(transform);
			}
			else if (hasAnyTransformValue(values, componentPrefix))
			{
				++result.invalidSceneComponentTransforms;
			}

			actorSceneComponents[static_cast<std::size_t>(sceneComponentIndex)] = &sceneComponent;
			sceneComponentOwnerActorIndex[&sceneComponent] = actorIndex;
			++result.sceneComponentCount;
		}

		int rootSceneComponentIndex = -1;
		if (getIntValue(values, actorPrefix + "rootSceneComponent", rootSceneComponentIndex))
		{
			if (rootSceneComponentIndex >= 0
				&& rootSceneComponentIndex < static_cast<int>(actorSceneComponents.size())
				&& actorSceneComponents[static_cast<std::size_t>(rootSceneComponentIndex)])
			{
				actor.setRootComponent(actorSceneComponents[static_cast<std::size_t>(rootSceneComponentIndex)]);
				++result.restoredRootComponents;
			}
			else
			{
				++result.invalidRootSceneComponentIndices;
			}
		}

		++result.actorCount;
	}

	if (result.duplicateActorPersistentIds > 0
		|| result.duplicateSceneComponentPersistentIds > 0
		|| result.invalidRootSceneComponentIndices > 0)
	{
		result.error = "invalid scene package graph";
		return result;
	}

	std::vector<PendingSceneComponentAttachment> pendingAttachments{};
	for (int actorIndex = 0; actorIndex < actorCount; ++actorIndex)
	{
		const std::string actorPrefix = "actor." + std::to_string(actorIndex) + ".";
		int actorSceneComponentCount = 0;
		getIntValue(values, actorPrefix + "sceneComponentCount", actorSceneComponentCount);
		for (int sceneComponentIndex = 0; sceneComponentIndex < actorSceneComponentCount; ++sceneComponentIndex)
		{
			const std::string componentPrefix =
				actorPrefix + "sceneComponent." + std::to_string(sceneComponentIndex) + ".";
			auto* sceneComponent = loadedSceneComponents[static_cast<std::size_t>(actorIndex)][static_cast<std::size_t>(sceneComponentIndex)];
			if (!sceneComponent)
			{
				continue;
			}

			SceneComponent* parent = nullptr;
			int parentActorIndex = -1;
			std::string parentPersistentId{};
			const bool hasParentPersistentId =
				getStringValue(values, componentPrefix + "parentPersistentId", parentPersistentId)
				&& !parentPersistentId.empty();
			if (hasParentPersistentId)
			{
				const auto foundParent = sceneComponentsByPersistentId.find(parentPersistentId);
				if (foundParent != sceneComponentsByPersistentId.end())
				{
					parent = foundParent->second;
					const auto foundOwner = sceneComponentOwnerActorIndex.find(parent);
					if (foundOwner != sceneComponentOwnerActorIndex.end())
					{
						parentActorIndex = foundOwner->second;
					}
				}
				else
				{
					++result.unresolvedParentReferences;
				}
			}
			else
			{
				int parentSceneComponentIndex = -1;
				if (getIntValue(values, componentPrefix + "parentSceneComponent", parentSceneComponentIndex))
				{
					if (parentSceneComponentIndex >= 0
						&& parentSceneComponentIndex < static_cast<int>(loadedSceneComponents[static_cast<std::size_t>(actorIndex)].size())
						&& loadedSceneComponents[static_cast<std::size_t>(actorIndex)][static_cast<std::size_t>(parentSceneComponentIndex)])
					{
						parent = loadedSceneComponents[static_cast<std::size_t>(actorIndex)][static_cast<std::size_t>(parentSceneComponentIndex)];
						parentActorIndex = actorIndex;
					}
					else if (parentSceneComponentIndex != -1)
					{
						++result.invalidParentSceneComponentIndices;
					}
				}
			}

			if (parent)
			{
				if (parent == sceneComponent)
				{
					++result.selfParentReferences;
				}
				else
				{
					pendingAttachments.push_back(PendingSceneComponentAttachment{
						sceneComponent,
						parent,
						actorIndex,
						parentActorIndex
					});
				}
			}
		}
	}

	result.cyclicParentReferences = countCyclicSceneComponentParentReferences(pendingAttachments);
	if (result.unresolvedParentReferences > 0
		|| result.invalidParentSceneComponentIndices > 0
		|| result.selfParentReferences > 0
		|| result.cyclicParentReferences > 0)
	{
		result.error = "invalid scene package graph";
		return result;
	}

	for (const auto& attachment : pendingAttachments)
	{
		attachment.child->attachTo(attachment.parent);
		++result.restoredSceneComponentParents;
		if (attachment.parentActorIndex >= 0 && attachment.childActorIndex != attachment.parentActorIndex)
		{
			++result.restoredCrossActorParentReferences;
		}
	}

	result.loaded = result.actorCount == actorCount && result.sceneComponentCount == sceneComponentCount;
	if (!result.loaded)
	{
		result.error = "scene package loaded with mismatched object counts";
		return result;
	}

	result.world = std::move(loadedWorld);
	return result;
}

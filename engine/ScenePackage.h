#pragma once

#include <memory>
#include <string>

namespace GLframework
{
	class Light;
	class Mesh;
	class Object;
}

namespace GLengine
{
	class AssetRegistry;
	class World;

	struct ScenePackageAdapterDescriptor
	{
		std::string componentType{ "SceneComponent" };
		std::string adapterKind{ "none" };
		std::string assetHandle{ "none" };
		std::string assetReference{ "none" };
		std::string objectName{};
		std::string objectType{};
		std::string materialType{};
		bool hasAdapterReference{ false };
	};

	class ScenePackageAssetResolver
	{
	public:
		virtual ~ScenePackageAssetResolver() = default;

		virtual std::shared_ptr<GLframework::Mesh> resolveMesh(const ScenePackageAdapterDescriptor& descriptor);
		virtual std::shared_ptr<GLframework::Light> resolveLight(const ScenePackageAdapterDescriptor& descriptor);
		virtual std::shared_ptr<GLframework::Object> resolveLegacyObject(const ScenePackageAdapterDescriptor& descriptor);
	};

	struct ScenePackageLoadOptions
	{
		ScenePackageAssetResolver* assetResolver{ nullptr };
		AssetRegistry* assetRegistry{ nullptr };
	};

	struct ScenePackageSaveResult
	{
		bool saved{ false };
		int actorCount{ 0 };
		int sceneComponentCount{ 0 };
		int typedActorCount{ 0 };
		int typedSceneComponentCount{ 0 };
		int adapterDescriptorCount{ 0 };
		int assetHandleCount{ 0 };
		int assetManifestCount{ 0 };
		int meshAssetManifestCount{ 0 };
		int materialAssetManifestCount{ 0 };
		int textureAssetManifestCount{ 0 };
		std::string path{};
		std::string error{};
	};

	struct ScenePackageLoadResult
	{
		ScenePackageLoadResult();
		~ScenePackageLoadResult();
		ScenePackageLoadResult(ScenePackageLoadResult&&) noexcept;
		ScenePackageLoadResult& operator=(ScenePackageLoadResult&&) noexcept;
		ScenePackageLoadResult(const ScenePackageLoadResult&) = delete;
		ScenePackageLoadResult& operator=(const ScenePackageLoadResult&) = delete;

		bool loaded{ false };
		std::unique_ptr<World> world{ nullptr };
		int actorCount{ 0 };
		int sceneComponentCount{ 0 };
		int loadedTypedActors{ 0 };
		int actorsWithPersistentIds{ 0 };
		int sceneComponentsWithPersistentIds{ 0 };
		int loadedTypedSceneComponents{ 0 };
		int unknownActorTypes{ 0 };
		int unknownSceneComponentTypes{ 0 };
		int invalidSceneComponentTransforms{ 0 };
		int duplicateActorPersistentIds{ 0 };
		int duplicateSceneComponentPersistentIds{ 0 };
		int invalidRootSceneComponentIndices{ 0 };
		int unresolvedParentReferences{ 0 };
		int invalidParentSceneComponentIndices{ 0 };
		int selfParentReferences{ 0 };
		int cyclicParentReferences{ 0 };
		int loadedAdapterReferences{ 0 };
		int loadedAssetHandles{ 0 };
		int resolvedAdapterReferences{ 0 };
		int resolvedAssetHandles{ 0 };
		int unresolvedAdapterReferences{ 0 };
		int unresolvedAssetHandles{ 0 };
		int loadedAssetManifestCount{ 0 };
		int loadedMeshAssetManifestCount{ 0 };
		int loadedMaterialAssetManifestCount{ 0 };
		int loadedTextureAssetManifestCount{ 0 };
		int registeredAssetManifestCount{ 0 };
		int registeredMeshAssetManifestCount{ 0 };
		int registeredMaterialAssetManifestCount{ 0 };
		int registeredTextureAssetManifestCount{ 0 };
		int restoredRootComponents{ 0 };
		int restoredSceneComponentParents{ 0 };
		int restoredCrossActorParentReferences{ 0 };
		std::string path{};
		std::string error{};
	};

	std::string defaultScenePackagePath();
	ScenePackageSaveResult saveScenePackage(const World& world, const std::string& path);
	ScenePackageLoadResult loadScenePackage(const std::string& path);
	ScenePackageLoadResult loadScenePackage(const std::string& path, const ScenePackageLoadOptions& options);
}

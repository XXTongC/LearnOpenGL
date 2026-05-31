#pragma once

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace GLengine
{
	enum class AssetKind
	{
		Unknown,
		Mesh,
		Material,
		Texture,
		Object,
		LegacyObject,
		Light,
		Camera,
		Scene
	};

	struct AssetHandle
	{
		std::string value{};

		bool isValid() const;
	};

	struct AssetDescriptor
	{
		AssetHandle handle{};
		AssetKind kind{ AssetKind::Unknown };
		std::string source{};
		std::string name{};
		std::string path{};
		std::string materialType{};
	};

	class AssetRegistry
	{
	public:
		bool registerAsset(const AssetDescriptor& descriptor);
		const AssetDescriptor* find(const AssetHandle& handle) const;
		bool contains(const AssetHandle& handle) const;
		std::vector<AssetDescriptor> listAssets() const;
		int count() const;
		int countByKind(AssetKind kind) const;
		void clear();

	private:
		std::unordered_map<std::string, AssetDescriptor> mAssets{};
	};

	std::string_view assetKindToken(AssetKind kind);
	std::string sanitizeAssetHandleSegment(std::string_view value, std::string_view fallback = "unnamed");
	AssetHandle makeAssetHandle(
		AssetKind kind,
		std::string_view source,
		std::initializer_list<std::string_view> pathSegments
	);
	bool isAssetHandle(std::string_view value);
	bool assetHandleMatches(std::string_view value, AssetKind kind, std::string_view source);
}

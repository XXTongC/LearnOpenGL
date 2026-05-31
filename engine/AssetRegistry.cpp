#include "engine/AssetRegistry.h"

#include <algorithm>
#include <cctype>

using namespace GLengine;

namespace
{
	bool startsWith(std::string_view value, std::string_view prefix)
	{
		return value.size() >= prefix.size()
			&& value.compare(0, prefix.size(), prefix) == 0;
	}
}

bool AssetHandle::isValid() const
{
	return isAssetHandle(value);
}

bool AssetRegistry::registerAsset(const AssetDescriptor& descriptor)
{
	if (!descriptor.handle.isValid())
	{
		return false;
	}

	mAssets[descriptor.handle.value] = descriptor;
	return true;
}

const AssetDescriptor* AssetRegistry::find(const AssetHandle& handle) const
{
	const auto found = mAssets.find(handle.value);
	return found == mAssets.end() ? nullptr : &found->second;
}

bool AssetRegistry::contains(const AssetHandle& handle) const
{
	return find(handle) != nullptr;
}

std::vector<AssetDescriptor> AssetRegistry::listAssets() const
{
	std::vector<AssetDescriptor> assets{};
	assets.reserve(mAssets.size());
	for (const auto& entry : mAssets)
	{
		assets.push_back(entry.second);
	}

	std::sort(
		assets.begin(),
		assets.end(),
		[](const AssetDescriptor& left, const AssetDescriptor& right)
		{
			return left.handle.value < right.handle.value;
		}
	);
	return assets;
}

int AssetRegistry::count() const
{
	return static_cast<int>(mAssets.size());
}

int AssetRegistry::countByKind(AssetKind kind) const
{
	return static_cast<int>(std::count_if(
		mAssets.begin(),
		mAssets.end(),
		[kind](const auto& entry)
		{
			return entry.second.kind == kind;
		}
	));
}

void AssetRegistry::clear()
{
	mAssets.clear();
}

std::string_view GLengine::assetKindToken(AssetKind kind)
{
	switch (kind)
	{
	case AssetKind::Mesh:
		return "mesh";
	case AssetKind::Material:
		return "material";
	case AssetKind::Texture:
		return "texture";
	case AssetKind::Object:
		return "object";
	case AssetKind::LegacyObject:
		return "legacy-object";
	case AssetKind::Light:
		return "light";
	case AssetKind::Camera:
		return "camera";
	case AssetKind::Scene:
		return "scene";
	default:
		return "unknown";
	}
}

std::string GLengine::sanitizeAssetHandleSegment(std::string_view value, std::string_view fallback)
{
	if (value.empty())
	{
		value = fallback;
	}

	std::string result{};
	result.reserve(value.size());
	for (const unsigned char ch : value)
	{
		if (std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.')
		{
			result.push_back(static_cast<char>(std::tolower(ch)));
		}
		else
		{
			result.push_back('-');
		}
	}

	while (!result.empty() && result.back() == '-')
	{
		result.pop_back();
	}
	return result.empty() ? std::string{ fallback } : result;
}

AssetHandle GLengine::makeAssetHandle(
	AssetKind kind,
	std::string_view source,
	std::initializer_list<std::string_view> pathSegments
)
{
	std::string value = "asset:";
	value += assetKindToken(kind);
	value += ":";
	value += sanitizeAssetHandleSegment(source, "runtime");

	bool hasPath = false;
	for (const std::string_view segment : pathSegments)
	{
		if (segment.empty())
		{
			continue;
		}

		value += ":";
		value += sanitizeAssetHandleSegment(segment);
		hasPath = true;
	}

	if (!hasPath)
	{
		value += ":unnamed";
	}
	return AssetHandle{ value };
}

bool GLengine::isAssetHandle(std::string_view value)
{
	return startsWith(value, "asset:");
}

bool GLengine::assetHandleMatches(std::string_view value, AssetKind kind, std::string_view source)
{
	std::string prefix = "asset:";
	prefix += assetKindToken(kind);
	prefix += ":";
	prefix += sanitizeAssetHandleSegment(source, "runtime");
	prefix += ":";
	return startsWith(value, prefix);
}

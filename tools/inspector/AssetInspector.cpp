#include "AssetInspector.h"

#include "../../engine/AssetRegistry.h"

namespace GL_EDITOR
{
	bool isImportedAsset(const GLengine::AssetDescriptor& asset)
	{
		return asset.source == "imported-asset";
	}

	std::string getAssetDisplayName(const GLengine::AssetDescriptor& asset)
	{
		return asset.name.empty() ? asset.handle.value : asset.name;
	}

	PropertyBuilder buildAssetPropertySchema(const GLengine::AssetDescriptor& asset)
	{
		PropertyBuilder builder{};
		builder.addSection("Asset");
		builder.addReadOnlyString("Name", getAssetDisplayName(asset));
		builder.addReadOnlyString("Kind", std::string(GLengine::assetKindToken(asset.kind)));
		builder.addReadOnlyString("Handle", asset.handle.value);
		builder.addReadOnlyString("Source", asset.source.empty() ? "unknown" : asset.source);
		builder.addReadOnlyString("Path", asset.path.empty() ? "unknown" : asset.path);
		if (!asset.materialType.empty())
		{
			builder.addReadOnlyString("Material Type", asset.materialType);
		}
		return builder;
	}
}

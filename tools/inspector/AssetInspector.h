#pragma once

#include <string>

#include "PropertySchema.h"

namespace GLengine
{
	struct AssetDescriptor;
}

namespace GL_EDITOR
{
	bool isImportedAsset(const GLengine::AssetDescriptor& asset);
	std::string getAssetDisplayName(const GLengine::AssetDescriptor& asset);
	PropertyBuilder buildAssetPropertySchema(const GLengine::AssetDescriptor& asset);
}

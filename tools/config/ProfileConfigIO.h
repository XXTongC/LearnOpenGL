#pragma once

#include <string>

#include "../inspector/PropertySchema.h"

namespace GL_CONFIG
{
	bool applyPropertyConfigValue(
		const std::string& key,
		const std::string& value,
		const GL_EDITOR::PropertyBuilder& builder
	);
	bool loadPropertyConfig(const std::string& path, const GL_EDITOR::PropertyBuilder& builder);
	bool savePropertyConfig(
		const std::string& path,
		const std::string& headerComment,
		const GL_EDITOR::PropertyBuilder& builder
	);
}

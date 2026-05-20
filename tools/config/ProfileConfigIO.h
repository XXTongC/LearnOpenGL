#pragma once

#include <string>

#include "../inspector/PropertySchema.h"

namespace GL_CONFIG
{
	bool loadPropertyConfig(const std::string& path, const GL_EDITOR::PropertyBuilder& builder);
	bool savePropertyConfig(
		const std::string& path,
		const std::string& headerComment,
		const GL_EDITOR::PropertyBuilder& builder
	);
}

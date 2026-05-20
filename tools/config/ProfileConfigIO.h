#pragma once

#include <ostream>
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
	void writePropertyConfig(
		std::ostream& output,
		const std::string& prefix,
		const GL_EDITOR::PropertyBuilder& builder
	);
	bool savePropertyConfig(
		const std::string& path,
		const std::string& headerComment,
		const GL_EDITOR::PropertyBuilder& builder
	);
}

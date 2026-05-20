#pragma once

#include <functional>
#include <string>

namespace GL_CONFIG
{
	using KeyValueVisitor = std::function<void(const std::string& key, const std::string& value)>;

	std::string trim(std::string value);
	bool startsWith(const std::string& value, const std::string& prefix);
	bool readKeyValueFile(const std::string& path, const KeyValueVisitor& visitor);

	bool parseBool(std::string value, bool& output);
	bool parseFloat(const std::string& value, float& output);
	bool parseInt(const std::string& value, int& output);
	bool parseUnsigned(const std::string& value, unsigned int& output);
}

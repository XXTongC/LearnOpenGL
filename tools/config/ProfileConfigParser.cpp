#include "ProfileConfigParser.h"

#include <algorithm>
#include <cctype>
#include <fstream>

std::string GL_CONFIG::trim(std::string value)
{
	auto isSpace = [](unsigned char ch)
	{
		return std::isspace(ch) != 0;
	};

	value.erase(value.begin(), std::find_if(value.begin(), value.end(), [isSpace](char ch)
	{
		return !isSpace(static_cast<unsigned char>(ch));
	}));
	value.erase(std::find_if(value.rbegin(), value.rend(), [isSpace](char ch)
	{
		return !isSpace(static_cast<unsigned char>(ch));
	}).base(), value.end());
	return value;
}

bool GL_CONFIG::startsWith(const std::string& value, const std::string& prefix)
{
	return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

bool GL_CONFIG::readKeyValueFile(const std::string& path, const KeyValueVisitor& visitor)
{
	std::ifstream input(path);
	if (!input)
	{
		return false;
	}

	std::string line{};
	while (std::getline(input, line))
	{
		line = trim(line);
		if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[')
		{
			continue;
		}

		const auto separator = line.find('=');
		if (separator == std::string::npos)
		{
			continue;
		}

		visitor(trim(line.substr(0, separator)), trim(line.substr(separator + 1)));
	}

	return true;
}

bool GL_CONFIG::parseBool(std::string value, bool& output)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
	{
		return static_cast<char>(std::tolower(ch));
	});

	if (value == "1" || value == "true" || value == "yes" || value == "on")
	{
		output = true;
		return true;
	}

	if (value == "0" || value == "false" || value == "no" || value == "off")
	{
		output = false;
		return true;
	}

	return false;
}

bool GL_CONFIG::parseFloat(const std::string& value, float& output)
{
	try
	{
		size_t parsedCharacters{ 0 };
		const auto parsed = std::stof(value, &parsedCharacters);
		if (parsedCharacters != value.size())
		{
			return false;
		}

		output = parsed;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool GL_CONFIG::parseInt(const std::string& value, int& output)
{
	try
	{
		size_t parsedCharacters{ 0 };
		const auto parsed = std::stoi(value, &parsedCharacters);
		if (parsedCharacters != value.size())
		{
			return false;
		}

		output = parsed;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool GL_CONFIG::parseUnsigned(const std::string& value, unsigned int& output)
{
	try
	{
		size_t parsedCharacters{ 0 };
		const auto parsed = std::stoul(value, &parsedCharacters);
		if (parsedCharacters != value.size())
		{
			return false;
		}

		output = static_cast<unsigned int>(parsed);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

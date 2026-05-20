#include "ProfileConfigIO.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "ProfileConfigParser.h"

namespace
{
	bool applyPropertyValue(const GL_EDITOR::PropertyDescriptor& property, const std::string& value)
	{
		switch (property.kind)
		{
		case GL_EDITOR::PropertyKind::Float:
		{
			float parsedValue{ property.getFloat ? property.getFloat() : 0.0f };
			if (GL_CONFIG::parseFloat(value, parsedValue) && property.setFloat)
			{
				property.setFloat(parsedValue);
			}
			return true;
		}

		case GL_EDITOR::PropertyKind::Int:
		{
			int parsedValue{ property.getInt ? property.getInt() : 0 };
			if (GL_CONFIG::parseInt(value, parsedValue) && property.setInt)
			{
				property.setInt(parsedValue);
			}
			return true;
		}

		case GL_EDITOR::PropertyKind::Bool:
		{
			bool parsedValue{ property.getBool ? property.getBool() : false };
			if (GL_CONFIG::parseBool(value, parsedValue) && property.setBool)
			{
				property.setBool(parsedValue);
			}
			return true;
		}

		case GL_EDITOR::PropertyKind::String:
			if (property.setText)
			{
				property.setText(value);
			}
			return true;

		default:
			return false;
		}
	}

	std::string propertyValueToString(const GL_EDITOR::PropertyDescriptor& property)
	{
		std::ostringstream stream{};
		switch (property.kind)
		{
		case GL_EDITOR::PropertyKind::Float:
			stream << (property.getFloat ? property.getFloat() : 0.0f);
			return stream.str();

		case GL_EDITOR::PropertyKind::Int:
			stream << (property.getInt ? property.getInt() : 0);
			return stream.str();

		case GL_EDITOR::PropertyKind::Bool:
			return property.getBool && property.getBool() ? "1" : "0";

		case GL_EDITOR::PropertyKind::String:
			return property.getText ? property.getText() : "";

		default:
			return "";
		}
	}

	bool canSaveProperty(const GL_EDITOR::PropertyDescriptor& property)
	{
		return !property.configKey.empty()
			&& (
				property.kind == GL_EDITOR::PropertyKind::Float
				|| property.kind == GL_EDITOR::PropertyKind::Int
				|| property.kind == GL_EDITOR::PropertyKind::Bool
				|| property.kind == GL_EDITOR::PropertyKind::String
			);
	}
}

bool GL_CONFIG::loadPropertyConfig(const std::string& path, const GL_EDITOR::PropertyBuilder& builder)
{
	return readKeyValueFile(path, [&builder](const std::string& key, const std::string& value)
	{
		for (const auto& property : builder.getProperties())
		{
			if (property.configKey == key)
			{
				applyPropertyValue(property, value);
				return;
			}
		}
	});
}

bool GL_CONFIG::savePropertyConfig(
	const std::string& path,
	const std::string& headerComment,
	const GL_EDITOR::PropertyBuilder& builder
)
{
	const std::filesystem::path filePath{ path };
	const auto parentPath = filePath.parent_path();
	if (!parentPath.empty())
	{
		std::error_code error{};
		std::filesystem::create_directories(parentPath, error);
		if (error)
		{
			return false;
		}
	}

	std::ofstream output(path, std::ios::trunc);
	if (!output)
	{
		return false;
	}

	if (!headerComment.empty())
	{
		output << headerComment << '\n';
	}

	for (const auto& property : builder.getProperties())
	{
		if (canSaveProperty(property))
		{
			output << property.configKey << '=' << propertyValueToString(property) << '\n';
		}
	}

	return true;
}

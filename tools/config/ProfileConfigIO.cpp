#include "ProfileConfigIO.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "ProfileConfigParser.h"

namespace
{
	constexpr size_t invalidConfigKeyIndex = static_cast<size_t>(-1);
	using Vec3Index = glm::vec3::length_type;

	size_t findConfigKeyIndex(const GL_EDITOR::PropertyDescriptor& property, const std::string& key)
	{
		for (size_t index = 0; index < property.configKeys.size(); ++index)
		{
			if (property.configKeys[index] == key)
			{
				return index;
			}
		}

		if (property.configKey == key)
		{
			return 0;
		}

		return invalidConfigKeyIndex;
	}

	bool applyVec3ComponentValue(
		const GL_EDITOR::PropertyDescriptor& property,
		const std::string& value,
		size_t componentIndex
	)
	{
		if (!property.getVec3 || !property.setVec3 || componentIndex >= 3)
		{
			return false;
		}

		const auto glmIndex = static_cast<Vec3Index>(componentIndex);
		float parsedValue{ property.getVec3()[glmIndex] };
		if (!GL_CONFIG::parseFloat(value, parsedValue))
		{
			return true;
		}

		glm::vec3 currentValue = property.getVec3();
		currentValue[glmIndex] = parsedValue;
		property.setVec3(currentValue);
		return true;
	}

	bool applyPropertyValue(
		const GL_EDITOR::PropertyDescriptor& property,
		const std::string& value,
		size_t configKeyIndex
	)
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

		case GL_EDITOR::PropertyKind::Vec3:
		case GL_EDITOR::PropertyKind::Color3:
			return applyVec3ComponentValue(property, value, configKeyIndex);

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

	bool isScalarConfigProperty(const GL_EDITOR::PropertyDescriptor& property)
	{
		return !property.configKey.empty()
			&& (
				property.kind == GL_EDITOR::PropertyKind::Float
				|| property.kind == GL_EDITOR::PropertyKind::Int
				|| property.kind == GL_EDITOR::PropertyKind::Bool
				|| property.kind == GL_EDITOR::PropertyKind::String
			);
	}

	bool isVec3ConfigProperty(const GL_EDITOR::PropertyDescriptor& property)
	{
		return property.configKeys.size() == 3
			&& property.getVec3
			&& (
				property.kind == GL_EDITOR::PropertyKind::Vec3
				|| property.kind == GL_EDITOR::PropertyKind::Color3
			);
	}

	void writePropertyConfig(std::ofstream& output, const GL_EDITOR::PropertyDescriptor& property)
	{
		if (isScalarConfigProperty(property))
		{
			output << property.configKey << '=' << propertyValueToString(property) << '\n';
			return;
		}

		if (!isVec3ConfigProperty(property))
		{
			return;
		}

		const glm::vec3 value = property.getVec3();
		for (size_t index = 0; index < property.configKeys.size(); ++index)
		{
			output << property.configKeys[index] << '=' << value[static_cast<Vec3Index>(index)] << '\n';
		}
	}
}

bool GL_CONFIG::loadPropertyConfig(const std::string& path, const GL_EDITOR::PropertyBuilder& builder)
{
	return readKeyValueFile(path, [&builder](const std::string& key, const std::string& value)
	{
		applyPropertyConfigValue(key, value, builder);
	});
}

bool GL_CONFIG::applyPropertyConfigValue(
	const std::string& key,
	const std::string& value,
	const GL_EDITOR::PropertyBuilder& builder
)
{
	for (const auto& property : builder.getProperties())
	{
		const size_t configKeyIndex = findConfigKeyIndex(property, key);
		if (configKeyIndex != invalidConfigKeyIndex)
		{
			return applyPropertyValue(property, value, configKeyIndex);
		}
	}

	return false;
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
		writePropertyConfig(output, property);
	}

	return true;
}

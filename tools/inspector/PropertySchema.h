#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "../../third_party/glm/glm.hpp"

namespace GL_EDITOR
{
	enum class PropertyKind
	{
		Section,
		Float,
		Int,
		Bool,
		Vec3,
		Color3,
		String,
		Text,
	};

	struct PropertyDescriptor
	{
		PropertyKind kind{ PropertyKind::Text };
		std::string label{};
		float minValue{ 0.0f };
		float maxValue{ 0.0f };
		std::string format{ "%.3f" };
		size_t stringCapacity{ 512 };
		std::function<float()> getFloat{};
		std::function<void(float)> setFloat{};
		std::function<int()> getInt{};
		std::function<void(int)> setInt{};
		std::function<bool()> getBool{};
		std::function<void(bool)> setBool{};
		std::function<glm::vec3()> getVec3{};
		std::function<void(glm::vec3)> setVec3{};
		std::function<std::string()> getText{};
		std::function<void(const std::string&)> setText{};
	};

	class PropertyBuilder
	{
	public:
		void addSection(std::string label)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::Section;
			descriptor.label = std::move(label);
			mProperties.push_back(std::move(descriptor));
		}

		void addFloat(
			std::string label,
			const std::function<float()>& getter,
			const std::function<void(float)>& setter,
			float minValue,
			float maxValue,
			std::string format = "%.3f"
		)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::Float;
			descriptor.label = std::move(label);
			descriptor.minValue = minValue;
			descriptor.maxValue = maxValue;
			descriptor.format = std::move(format);
			descriptor.getFloat = getter;
			descriptor.setFloat = setter;
			mProperties.push_back(std::move(descriptor));
		}

		void addFloat(std::string label, float* value, float minValue, float maxValue, std::string format = "%.3f")
		{
			addFloat(
				std::move(label),
				[value]() { return *value; },
				[value](float newValue) { *value = newValue; },
				minValue,
				maxValue,
				std::move(format)
			);
		}

		void addBool(
			std::string label,
			const std::function<bool()>& getter,
			const std::function<void(bool)>& setter
		)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::Bool;
			descriptor.label = std::move(label);
			descriptor.getBool = getter;
			descriptor.setBool = setter;
			mProperties.push_back(std::move(descriptor));
		}

		void addBool(std::string label, bool* value)
		{
			addBool(
				std::move(label),
				[value]() { return *value; },
				[value](bool newValue) { *value = newValue; }
			);
		}

		void addInt(
			std::string label,
			const std::function<int()>& getter,
			const std::function<void(int)>& setter,
			int minValue,
			int maxValue
		)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::Int;
			descriptor.label = std::move(label);
			descriptor.minValue = static_cast<float>(minValue);
			descriptor.maxValue = static_cast<float>(maxValue);
			descriptor.getInt = getter;
			descriptor.setInt = setter;
			mProperties.push_back(std::move(descriptor));
		}

		void addInt(std::string label, int* value, int minValue, int maxValue)
		{
			addInt(
				std::move(label),
				[value]() { return *value; },
				[value](int newValue) { *value = newValue; },
				minValue,
				maxValue
			);
		}

		void addVec3(
			std::string label,
			const std::function<glm::vec3()>& getter,
			const std::function<void(glm::vec3)>& setter
		)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::Vec3;
			descriptor.label = std::move(label);
			descriptor.getVec3 = getter;
			descriptor.setVec3 = setter;
			mProperties.push_back(std::move(descriptor));
		}

		void addVec3(std::string label, glm::vec3* value)
		{
			addVec3(
				std::move(label),
				[value]() { return *value; },
				[value](glm::vec3 newValue) { *value = newValue; }
			);
		}

		void addColor3(
			std::string label,
			const std::function<glm::vec3()>& getter,
			const std::function<void(glm::vec3)>& setter
		)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::Color3;
			descriptor.label = std::move(label);
			descriptor.getVec3 = getter;
			descriptor.setVec3 = setter;
			mProperties.push_back(std::move(descriptor));
		}

		void addColor3(std::string label, glm::vec3* value)
		{
			addColor3(
				std::move(label),
				[value]() { return *value; },
				[value](glm::vec3 newValue) { *value = newValue; }
			);
		}

		void addString(
			std::string label,
			const std::function<std::string()>& getter,
			const std::function<void(const std::string&)>& setter,
			size_t capacity = 512
		)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::String;
			descriptor.label = std::move(label);
			descriptor.stringCapacity = capacity;
			descriptor.getText = getter;
			descriptor.setText = setter;
			mProperties.push_back(std::move(descriptor));
		}

		void addString(std::string label, std::string* value, size_t capacity = 512)
		{
			addString(
				std::move(label),
				[value]() { return *value; },
				[value](const std::string& newValue) { *value = newValue; },
				capacity
			);
		}

		void addText(std::string label, const std::function<std::string()>& getter)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::Text;
			descriptor.label = std::move(label);
			descriptor.getText = getter;
			mProperties.push_back(std::move(descriptor));
		}

		void addText(std::string label, std::string value)
		{
			addText(
				std::move(label),
				[value = std::move(value)]() { return value; }
			);
		}

		const std::vector<PropertyDescriptor>& getProperties() const
		{
			return mProperties;
		}

	private:
		std::vector<PropertyDescriptor> mProperties{};
	};
}

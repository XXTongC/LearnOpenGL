#pragma once

#include <array>
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
		InputFloat,
		Int,
		InputInt,
		Bool,
		Vec3,
		SliderVec3,
		Color3,
		String,
		Text,
	};

	struct PropertyDescriptor
	{
		PropertyKind kind{ PropertyKind::Text };
		std::string label{};
		std::string configKey{};
		std::vector<std::string> configKeys{};
		bool readOnly{ false };
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

		void addReadOnlyFloat(std::string label, const std::function<float()>& getter, std::string format = "%.3f")
		{
			addFloat(
				std::move(label),
				getter,
				[](float) {},
				0.0f,
				0.0f,
				std::move(format)
			);
			setLastReadOnly();
		}

		void addReadOnlyFloat(std::string label, float value, std::string format = "%.3f")
		{
			addReadOnlyFloat(
				std::move(label),
				[value]() { return value; },
				std::move(format)
			);
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

		void addInputFloat(
			std::string label,
			const std::function<float()>& getter,
			const std::function<void(float)>& setter,
			std::string format = "%.3f"
		)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::InputFloat;
			descriptor.label = std::move(label);
			descriptor.format = std::move(format);
			descriptor.getFloat = getter;
			descriptor.setFloat = setter;
			mProperties.push_back(std::move(descriptor));
		}

		void addReadOnlyBool(std::string label, const std::function<bool()>& getter)
		{
			addBool(
				std::move(label),
				getter,
				[](bool) {}
			);
			setLastReadOnly();
		}

		void addReadOnlyBool(std::string label, bool value)
		{
			addReadOnlyBool(
				std::move(label),
				[value]() { return value; }
			);
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

		void addInputFloat(std::string label, float* value, std::string format = "%.3f")
		{
			addInputFloat(
				std::move(label),
				[value]() { return *value; },
				[value](float newValue) { *value = newValue; },
				std::move(format)
			);
		}

		void addConfigFloat(
			std::string key,
			std::string label,
			float* value,
			float minValue,
			float maxValue,
			std::string format = "%.3f"
		)
		{
			addFloat(std::move(label), value, minValue, maxValue, std::move(format));
			setLastConfigKey(std::move(key));
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

		void addReadOnlyInt(std::string label, const std::function<int()>& getter)
		{
			addInt(
				std::move(label),
				getter,
				[](int) {},
				0,
				0
			);
			setLastReadOnly();
		}

		void addReadOnlyInt(std::string label, int value)
		{
			addReadOnlyInt(
				std::move(label),
				[value]() { return value; }
			);
		}

		void addBool(std::string label, bool* value)
		{
			addBool(
				std::move(label),
				[value]() { return *value; },
				[value](bool newValue) { *value = newValue; }
			);
		}

		void addConfigBool(std::string key, std::string label, bool* value)
		{
			addBool(std::move(label), value);
			setLastConfigKey(std::move(key));
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

		void addInputInt(
			std::string label,
			const std::function<int()>& getter,
			const std::function<void(int)>& setter
		)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::InputInt;
			descriptor.label = std::move(label);
			descriptor.getInt = getter;
			descriptor.setInt = setter;
			mProperties.push_back(std::move(descriptor));
		}

		void addReadOnlyVec3(std::string label, const std::function<glm::vec3()>& getter)
		{
			addVec3(
				std::move(label),
				getter,
				[](glm::vec3) {}
			);
			setLastReadOnly();
		}

		void addReadOnlyVec3(std::string label, glm::vec3 value)
		{
			addReadOnlyVec3(
				std::move(label),
				[value]() { return value; }
			);
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

		void addInputInt(std::string label, int* value)
		{
			addInputInt(
				std::move(label),
				[value]() { return *value; },
				[value](int newValue) { *value = newValue; }
			);
		}

		void addConfigInt(
			std::string key,
			std::string label,
			const std::function<int()>& getter,
			const std::function<void(int)>& setter,
			int minValue,
			int maxValue
		)
		{
			addInt(std::move(label), getter, setter, minValue, maxValue);
			setLastConfigKey(std::move(key));
		}

		void addConfigInt(std::string key, std::string label, int* value, int minValue, int maxValue)
		{
			addInt(std::move(label), value, minValue, maxValue);
			setLastConfigKey(std::move(key));
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

		void addSliderVec3(
			std::string label,
			const std::function<glm::vec3()>& getter,
			const std::function<void(glm::vec3)>& setter,
			float minValue,
			float maxValue
		)
		{
			PropertyDescriptor descriptor{};
			descriptor.kind = PropertyKind::SliderVec3;
			descriptor.label = std::move(label);
			descriptor.minValue = minValue;
			descriptor.maxValue = maxValue;
			descriptor.getVec3 = getter;
			descriptor.setVec3 = setter;
			mProperties.push_back(std::move(descriptor));
		}

		void addReadOnlyString(std::string label, const std::function<std::string()>& getter)
		{
			addString(
				std::move(label),
				getter,
				[](const std::string&) {}
			);
			setLastReadOnly();
		}

		void addReadOnlyString(std::string label, std::string value)
		{
			addReadOnlyString(
				std::move(label),
				[value = std::move(value)]() { return value; }
			);
		}

		void addVec3(std::string label, glm::vec3* value)
		{
			addVec3(
				std::move(label),
				[value]() { return *value; },
				[value](glm::vec3 newValue) { *value = newValue; }
			);
		}

		void addConfigVec3(
			std::array<std::string, 3> keys,
			std::string label,
			glm::vec3* value
		)
		{
			addVec3(std::move(label), value);
			setLastConfigKeys(std::move(keys));
		}

		void addConfigVec3(
			std::array<std::string, 3> keys,
			std::string label,
			const std::function<glm::vec3()>& getter,
			const std::function<void(glm::vec3)>& setter
		)
		{
			addVec3(std::move(label), getter, setter);
			setLastConfigKeys(std::move(keys));
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

		void addConfigColor3(
			std::array<std::string, 3> keys,
			std::string label,
			glm::vec3* value
		)
		{
			addColor3(std::move(label), value);
			setLastConfigKeys(std::move(keys));
		}

		void addConfigColor3(
			std::array<std::string, 3> keys,
			std::string label,
			const std::function<glm::vec3()>& getter,
			const std::function<void(glm::vec3)>& setter
		)
		{
			addColor3(std::move(label), getter, setter);
			setLastConfigKeys(std::move(keys));
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

		void addConfigString(std::string key, std::string label, std::string* value, size_t capacity = 512)
		{
			addString(std::move(label), value, capacity);
			setLastConfigKey(std::move(key));
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
		void setLastConfigKey(std::string key)
		{
			if (!mProperties.empty())
			{
				mProperties.back().configKey = std::move(key);
			}
		}

		void setLastConfigKeys(std::array<std::string, 3> keys)
		{
			if (!mProperties.empty())
			{
				mProperties.back().configKeys = {
					std::move(keys[0]),
					std::move(keys[1]),
					std::move(keys[2])
				};
			}
		}

		void setLastReadOnly()
		{
			if (!mProperties.empty())
			{
				mProperties.back().readOnly = true;
			}
		}

		std::vector<PropertyDescriptor> mProperties{};
	};
}

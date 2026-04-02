#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../../core.h"
#include "../../materials/material.h"
#include "../../framework/texture.h"
#include "../../third_party/imgui/imgui.h"

namespace GL_EDITOR
{
	enum class PropertyKind
	{
		Section,
		Float,
		Bool,
		Vec3,
		Color3,
		Text,
	};

	struct PropertyDescriptor
	{
		PropertyKind kind{ PropertyKind::Text };
		std::string label{};
		float minValue{ 0.0f };
		float maxValue{ 0.0f };
		std::string format{ "%.3f" };
		std::function<float()> getFloat{};
		std::function<void(float)> setFloat{};
		std::function<bool()> getBool{};
		std::function<void(bool)> setBool{};
		std::function<glm::vec3()> getVec3{};
		std::function<void(glm::vec3)> setVec3{};
		std::function<std::string()> getText{};
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

	inline std::string getMaterialTypeName(GLframework::MaterialType type)
	{
		switch (type)
		{
		case GLframework::MaterialType::PhongMaterial: return "PhongMaterial";
		case GLframework::MaterialType::WhiteMaterial: return "WhiteMaterial";
		case GLframework::MaterialType::DepthMaterial: return "DepthMaterial";
		case GLframework::MaterialType::OpacityMaskMaterial: return "OpacityMaskMaterial";
		case GLframework::MaterialType::ScreenMaterial: return "ScreenMaterial";
		case GLframework::MaterialType::CubeMaterial: return "CubeMaterial";
		case GLframework::MaterialType::CubeSphereMaterial: return "CubeSphereMaterial";
		case GLframework::MaterialType::PhongEnvMaterial: return "PhongEnvMaterial";
		case GLframework::MaterialType::PhongEnvSphereMaterial: return "PhongEnvSphereMaterial";
		case GLframework::MaterialType::PhongInstanceMaterial: return "PhongInstanceMaterial";
		case GLframework::MaterialType::PhongNormalMaterial: return "PhongNormalMaterial";
		case GLframework::MaterialType::GrassInstanceMaterial: return "GrassInstanceMaterial";
		case GLframework::MaterialType::PhongParallaxMaterial: return "PhongParallaxMaterial";
		case GLframework::MaterialType::PhongShadowMaterial: return "PhongShadowMaterial";
		case GLframework::MaterialType::PhongCSMShadowMaterial: return "PhongCSMShadowMaterial";
		case GLframework::MaterialType::PhongPointShadowMaterial: return "PhongPointShadowMaterial";
		default: return "UnknownMaterial";
		}
	}

	inline std::string describeTexture(const std::shared_ptr<GLframework::Texture>& texture)
	{
		if (!texture) return "None";

		std::ostringstream stream;
		stream
			<< "Unit " << texture->getUnit()
			<< ", " << texture->getWidth() << "x" << texture->getHeight()
			<< ", Target " << texture->getTextureTarget();
		return stream.str();
	}

	inline bool drawProperties(const PropertyBuilder& builder)
	{
		bool changed = false;

		for (const auto& property : builder.getProperties())
		{
			switch (property.kind)
			{
			case PropertyKind::Section:
				ImGui::Spacing();
				ImGui::TextUnformatted(property.label.c_str());
				ImGui::Separator();
				break;

			case PropertyKind::Float:
			{
				float value = property.getFloat();
				if (ImGui::SliderFloat(property.label.c_str(), &value, property.minValue, property.maxValue, property.format.c_str()))
				{
					property.setFloat(value);
					changed = true;
				}
				break;
			}

			case PropertyKind::Bool:
			{
				bool value = property.getBool();
				if (ImGui::Checkbox(property.label.c_str(), &value))
				{
					property.setBool(value);
					changed = true;
				}
				break;
			}

			case PropertyKind::Vec3:
			{
				glm::vec3 value = property.getVec3();
				float buffer[3] = { value.x, value.y, value.z };
				if (ImGui::InputFloat3(property.label.c_str(), buffer))
				{
					property.setVec3({ buffer[0], buffer[1], buffer[2] });
					changed = true;
				}
				break;
			}

			case PropertyKind::Color3:
			{
				glm::vec3 value = property.getVec3();
				float buffer[3] = { value.x, value.y, value.z };
				if (ImGui::ColorEdit3(property.label.c_str(), buffer))
				{
					property.setVec3({ buffer[0], buffer[1], buffer[2] });
					changed = true;
				}
				break;
			}

			case PropertyKind::Text:
			{
				const std::string textValue = property.getText();
				ImGui::TextWrapped("%s: %s", property.label.c_str(), textValue.c_str());
				break;
			}
			}
		}

		return changed;
	}

	inline bool drawMaterialInspector(GLframework::Material& material)
	{
		PropertyBuilder builder;
		material.visitEditableProperties(builder);
		return drawProperties(builder);
	}
}

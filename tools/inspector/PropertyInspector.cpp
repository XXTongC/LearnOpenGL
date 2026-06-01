#include "PropertyInspector.h"

#include <cstddef>
#include <string>
#include <vector>

#include "../../third_party/imgui/imgui.h"

namespace GL_EDITOR
{
	bool drawProperties(const PropertyBuilder& builder)
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
				if (property.readOnly)
				{
					ImGui::Text("%s: %.3f", property.label.c_str(), value);
					break;
				}
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
				if (property.readOnly)
				{
					ImGui::Text("%s: %s", property.label.c_str(), value ? "yes" : "no");
					break;
				}
				if (ImGui::Checkbox(property.label.c_str(), &value))
				{
					property.setBool(value);
					changed = true;
				}
				break;
			}

			case PropertyKind::Int:
			{
				int value = property.getInt();
				if (property.readOnly)
				{
					ImGui::Text("%s: %d", property.label.c_str(), value);
					break;
				}
				if (ImGui::SliderInt(property.label.c_str(), &value, static_cast<int>(property.minValue), static_cast<int>(property.maxValue)))
				{
					property.setInt(value);
					changed = true;
				}
				break;
			}

			case PropertyKind::Vec3:
			{
				glm::vec3 value = property.getVec3();
				if (property.readOnly)
				{
					ImGui::Text(
						"%s: %.3f, %.3f, %.3f",
						property.label.c_str(),
						value.x,
						value.y,
						value.z
					);
					break;
				}
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
				if (property.readOnly)
				{
					ImGui::Text(
						"%s: %.3f, %.3f, %.3f",
						property.label.c_str(),
						value.x,
						value.y,
						value.z
					);
					break;
				}
				float buffer[3] = { value.x, value.y, value.z };
				if (ImGui::ColorEdit3(property.label.c_str(), buffer))
				{
					property.setVec3({ buffer[0], buffer[1], buffer[2] });
					changed = true;
				}
				break;
			}

			case PropertyKind::String:
			{
				if (property.readOnly)
				{
					const std::string value = property.getText();
					ImGui::TextWrapped("%s: %s", property.label.c_str(), value.c_str());
					break;
				}
				const size_t capacity = property.stringCapacity > 1 ? property.stringCapacity : 2;
				std::vector<char> buffer(capacity, '\0');
				const std::string value = property.getText();
				value.copy(buffer.data(), capacity - 1);
				if (ImGui::InputText(property.label.c_str(), buffer.data(), buffer.size()))
				{
					property.setText(buffer.data());
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
}

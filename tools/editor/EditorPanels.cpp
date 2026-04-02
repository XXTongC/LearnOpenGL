#include "EditorPanels.h"

#include "../../light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
#include "../../light/shadow/directionalLightShadow/directionalLightShadow.h"
#include "../../light/shadow/pointLightShadow/pointLightShadow.h"
#include "../../mesh/mesh.h"
#include "../../camera/orthographiccamera.h"
#include "../../camera/perspectivecamera.h"
#include "../inspector/MaterialInspector.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
	std::string getObjectTypeName(GLframework::ObjectType type)
	{
		switch (type)
		{
		case GLframework::ObjectType::Object: return "Object";
		case GLframework::ObjectType::Mesh: return "Mesh";
		case GLframework::ObjectType::Scene: return "Scene";
		case GLframework::ObjectType::InstancedMesh: return "InstancedMesh";
		case GLframework::ObjectType::Light: return "Light";
		default: return "Unknown";
		}
	}

	std::string getObjectDisplayName(const std::shared_ptr<GLframework::Object>& object)
	{
		if (!object) return "Null";

		const std::string explicitName = object->getName();
		if (!explicitName.empty()) return explicitName;

		return getObjectTypeName(object->getType());
	}

	std::string getShadowTypeName(const std::shared_ptr<GLframework::Shadow>& shadow)
	{
		if (!shadow) return "Shadow";
		if (std::dynamic_pointer_cast<GLframework::DirectionalLightCSMShadow>(shadow)) return "DirectionalLightCSMShadow";
		if (std::dynamic_pointer_cast<GLframework::DirectionalLightShadow>(shadow)) return "DirectionalLightShadow";
		if (std::dynamic_pointer_cast<GLframework::PointLightShadow>(shadow)) return "PointLightShadow";
		return "Shadow";
	}

	void renderLightInspector(const std::shared_ptr<GLframework::Light>& light, GL_EDITOR::SelectionContext& selection)
	{
		glm::vec3 color = light->getColor();
		if (ImGui::ColorEdit3("Light Color", &color[0]))
		{
			light->setColor(color);
		}

		float intensity = light->getIntensity();
		if (ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f))
		{
			light->setIntensity(intensity);
		}

		float specular = light->getSpecularIntensity();
		if (ImGui::SliderFloat("Specular", &specular, 0.0f, 10.0f))
		{
			light->setSpecularIntensity(specular);
		}

		if (auto pointLight = std::dynamic_pointer_cast<GLframework::PointLight>(light))
		{
			float attenuation[3] = { pointLight->getK2(), pointLight->getK1(), pointLight->getK0() };
			if (ImGui::InputFloat3("Attenuation (k2,k1,k0)", attenuation))
			{
				pointLight->setK(attenuation[0], attenuation[1], attenuation[2]);
			}
		}

		if (auto spot = std::dynamic_pointer_cast<GLframework::SpotLight>(light))
		{
			float inner = spot->getInnerAngle();
			float outer = spot->getOutAngle();
			if (ImGui::SliderFloat("Inner Angle", &inner, 0.0f, 90.0f))
			{
				spot->setInnerAngle(inner);
			}
			if (ImGui::SliderFloat("Outer Angle", &outer, 0.0f, 90.0f))
			{
				spot->setOutAngle(outer);
			}
		}

		if (light->getShadow())
		{
			ImGui::Text("Shadow: %s", getShadowTypeName(light->getShadow()).c_str());
			if (ImGui::Button("Inspect Shadow"))
			{
				GL_EDITOR::selectShadow(selection, light->getShadow(), getObjectDisplayName(light) + " Shadow");
			}
		}
	}

	void renderShadowInspector(const std::shared_ptr<GLframework::Shadow>& shadow, GL_EDITOR::SelectionContext& selection)
	{
		ImGui::SliderFloat("Bias", &shadow->mBias, 0.0f, 0.01f, "%.6f");
		ImGui::SliderFloat("PCF Radius", &shadow->mPcfRadius, 0.0f, 10.0f, "%.3f");
		ImGui::SliderFloat("Disk Tightness", &shadow->mDiskTightness, 0.0f, 4.0f, "%.3f");
		ImGui::SliderFloat("Light Size", &shadow->mLightSize, 0.0f, 1.0f, "%.3f");

		if (shadow->mRenderTarget)
		{
			int widthValue = static_cast<int>(shadow->mRenderTarget->getWidth());
			int heightValue = static_cast<int>(shadow->mRenderTarget->getHeight());
			if (ImGui::InputInt("Shadow Width", &widthValue) | ImGui::InputInt("Shadow Height", &heightValue))
			{
				if (widthValue < 1) widthValue = 1;
				if (heightValue < 1) heightValue = 1;
				shadow->setRenderTargetSize(widthValue, heightValue);
			}
		}

		if (auto csmShadow = std::dynamic_pointer_cast<GLframework::DirectionalLightCSMShadow>(shadow))
		{
			int layerCount = csmShadow->getLayerCount();
			if (ImGui::SliderInt("Cascade Layers", &layerCount, 1, 8))
			{
				csmShadow->setLayerCount(layerCount);
				if (csmShadow->mRenderTarget)
				{
					csmShadow->setRenderTargetSize(
						static_cast<int>(csmShadow->mRenderTarget->getWidth()),
						static_cast<int>(csmShadow->mRenderTarget->getHeight())
					);
				}
			}
		}

		if (auto pointShadow = std::dynamic_pointer_cast<GLframework::PointLightShadow>(shadow))
		{
			int shadowMapIndex = pointShadow->getShadowMapIndex();
			ImGui::InputInt("Shadow Map Index", &shadowMapIndex);
			pointShadow->setShadowMapIndex(shadowMapIndex);
			ImGui::Text("Max Point Lights: %d", GLframework::PointLightShadow::getMAX_POINT_LIGHT());
		}

		if (shadow->mCamera)
		{
			if (ImGui::Button("Inspect Shadow Camera"))
			{
				GL_EDITOR::selectCamera(selection, shadow->mCamera.get(), selection.label + " Camera");
			}
		}
	}

	void renderCameraInspector(Camera* selectedCamera)
	{
		if (!selectedCamera) return;

		ImGui::InputFloat3("Position", &selectedCamera->mPosition[0]);
		ImGui::InputFloat3("Up", &selectedCamera->mUp[0]);
		ImGui::InputFloat3("Right", &selectedCamera->mRight[0]);
		ImGui::InputFloat("Near", &selectedCamera->mNear);
		ImGui::InputFloat("Far", &selectedCamera->mFar);

		if (auto perspective = dynamic_cast<PerspectiveCamera*>(selectedCamera))
		{
			float fovy = perspective->getFovy();
			float aspect = perspective->getAspect();
			if (ImGui::SliderFloat("Fovy", &fovy, 1.0f, 179.0f))
			{
				perspective->setFovy(fovy);
			}
			if (ImGui::InputFloat("Aspect", &aspect))
			{
				perspective->setAspect(aspect);
			}
		}

		if (auto orthographic = dynamic_cast<OrthographicCamera*>(selectedCamera))
		{
			ImGui::InputFloat("Left", &orthographic->mL);
			ImGui::InputFloat("Right", &orthographic->mR);
			ImGui::InputFloat("Top", &orthographic->mT);
			ImGui::InputFloat("Bottom", &orthographic->mB);
		}
	}

	void renderObjectHierarchyNode(const std::shared_ptr<GLframework::Object>& object, GL_EDITOR::SelectionContext& selection)
	{
		if (!object) return;

		const auto selectedObject = GL_EDITOR::getSelectedObject(selection);
		const bool isSelected = selectedObject && selectedObject.get() == object.get();
		const bool hasChildren = !object->getChildren().empty();

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
		if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

		ImGui::PushID(object.get());
		const bool isOpen = ImGui::TreeNodeEx("node", flags, "%s", getObjectDisplayName(object).c_str());
		if (ImGui::IsItemClicked())
		{
			GL_EDITOR::selectObject(selection, object);
		}

		if (isOpen)
		{
			for (const auto& child : object->getChildren())
			{
				renderObjectHierarchyNode(child, selection);
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	void renderLightHierarchyNode(const std::shared_ptr<GLframework::Light>& light, const std::string& fallbackName, GL_EDITOR::SelectionContext& selection)
	{
		if (!light) return;

		if (light->getName().empty())
		{
			light->setName(fallbackName);
		}

		const auto selectedObject = GL_EDITOR::getSelectedObject(selection);
		const bool isSelected = selection.kind == GL_EDITOR::SelectionKind::Object && selectedObject && selectedObject.get() == light.get();
		const auto shadow = light->getShadow();
		const bool hasChildren = shadow != nullptr;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;
		if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

		ImGui::PushID(light.get());
		const bool isOpen = ImGui::TreeNodeEx("light", flags, "%s", getObjectDisplayName(light).c_str());
		if (ImGui::IsItemClicked())
		{
			GL_EDITOR::selectObject(selection, light);
		}

		if (isOpen)
		{
			if (shadow)
			{
				const auto selectedShadow = GL_EDITOR::getSelectedShadow(selection);
				const bool shadowSelected = selection.kind == GL_EDITOR::SelectionKind::Shadow && selectedShadow && selectedShadow.get() == shadow.get();
				ImGuiTreeNodeFlags shadowFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
				if (shadowSelected) shadowFlags |= ImGuiTreeNodeFlags_Selected;

				ImGui::PushID(shadow.get());
				ImGui::TreeNodeEx("shadow", shadowFlags, "%s", getShadowTypeName(shadow).c_str());
				if (ImGui::IsItemClicked())
				{
					GL_EDITOR::selectShadow(selection, shadow, getObjectDisplayName(light) + " Shadow");
				}

				if (shadow->mCamera)
				{
					const bool cameraSelected = selection.kind == GL_EDITOR::SelectionKind::Camera && GL_EDITOR::getSelectedCamera(selection) == shadow->mCamera.get();
					ImGuiTreeNodeFlags cameraFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
					if (cameraSelected) cameraFlags |= ImGuiTreeNodeFlags_Selected;

					ImGui::TreeNodeEx("shadow-camera", cameraFlags, "%s", "Shadow Camera");
					if (ImGui::IsItemClicked())
					{
						GL_EDITOR::selectCamera(selection, shadow->mCamera.get(), getObjectDisplayName(light) + " Shadow Camera");
					}
				}

				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}
}

void GL_EDITOR::ensureSelectionIsInitialized(SelectionContext& selection, const std::shared_ptr<GLframework::Object>& defaultObject)
{
	if (selection.kind != SelectionKind::None) return;
	if (defaultObject)
	{
		selectObject(selection, defaultObject);
	}
}

std::shared_ptr<GLframework::Object> GL_EDITOR::getSelectedObject(const SelectionContext& selection)
{
	if (selection.kind != SelectionKind::Object) return nullptr;
	return selection.selectedObject.lock();
}

std::shared_ptr<GLframework::Shadow> GL_EDITOR::getSelectedShadow(const SelectionContext& selection)
{
	if (selection.kind != SelectionKind::Shadow) return nullptr;
	return selection.selectedShadow.lock();
}

Camera* GL_EDITOR::getSelectedCamera(const SelectionContext& selection)
{
	if (selection.kind != SelectionKind::Camera) return nullptr;
	return selection.selectedCamera;
}

void GL_EDITOR::selectObject(SelectionContext& selection, const std::shared_ptr<GLframework::Object>& object)
{
	selection.kind = SelectionKind::Object;
	selection.selectedObject = object;
	selection.selectedShadow.reset();
	selection.selectedCamera = nullptr;
	selection.label.clear();
}

void GL_EDITOR::selectShadow(SelectionContext& selection, const std::shared_ptr<GLframework::Shadow>& shadow, const std::string& label)
{
	selection.kind = SelectionKind::Shadow;
	selection.selectedObject.reset();
	selection.selectedShadow = shadow;
	selection.selectedCamera = nullptr;
	selection.label = label;
}

void GL_EDITOR::selectCamera(SelectionContext& selection, Camera* selectedCamera, const std::string& label)
{
	selection.kind = SelectionKind::Camera;
	selection.selectedObject.reset();
	selection.selectedShadow.reset();
	selection.selectedCamera = selectedCamera;
	selection.label = label;
}

void GL_EDITOR::drawHierarchyPanel(const EditorPanelContext& context, SelectionContext& selection)
{
	ImGui::Begin("hierarchy");

	if (ImGui::CollapsingHeader("Scenes", ImGuiTreeNodeFlags_DefaultOpen))
	{
		renderObjectHierarchyNode(context.sceneOffScreen, selection);
		renderObjectHierarchyNode(context.sceneInScreen, selection);
	}

	if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
	{
		renderLightHierarchyNode(context.directionalLight, "Directional Light", selection);
		renderLightHierarchyNode(context.spotLight, "Spot Light", selection);
		if (context.pointLights)
		{
			for (std::size_t index = 0; index < context.pointLights->size(); ++index)
			{
				renderLightHierarchyNode((*context.pointLights)[index], std::string("Point Light ") + std::to_string(index), selection);
			}
		}
	}

	if (ImGui::CollapsingHeader("Cameras", ImGuiTreeNodeFlags_DefaultOpen) && context.mainCamera)
	{
		const bool cameraSelected = selection.kind == SelectionKind::Camera && getSelectedCamera(selection) == context.mainCamera;
		ImGuiTreeNodeFlags cameraFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (cameraSelected) cameraFlags |= ImGuiTreeNodeFlags_Selected;
		ImGui::TreeNodeEx("main-camera", cameraFlags, "%s", "Main Camera");
		if (ImGui::IsItemClicked())
		{
			selectCamera(selection, context.mainCamera, "Main Camera");
		}
	}

	ImGui::End();
}

void GL_EDITOR::drawSelectionInspectorPanel(const EditorPanelContext&, SelectionContext& selection)
{
	ImGui::Begin("inspector");

	auto selectedObject = getSelectedObject(selection);
	auto selectedShadow = getSelectedShadow(selection);
	auto selectedCamera = getSelectedCamera(selection);
	if (!selectedObject && !selectedShadow && !selectedCamera)
	{
		ImGui::TextUnformatted("No target selected.");
		ImGui::End();
		return;
	}

	if (selectedShadow)
	{
		ImGui::Text("Name: %s", selection.label.c_str());
		ImGui::Text("Type: %s", getShadowTypeName(selectedShadow).c_str());
		ImGui::Separator();
		renderShadowInspector(selectedShadow, selection);
		ImGui::End();
		return;
	}

	if (selectedCamera)
	{
		ImGui::Text(
			"Name: %s",
			selection.label.c_str()
		);
		ImGui::Text(
			"Type: %s",
			dynamic_cast<PerspectiveCamera*>(selectedCamera) ? "PerspectiveCamera"
			: (dynamic_cast<OrthographicCamera*>(selectedCamera) ? "OrthographicCamera" : "Camera")
		);
		ImGui::Separator();
		renderCameraInspector(selectedCamera);
		ImGui::End();
		return;
	}

	ImGui::Text("Name: %s", getObjectDisplayName(selectedObject).c_str());
	ImGui::Text("Type: %s", getObjectTypeName(selectedObject->getType()).c_str());
	ImGui::Text("Children: %d", static_cast<int>(selectedObject->getChildren().size()));
	ImGui::Separator();

	glm::vec3 position = selectedObject->getPosition();
	if (ImGui::InputFloat3("Position", &position[0]))
	{
		selectedObject->setPosition(position);
	}

	glm::vec3 rotation = {
		selectedObject->getAngleX(),
		selectedObject->getAngleY(),
		selectedObject->getAngleZ()
	};
	if (ImGui::SliderFloat3("Rotation", &rotation[0], -360.0f, 360.0f))
	{
		selectedObject->setAngleX(rotation.x);
		selectedObject->setAngleY(rotation.y);
		selectedObject->setAngleZ(rotation.z);
	}

	glm::vec3 scaleValue = selectedObject->getScale();
	if (ImGui::InputFloat3("Scale", &scaleValue[0]))
	{
		selectedObject->setScale(scaleValue);
	}

	if (auto selectedLight = std::dynamic_pointer_cast<GLframework::Light>(selectedObject))
	{
		ImGui::Spacing();
		ImGui::Separator();
		renderLightInspector(selectedLight, selection);
	}

	auto selectedMesh = std::dynamic_pointer_cast<GLframework::Mesh>(selectedObject);
	if (selectedMesh && selectedMesh->getMaterial())
	{
		ImGui::Spacing();
		ImGui::Text("Material: %s", GL_EDITOR::getMaterialTypeName(selectedMesh->getMaterial()->getMaterialType()).c_str());
		ImGui::Separator();
		GL_EDITOR::drawMaterialInspector(*selectedMesh->getMaterial());
	}

	ImGui::End();
}

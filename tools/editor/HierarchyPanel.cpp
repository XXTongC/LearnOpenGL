#include "EditorPanelContext.h"
#include "EditorPanelFacades.h"
#include "EditorSelectionState.h"

#include <cstddef>
#include <memory>
#include <string>

#include "../../engine/Actor.h"
#include "../../engine/ActorComponent.h"
#include "../../engine/Level.h"
#include "../../engine/World.h"
#include "../../framework/object.h"
#include "../../framework/scene.h"
#include "../../light/directionalLight.h"
#include "../../light/light.h"
#include "../../light/pointLight.h"
#include "../../light/shadow/shadow.h"
#include "../../light/spotLight.h"
#include "../inspector/EngineWorldInspector.h"
#include "../inspector/SceneObjectInspector.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
	using GL_EDITOR::getActorTypeName;
	using GL_EDITOR::getComponentTypeName;
	using GL_EDITOR::getEngineObjectDisplayName;
	using GL_EDITOR::getObjectDisplayName;

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

	void renderActorHierarchyNode(GLengine::Actor& actor, GL_EDITOR::SelectionContext& selection)
	{
		const bool isSelected = selection.kind == GL_EDITOR::SelectionKind::Actor && GL_EDITOR::getSelectedActor(selection) == &actor;
		const bool hasComponents = !actor.getComponents().empty();

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (!hasComponents) flags |= ImGuiTreeNodeFlags_Leaf;
		if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

		ImGui::PushID(&actor);
		const std::string actorName = getEngineObjectDisplayName(actor, "Actor");
		const std::string actorTypeName = getActorTypeName(actor);
		const bool isOpen = ImGui::TreeNodeEx(
			"actor",
			flags,
			"%s (%s)",
			actorName.c_str(),
			actorTypeName.c_str()
		);
		if (ImGui::IsItemClicked())
		{
			GL_EDITOR::selectActor(selection, &actor, actorName);
		}

		if (isOpen)
		{
			for (const auto& component : actor.getComponents())
			{
				if (!component)
				{
					continue;
				}

				ImGuiTreeNodeFlags componentFlags =
					ImGuiTreeNodeFlags_Leaf
					| ImGuiTreeNodeFlags_NoTreePushOnOpen
					| ImGuiTreeNodeFlags_SpanAvailWidth;
				const bool componentSelected =
					selection.kind == GL_EDITOR::SelectionKind::Component
					&& GL_EDITOR::getSelectedComponent(selection) == component.get();
				if (componentSelected)
				{
					componentFlags |= ImGuiTreeNodeFlags_Selected;
				}
				const std::string componentName = getEngineObjectDisplayName(*component, "Component");
				const std::string componentTypeName = getComponentTypeName(*component);
				ImGui::PushID(component.get());
				ImGui::TreeNodeEx(
					"component",
					componentFlags,
					"%s (%s)",
					componentName.c_str(),
					componentTypeName.c_str()
				);
				if (ImGui::IsItemClicked())
				{
					GL_EDITOR::selectComponent(selection, component.get(), componentName);
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	void renderEngineWorldHierarchy(GLengine::World* world, GL_EDITOR::SelectionContext& selection)
	{
		if (!world)
		{
			ImGui::TextUnformatted("No runtime engine World.");
			return;
		}

		const GLengine::Level* level = world->getPersistentLevel();
		const int actorCount = level ? static_cast<int>(level->getActors().size()) : 0;
		const std::string worldName = getEngineObjectDisplayName(*world, "World");
		ImGui::PushID(world);
		const bool worldOpen = ImGui::TreeNodeEx(
			"engine-world",
			ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth,
			"%s (actors=%d)",
			worldName.c_str(),
			actorCount
		);
		if (worldOpen)
		{
			if (!level)
			{
				ImGui::TextUnformatted("No persistent level.");
			}
			else
			{
				const std::string levelName = getEngineObjectDisplayName(*level, "Persistent Level");
				ImGui::PushID(level);
				const bool levelOpen = ImGui::TreeNodeEx(
					"persistent-level",
					ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth,
					"%s",
					levelName.c_str()
				);
				if (levelOpen)
				{
					for (const auto& actor : level->getActors())
					{
						if (actor)
						{
							renderActorHierarchyNode(*actor, selection);
						}
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
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
				ImGui::TreeNodeEx("shadow", shadowFlags, "%s", GL_EDITOR::getShadowTypeName(shadow).c_str());
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

void GL_EDITOR::drawHierarchyPanel(const EditorPanelContext& context, SelectionContext& selection)
{
	ImGui::Begin("hierarchy");

	if (ImGui::CollapsingHeader("Scenes", ImGuiTreeNodeFlags_DefaultOpen))
	{
		renderObjectHierarchyNode(context.sceneOffScreen, selection);
		renderObjectHierarchyNode(context.sceneInScreen, selection);
	}

	if (ImGui::CollapsingHeader("Engine World", ImGuiTreeNodeFlags_DefaultOpen))
	{
		renderEngineWorldHierarchy(context.engineWorld, selection);
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

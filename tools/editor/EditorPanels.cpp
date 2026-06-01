#include "EditorPanels.h"

#include "../../framework/object.h"
#include "../../framework/scene.h"
#include "../../light/directionalLight.h"
#include "../../light/light.h"
#include "../../light/pointLight.h"
#include "../../light/shadow/shadow.h"
#include "../../light/spotLight.h"
#include "../../mesh/mesh.h"
#include "../../engine/Actor.h"
#include "../../engine/ActorComponent.h"
#include "../../engine/AssetRegistry.h"
#include "../../engine/Level.h"
#include "../../engine/World.h"
#include "EditorWorldActions.h"
#include "SceneTransformSnapshot.h"
#include "../inspector/AssetInspector.h"
#include "../inspector/EngineWorldInspector.h"
#include "../inspector/MaterialInspector.h"
#include "../inspector/PropertyInspector.h"
#include "../inspector/SceneObjectInspector.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
	using GL_EDITOR::getActorTypeName;
	using GL_EDITOR::getComponentTypeName;
	using GL_EDITOR::getEngineObjectDisplayName;

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

	void renderLightInspector(const std::shared_ptr<GLframework::Light>& light, GL_EDITOR::SelectionContext& selection)
	{
		const auto lightProperties = GL_EDITOR::buildLightPropertySchema(light);
		GL_EDITOR::drawProperties(lightProperties);

		if (light->getShadow())
		{
			ImGui::Text("Shadow: %s", GL_EDITOR::getShadowTypeName(light->getShadow()).c_str());
			if (ImGui::Button("Inspect Shadow"))
			{
				GL_EDITOR::selectShadow(selection, light->getShadow(), getObjectDisplayName(light) + " Shadow");
			}
		}
	}

	void renderShadowInspector(const std::shared_ptr<GLframework::Shadow>& shadow, GL_EDITOR::SelectionContext& selection)
	{
		const auto shadowProperties = GL_EDITOR::buildShadowPropertySchema(shadow);
		GL_EDITOR::drawProperties(shadowProperties);

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

		const auto cameraProperties = GL_EDITOR::buildCameraPropertySchema(selectedCamera);
		GL_EDITOR::drawProperties(cameraProperties);
	}

	void renderActorInspector(GLengine::Actor& actor, bool engineWorldEditable, GL_EDITOR::EditTransactionLog* editTransactions)
	{
		const auto actorProperties = GL_EDITOR::buildActorPropertySchema(actor, engineWorldEditable);
		GL_EDITOR::drawProperties(actorProperties);

		if (ImGui::TreeNodeEx("actor-components", ImGuiTreeNodeFlags_DefaultOpen, "%s", "Components"))
		{
			for (const auto& component : actor.getComponents())
			{
				if (!component)
				{
					continue;
				}

				const std::string componentName = getEngineObjectDisplayName(*component, "Component");
				const std::string componentTypeName = getComponentTypeName(*component);
				ImGui::PushID(component.get());
				const bool componentOpen = ImGui::TreeNodeEx(
					"component-properties",
					ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth,
					"%s (%s)",
					componentName.c_str(),
					componentTypeName.c_str()
				);
				if (componentOpen)
				{
					const auto componentProperties = GL_EDITOR::buildComponentPropertySchema(*component, engineWorldEditable, editTransactions);
					GL_EDITOR::drawProperties(componentProperties);
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}

	void renderComponentInspector(GLengine::ActorComponent& component, bool engineWorldEditable, GL_EDITOR::EditTransactionLog* editTransactions)
	{
		const auto componentProperties = GL_EDITOR::buildComponentPropertySchema(component, engineWorldEditable, editTransactions);
		GL_EDITOR::drawProperties(componentProperties);
	}

	void renderEditTransactionSummary(
		GL_EDITOR::EditTransactionLog* editTransactions,
		GLengine::World* engineWorld,
		bool engineWorldEditable
	)
	{
		if (!editTransactions)
		{
			return;
		}

		static std::string lastSnapshotStatus{};

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextUnformatted("Edit Transactions");
		ImGui::Text("Dirty: %s", editTransactions->isDirty() ? "yes" : "no");
		ImGui::Text("Records: %d", static_cast<int>(editTransactions->getRecords().size()));
		if (ImGui::Button("Mark Saved"))
		{
			editTransactions->markSaved();
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear Transactions"))
		{
			editTransactions->clear();
			return;
		}
		if (engineWorld && engineWorldEditable)
		{
			if (ImGui::Button("Create Empty Actor"))
			{
				const auto result = GL_EDITOR::createEditorEmptyActor(*engineWorld);
				if (result.created && result.actor)
				{
					editTransactions->recordLifecycle(
						getEngineObjectDisplayName(*result.actor, "Editor Created Empty Actor"),
						getActorTypeName(*result.actor),
						result.actor->getObjectId(),
						reinterpret_cast<std::uintptr_t>(result.actor),
						"Create Actor"
					);
					lastSnapshotStatus =
						"Actor created: " + result.actorPersistentId
						+ " root=" + result.rootComponentPersistentId;
				}
				else
				{
					lastSnapshotStatus = "Actor create failed: " + result.error;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Save Transform Snapshot"))
			{
				const auto result = GL_EDITOR::saveSceneTransformSnapshot(
					*engineWorld,
					GL_EDITOR::defaultSceneTransformSnapshotPath()
				);
				if (result.saved)
				{
					editTransactions->markSaved();
					lastSnapshotStatus =
						"Snapshot saved: " + result.path
						+ " actors=" + std::to_string(result.actorCount)
						+ " sceneComponents=" + std::to_string(result.sceneComponentCount);
				}
				else
				{
					lastSnapshotStatus = "Snapshot save failed: " + result.error;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Apply Transform Snapshot"))
			{
				const auto result = GL_EDITOR::applySceneTransformSnapshot(
					*engineWorld,
					GL_EDITOR::defaultSceneTransformSnapshotPath()
				);
				if (result.applied)
				{
					editTransactions->markSaved();
					lastSnapshotStatus =
						"Snapshot applied: " + result.path
						+ " matched=" + std::to_string(result.matchedSceneComponentCount)
						+ " applied=" + std::to_string(result.appliedSceneComponentCount)
						+ " changed=" + std::to_string(result.changedSceneComponentCount);
				}
				else
				{
					lastSnapshotStatus = "Snapshot apply failed: " + result.error;
				}
			}
		}
		else if (engineWorld && !engineWorldEditable)
		{
			ImGui::TextDisabled("Transform snapshot disabled for read-only World mirror.");
		}
		if (!lastSnapshotStatus.empty())
		{
			ImGui::TextWrapped("%s", lastSnapshotStatus.c_str());
		}
		if (const auto* latest = editTransactions->getLatestRecord())
		{
			ImGui::Text(
				"Latest: #%llu %s.%s",
				static_cast<unsigned long long>(latest->sequence),
				latest->targetLabel.c_str(),
				latest->field.c_str()
			);
			if (latest->kind == GL_EDITOR::EditTransactionRecordKind::Lifecycle)
			{
				ImGui::Text("Lifecycle transaction; undo is not implemented yet.");
			}
			else
			{
				ImGui::Text(
					"Before: %.3f, %.3f, %.3f",
					latest->beforeValue.x,
					latest->beforeValue.y,
					latest->beforeValue.z
				);
				ImGui::Text(
					"After: %.3f, %.3f, %.3f",
					latest->afterValue.x,
					latest->afterValue.y,
					latest->afterValue.z
				);
			}
			if (latest->kind == GL_EDITOR::EditTransactionRecordKind::TransformVec3
				&& latest->sceneComponent
				&& ImGui::Button("Undo Latest Transform"))
			{
				GL_EDITOR::undoLatestSceneComponentVec3Edit(*editTransactions);
			}
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

	void renderAssetDescriptor(const GLengine::AssetDescriptor& asset, GL_EDITOR::SelectionContext& selection)
	{
		const std::string label = GL_EDITOR::getAssetDisplayName(asset);
		const bool isSelected =
			selection.kind == GL_EDITOR::SelectionKind::Asset
			&& GL_EDITOR::getSelectedAssetHandle(selection) == asset.handle.value;
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (isSelected)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		ImGui::PushID(asset.handle.value.c_str());
		const bool isOpen = ImGui::TreeNodeEx(
			"asset",
			flags,
			"%s (%s)",
			label.c_str(),
			std::string(GLengine::assetKindToken(asset.kind)).c_str()
		);
		if (ImGui::IsItemClicked())
		{
			GL_EDITOR::selectAsset(selection, asset.handle.value, label);
		}
		if (isOpen)
		{
			ImGui::TextWrapped("Handle: %s", asset.handle.value.c_str());
			ImGui::Text("Source: %s", asset.source.empty() ? "unknown" : asset.source.c_str());
			ImGui::TextWrapped("Path: %s", asset.path.empty() ? "unknown" : asset.path.c_str());
			if (!asset.materialType.empty())
			{
				ImGui::Text("Material Type: %s", asset.materialType.c_str());
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	void renderAssetInspector(const GLengine::AssetRegistry* assetRegistry, const std::string& assetHandle)
	{
		if (!assetRegistry)
		{
			ImGui::TextUnformatted("No asset registry.");
			return;
		}

		const auto* asset = assetRegistry->find(GLengine::AssetHandle{ assetHandle });
		if (!asset)
		{
			ImGui::TextWrapped("Asset handle is no longer registered: %s", assetHandle.c_str());
			return;
		}

		const auto builder = GL_EDITOR::buildAssetPropertySchema(*asset);
		GL_EDITOR::drawProperties(builder);
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

GLengine::Actor* GL_EDITOR::getSelectedActor(const SelectionContext& selection)
{
	if (selection.kind != SelectionKind::Actor) return nullptr;
	return selection.selectedActor;
}

GLengine::ActorComponent* GL_EDITOR::getSelectedComponent(const SelectionContext& selection)
{
	if (selection.kind != SelectionKind::Component) return nullptr;
	return selection.selectedComponent;
}

const std::string& GL_EDITOR::getSelectedAssetHandle(const SelectionContext& selection)
{
	static const std::string empty{};
	if (selection.kind != SelectionKind::Asset) return empty;
	return selection.selectedAssetHandle;
}

void GL_EDITOR::selectObject(SelectionContext& selection, const std::shared_ptr<GLframework::Object>& object)
{
	selection.kind = SelectionKind::Object;
	selection.selectedObject = object;
	selection.selectedShadow.reset();
	selection.selectedCamera = nullptr;
	selection.selectedActor = nullptr;
	selection.selectedComponent = nullptr;
	selection.selectedAssetHandle.clear();
	selection.label.clear();
}

void GL_EDITOR::selectShadow(SelectionContext& selection, const std::shared_ptr<GLframework::Shadow>& shadow, const std::string& label)
{
	selection.kind = SelectionKind::Shadow;
	selection.selectedObject.reset();
	selection.selectedShadow = shadow;
	selection.selectedCamera = nullptr;
	selection.selectedActor = nullptr;
	selection.selectedComponent = nullptr;
	selection.selectedAssetHandle.clear();
	selection.label = label;
}

void GL_EDITOR::selectCamera(SelectionContext& selection, Camera* selectedCamera, const std::string& label)
{
	selection.kind = SelectionKind::Camera;
	selection.selectedObject.reset();
	selection.selectedShadow.reset();
	selection.selectedCamera = selectedCamera;
	selection.selectedActor = nullptr;
	selection.selectedComponent = nullptr;
	selection.selectedAssetHandle.clear();
	selection.label = label;
}

void GL_EDITOR::selectActor(SelectionContext& selection, GLengine::Actor* actor, const std::string& label)
{
	selection.kind = SelectionKind::Actor;
	selection.selectedObject.reset();
	selection.selectedShadow.reset();
	selection.selectedCamera = nullptr;
	selection.selectedActor = actor;
	selection.selectedComponent = nullptr;
	selection.selectedAssetHandle.clear();
	selection.label = label;
}

void GL_EDITOR::selectComponent(SelectionContext& selection, GLengine::ActorComponent* component, const std::string& label)
{
	selection.kind = SelectionKind::Component;
	selection.selectedObject.reset();
	selection.selectedShadow.reset();
	selection.selectedCamera = nullptr;
	selection.selectedActor = nullptr;
	selection.selectedComponent = component;
	selection.selectedAssetHandle.clear();
	selection.label = label;
}

void GL_EDITOR::selectAsset(SelectionContext& selection, std::string assetHandle, const std::string& label)
{
	selection.kind = SelectionKind::Asset;
	selection.selectedObject.reset();
	selection.selectedShadow.reset();
	selection.selectedCamera = nullptr;
	selection.selectedActor = nullptr;
	selection.selectedComponent = nullptr;
	selection.selectedAssetHandle = std::move(assetHandle);
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

void GL_EDITOR::drawAssetBrowserPanel(const EditorPanelContext& context, SelectionContext& selection)
{
	ImGui::Begin("asset browser");

	if (!context.assetRegistry)
	{
		ImGui::TextUnformatted("No asset registry.");
		ImGui::End();
		return;
	}

	const auto assets = context.assetRegistry->listAssets();
	int importedAssetCount = 0;
	for (const auto& asset : assets)
	{
		if (GL_EDITOR::isImportedAsset(asset))
		{
			++importedAssetCount;
		}
	}

	ImGui::Text("Assets: %d", context.assetRegistry->count());
	ImGui::Text(
		"Meshes: %d  Materials: %d  Textures: %d",
		context.assetRegistry->countByKind(GLengine::AssetKind::Mesh),
		context.assetRegistry->countByKind(GLengine::AssetKind::Material),
		context.assetRegistry->countByKind(GLengine::AssetKind::Texture)
	);
	ImGui::Text("Imported Handles: %d", importedAssetCount);

	if (ImGui::CollapsingHeader("Imported Asset Handles", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (importedAssetCount == 0)
		{
			ImGui::TextDisabled("No imported assets registered.");
		}
		for (const auto& asset : assets)
		{
			if (GL_EDITOR::isImportedAsset(asset))
			{
				renderAssetDescriptor(asset, selection);
			}
		}
	}

	if (ImGui::CollapsingHeader("All Asset Handles"))
	{
		if (assets.empty())
		{
			ImGui::TextDisabled("No assets registered.");
		}
		for (const auto& asset : assets)
		{
			renderAssetDescriptor(asset, selection);
		}
	}

	ImGui::End();
}

void GL_EDITOR::drawSelectionInspectorPanel(const EditorPanelContext& context, SelectionContext& selection)
{
	ImGui::Begin("inspector");

	auto selectedObject = getSelectedObject(selection);
	auto selectedShadow = getSelectedShadow(selection);
	auto selectedCamera = getSelectedCamera(selection);
	auto selectedActor = getSelectedActor(selection);
	auto selectedComponent = getSelectedComponent(selection);
	const auto& selectedAssetHandle = getSelectedAssetHandle(selection);
	if (!selectedObject
		&& !selectedShadow
		&& !selectedCamera
		&& !selectedActor
		&& !selectedComponent
		&& selectedAssetHandle.empty())
	{
		ImGui::TextUnformatted("No target selected.");
		ImGui::End();
		return;
	}

	if (!selectedAssetHandle.empty())
	{
		renderAssetInspector(context.assetRegistry, selectedAssetHandle);
		ImGui::End();
		return;
	}

	if (selectedComponent)
	{
		renderComponentInspector(*selectedComponent, context.engineWorldEditable, context.editTransactions);
		renderEditTransactionSummary(context.editTransactions, context.engineWorld, context.engineWorldEditable);
		ImGui::End();
		return;
	}

	if (selectedActor)
	{
		renderActorInspector(*selectedActor, context.engineWorldEditable, context.editTransactions);
		renderEditTransactionSummary(context.editTransactions, context.engineWorld, context.engineWorldEditable);
		ImGui::End();
		return;
	}

	if (selectedShadow)
	{
		ImGui::Text("Name: %s", selection.label.c_str());
		ImGui::Text("Type: %s", GL_EDITOR::getShadowTypeName(selectedShadow).c_str());
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
		ImGui::Text("Type: %s", GL_EDITOR::getCameraTypeName(selectedCamera).c_str());
		ImGui::Separator();
		renderCameraInspector(selectedCamera);
		ImGui::End();
		return;
	}

	ImGui::Text("Name: %s", getObjectDisplayName(selectedObject).c_str());
	ImGui::Text("Type: %s", getObjectTypeName(selectedObject->getType()).c_str());
	ImGui::Text("Children: %d", static_cast<int>(selectedObject->getChildren().size()));
	ImGui::Separator();

	const auto transformProperties = GL_EDITOR::buildObjectTransformPropertySchema(selectedObject);
	GL_EDITOR::drawProperties(transformProperties);

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

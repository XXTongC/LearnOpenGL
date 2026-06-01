#include "../editor/EditorPanelContext.h"
#include "../editor/EditorPanelFacades.h"
#include "../editor/EditorSelectionState.h"

#include <cstdint>
#include <memory>
#include <string>

#include "../../engine/Actor.h"
#include "../../engine/ActorComponent.h"
#include "../../engine/AssetRegistry.h"
#include "../../engine/World.h"
#include "../../framework/object.h"
#include "../../light/light.h"
#include "../../light/shadow/shadow.h"
#include "../../mesh/mesh.h"
#include "../editor/EditorWorldActions.h"
#include "../editor/SceneTransformSnapshot.h"
#include "AssetInspector.h"
#include "EngineWorldInspector.h"
#include "MaterialInspector.h"
#include "PropertyInspector.h"
#include "SceneObjectInspector.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
	void renderLightInspector(const std::shared_ptr<GLframework::Light>& light, GL_EDITOR::SelectionContext& selection)
	{
		const auto lightProperties = GL_EDITOR::buildLightPropertySchema(light);
		GL_EDITOR::drawProperties(lightProperties);

		if (light->getShadow())
		{
			ImGui::Text("Shadow: %s", GL_EDITOR::getShadowTypeName(light->getShadow()).c_str());
			if (ImGui::Button("Inspect Shadow"))
			{
				GL_EDITOR::selectShadow(selection, light->getShadow(), GL_EDITOR::getObjectDisplayName(light) + " Shadow");
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

				const std::string componentName = GL_EDITOR::getEngineObjectDisplayName(*component, "Component");
				const std::string componentTypeName = GL_EDITOR::getComponentTypeName(*component);
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
						GL_EDITOR::getEngineObjectDisplayName(*result.actor, "Editor Created Empty Actor"),
						GL_EDITOR::getActorTypeName(*result.actor),
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

	ImGui::Text("Name: %s", GL_EDITOR::getObjectDisplayName(selectedObject).c_str());
	ImGui::Text("Type: %s", GL_EDITOR::getObjectTypeName(selectedObject).c_str());
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

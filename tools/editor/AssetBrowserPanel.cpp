#include "EditorPanelContext.h"
#include "EditorPanelFacades.h"
#include "EditorSelectionState.h"

#include <string>

#include "../../engine/AssetRegistry.h"
#include "../inspector/AssetInspector.h"
#include "../../third_party/imgui/imgui.h"

namespace
{
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

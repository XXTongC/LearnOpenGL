#pragma once

#include <memory>
#include <vector>

#include "EditorSelectionState.h"

class Camera;

namespace GLengine
{
	class AssetRegistry;
	class World;
}

namespace GLframework
{
	class DirectionalLight;
	class Object;
	class PointLight;
	class Scene;
	class Shadow;
	class SpotLight;
}

namespace GL_EDITOR
{
	struct EditorPanelContext
	{
		std::shared_ptr<GLframework::Scene> sceneOffScreen{ nullptr };
		std::shared_ptr<GLframework::Scene> sceneInScreen{ nullptr };
		std::shared_ptr<GLframework::DirectionalLight> directionalLight{ nullptr };
		std::shared_ptr<GLframework::SpotLight> spotLight{ nullptr };
		const std::vector<std::shared_ptr<GLframework::PointLight>>* pointLights{ nullptr };
		Camera* mainCamera{ nullptr };
		GLengine::World* engineWorld{ nullptr };
		const GLengine::AssetRegistry* assetRegistry{ nullptr };
		bool engineWorldEditable{ false };
		EditTransactionLog* editTransactions{ nullptr };
	};

	void drawHierarchyPanel(const EditorPanelContext& context, SelectionContext& selection);
	void drawAssetBrowserPanel(const EditorPanelContext& context, SelectionContext& selection);
	void drawSelectionInspectorPanel(const EditorPanelContext& context, SelectionContext& selection);
}

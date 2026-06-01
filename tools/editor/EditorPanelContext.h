#pragma once

#include <memory>
#include <vector>

class Camera;

namespace GLengine
{
	class AssetRegistry;
	class World;
}

namespace GLframework
{
	class DirectionalLight;
	class PointLight;
	class Scene;
	class SpotLight;
}

namespace GL_EDITOR
{
	class EditTransactionLog;

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
}

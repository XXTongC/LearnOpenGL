#pragma once

#include "../../camera/camera.h"
#include "../../light/directionalLight.h"
#include "../../light/shadow/shadow.h"
#include "../../framework/object.h"
#include "../../light/pointLight.h"
#include "../../framework/scene.h"
#include "../../light/spotLight.h"

namespace GL_EDITOR
{
	enum class SelectionKind
	{
		None,
		Object,
		Camera,
		Shadow,
	};

	struct SelectionContext
	{
		SelectionKind kind{ SelectionKind::None };
		std::weak_ptr<GLframework::Object> selectedObject{};
		std::weak_ptr<GLframework::Shadow> selectedShadow{};
		Camera* selectedCamera{ nullptr };
		std::string label{};
	};

	struct EditorPanelContext
	{
		std::shared_ptr<GLframework::Scene> sceneOffScreen{ nullptr };
		std::shared_ptr<GLframework::Scene> sceneInScreen{ nullptr };
		std::shared_ptr<GLframework::DirectionalLight> directionalLight{ nullptr };
		std::shared_ptr<GLframework::SpotLight> spotLight{ nullptr };
		const std::vector<std::shared_ptr<GLframework::PointLight>>* pointLights{ nullptr };
		Camera* mainCamera{ nullptr };
	};

	void ensureSelectionIsInitialized(SelectionContext& selection, const std::shared_ptr<GLframework::Object>& defaultObject);
	std::shared_ptr<GLframework::Object> getSelectedObject(const SelectionContext& selection);
	std::shared_ptr<GLframework::Shadow> getSelectedShadow(const SelectionContext& selection);
	Camera* getSelectedCamera(const SelectionContext& selection);

	void selectObject(SelectionContext& selection, const std::shared_ptr<GLframework::Object>& object);
	void selectShadow(SelectionContext& selection, const std::shared_ptr<GLframework::Shadow>& shadow, const std::string& label);
	void selectCamera(SelectionContext& selection, Camera* selectedCamera, const std::string& label);

	void drawHierarchyPanel(const EditorPanelContext& context, SelectionContext& selection);
	void drawSelectionInspectorPanel(const EditorPanelContext& context, SelectionContext& selection);
}

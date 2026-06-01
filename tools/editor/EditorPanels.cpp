#include "EditorPanels.h"

#include <string>
#include <utility>

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

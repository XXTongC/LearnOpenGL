#include "engine/ActorAdapters.h"

#include <utility>

#include "camera/camera.h"
#include "framework/object.h"
#include "light/light.h"
#include "mesh/mesh.h"

using namespace GLengine;

MeshComponent::MeshComponent(std::shared_ptr<GLframework::Mesh> mesh, std::string name)
	: SceneComponent(std::move(name))
	, mMesh(std::move(mesh))
{
}

MeshComponent::~MeshComponent() = default;

void MeshComponent::setMesh(std::shared_ptr<GLframework::Mesh> mesh)
{
	mMesh = std::move(mesh);
}

const std::shared_ptr<GLframework::Mesh>& MeshComponent::getMesh() const
{
	return mMesh;
}

LightComponent::LightComponent(std::shared_ptr<GLframework::Light> light, std::string name)
	: SceneComponent(std::move(name))
	, mLight(std::move(light))
{
}

LightComponent::~LightComponent() = default;

void LightComponent::setLight(std::shared_ptr<GLframework::Light> light)
{
	mLight = std::move(light);
}

const std::shared_ptr<GLframework::Light>& LightComponent::getLight() const
{
	return mLight;
}

CameraComponent::CameraComponent(Camera* camera, std::string name)
	: SceneComponent(std::move(name))
	, mCamera(camera)
{
}

void CameraComponent::setCamera(Camera* camera)
{
	mCamera = camera;
}

Camera* CameraComponent::getCamera() const
{
	return mCamera;
}

LegacyObjectComponent::LegacyObjectComponent(std::shared_ptr<GLframework::Object> object, std::string name)
	: SceneComponent(std::move(name))
	, mObject(std::move(object))
{
}

LegacyObjectComponent::~LegacyObjectComponent() = default;

void LegacyObjectComponent::setObject(std::shared_ptr<GLframework::Object> object)
{
	mObject = std::move(object);
}

const std::shared_ptr<GLframework::Object>& LegacyObjectComponent::getObject() const
{
	return mObject;
}

MeshActor::MeshActor(std::shared_ptr<GLframework::Mesh> mesh, std::string name)
	: Actor(std::move(name))
{
	mMeshComponent = &createComponent<MeshComponent>(std::move(mesh));
	setRootComponent(mMeshComponent);
}

MeshComponent* MeshActor::getMeshComponent() const
{
	return mMeshComponent;
}

LightActor::LightActor(std::shared_ptr<GLframework::Light> light, std::string name)
	: Actor(std::move(name))
{
	mLightComponent = &createComponent<LightComponent>(std::move(light));
	setRootComponent(mLightComponent);
}

LightComponent* LightActor::getLightComponent() const
{
	return mLightComponent;
}

CameraActor::CameraActor(Camera* camera, std::string name)
	: Actor(std::move(name))
{
	mCameraComponent = &createComponent<CameraComponent>(camera);
	setRootComponent(mCameraComponent);
}

CameraComponent* CameraActor::getCameraComponent() const
{
	return mCameraComponent;
}

LegacyObjectActor::LegacyObjectActor(std::shared_ptr<GLframework::Object> object, std::string name)
	: Actor(std::move(name))
{
	mLegacyObjectComponent = &createComponent<LegacyObjectComponent>(std::move(object));
	setRootComponent(mLegacyObjectComponent);
}

LegacyObjectComponent* LegacyObjectActor::getLegacyObjectComponent() const
{
	return mLegacyObjectComponent;
}

#pragma once

#include <memory>
#include <string>

#include "engine/Actor.h"
#include "engine/SceneComponent.h"

class Camera;

namespace GLframework
{
	class Light;
	class Mesh;
	class Object;
}

namespace GLengine
{
	class MeshComponent : public SceneComponent
	{
	public:
		explicit MeshComponent(std::shared_ptr<GLframework::Mesh> mesh = nullptr, std::string name = "Mesh Component");
		~MeshComponent() override;

		void setMesh(std::shared_ptr<GLframework::Mesh> mesh);
		const std::shared_ptr<GLframework::Mesh>& getMesh() const;

	private:
		std::shared_ptr<GLframework::Mesh> mMesh{ nullptr };
	};

	class LightComponent : public SceneComponent
	{
	public:
		explicit LightComponent(std::shared_ptr<GLframework::Light> light = nullptr, std::string name = "Light Component");
		~LightComponent() override;

		void setLight(std::shared_ptr<GLframework::Light> light);
		const std::shared_ptr<GLframework::Light>& getLight() const;

	private:
		std::shared_ptr<GLframework::Light> mLight{ nullptr };
	};

	class CameraComponent : public SceneComponent
	{
	public:
		explicit CameraComponent(Camera* camera = nullptr, std::string name = "Camera Component");
		~CameraComponent() override = default;

		void setCamera(Camera* camera);
		Camera* getCamera() const;

	private:
		Camera* mCamera{ nullptr };
	};

	class LegacyObjectComponent : public SceneComponent
	{
	public:
		explicit LegacyObjectComponent(std::shared_ptr<GLframework::Object> object = nullptr, std::string name = "Legacy Object Component");
		~LegacyObjectComponent() override;

		void setObject(std::shared_ptr<GLframework::Object> object);
		const std::shared_ptr<GLframework::Object>& getObject() const;

	private:
		std::shared_ptr<GLframework::Object> mObject{ nullptr };
	};

	class MeshActor : public Actor
	{
	public:
		explicit MeshActor(std::shared_ptr<GLframework::Mesh> mesh = nullptr, std::string name = "Mesh Actor");

		MeshComponent* getMeshComponent() const;

	private:
		MeshComponent* mMeshComponent{ nullptr };
	};

	class LightActor : public Actor
	{
	public:
		explicit LightActor(std::shared_ptr<GLframework::Light> light = nullptr, std::string name = "Light Actor");

		LightComponent* getLightComponent() const;

	private:
		LightComponent* mLightComponent{ nullptr };
	};

	class CameraActor : public Actor
	{
	public:
		explicit CameraActor(Camera* camera = nullptr, std::string name = "Camera Actor");

		CameraComponent* getCameraComponent() const;

	private:
		CameraComponent* mCameraComponent{ nullptr };
	};

	class LegacyObjectActor : public Actor
	{
	public:
		explicit LegacyObjectActor(std::shared_ptr<GLframework::Object> object = nullptr, std::string name = "Legacy Object Actor");

		LegacyObjectComponent* getLegacyObjectComponent() const;

	private:
		LegacyObjectComponent* mLegacyObjectComponent{ nullptr };
	};
}

#include "RenderQueue.h"

#include <algorithm>

using namespace GLframework;

void RenderQueue::build(const std::shared_ptr<Scene>& scene, Camera* camera)
{
	clear();
	if (scene == nullptr)
	{
		return;
	}

	projectObject(scene);
	sortTransparentObjects(camera);
}

void RenderQueue::clear()
{
	mOpacityObjects.clear();
	mTransparentObjects.clear();
}

const std::vector<std::shared_ptr<Mesh>>& RenderQueue::getOpacityObjects() const
{
	return mOpacityObjects;
}

const std::vector<std::shared_ptr<Mesh>>& RenderQueue::getTransparentObjects() const
{
	return mTransparentObjects;
}

void RenderQueue::projectObject(const std::shared_ptr<Object>& object)
{
	if (object->getType() == ObjectType::Mesh || object->getType() == ObjectType::InstancedMesh)
	{
		std::shared_ptr<Mesh> mesh = std::static_pointer_cast<Mesh>(object);
		std::shared_ptr<Material> material = mesh->getMaterial();
		if (material != nullptr && material->getColorBlendState())
		{
			mTransparentObjects.push_back(mesh);
		}
		else
		{
			mOpacityObjects.push_back(mesh);
		}
	}

	for (auto& child : object->getChildren())
	{
		projectObject(child);
	}
}

void RenderQueue::sortTransparentObjects(Camera* camera)
{
	if (camera == nullptr)
	{
		return;
	}

	std::sort(
		mTransparentObjects.begin(),
		mTransparentObjects.end(),
		[camera](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b)
		{
			const auto viewMatrix = camera->getViewMatrix();

			const auto worldPositionA = a->getModelMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			const auto cameraPositionA = viewMatrix * worldPositionA;

			const auto worldPositionB = b->getModelMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			const auto cameraPositionB = viewMatrix * worldPositionB;

			return cameraPositionA.z < cameraPositionB.z;
		}
	);
}

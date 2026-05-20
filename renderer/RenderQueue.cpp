#include "RenderQueue.h"

#include <algorithm>

#include "materials/material.h"

using namespace GLframework;

namespace
{
	bool isPbrMesh(const std::shared_ptr<Mesh>& mesh)
	{
		const auto material = mesh ? mesh->getMaterial() : nullptr;
		return material && material->getMaterialType() == MaterialType::PBRMaterial;
	}

	bool isTransparentMesh(const std::shared_ptr<Mesh>& mesh)
	{
		const auto material = mesh ? mesh->getMaterial() : nullptr;
		return material && material->getColorBlendState();
	}
}

void RenderQueue::build(const std::shared_ptr<Scene>& scene, Camera* camera)
{
	clear();
	if (scene == nullptr)
	{
		return;
	}

	projectObject(scene);
	sortTransparentObjects(camera, mTransparentObjects);
	sortTransparentObjects(camera, mLegacyTransparentObjects);
	sortTransparentObjects(camera, mPbrTransparentObjects);
}

void RenderQueue::clear()
{
	mOpacityObjects.clear();
	mTransparentObjects.clear();
	mLegacyOpacityObjects.clear();
	mLegacyTransparentObjects.clear();
	mPbrOpacityObjects.clear();
	mPbrTransparentObjects.clear();
}

const std::vector<std::shared_ptr<Mesh>>& RenderQueue::getOpacityObjects() const
{
	return mOpacityObjects;
}

const std::vector<std::shared_ptr<Mesh>>& RenderQueue::getTransparentObjects() const
{
	return mTransparentObjects;
}

const std::vector<std::shared_ptr<Mesh>>& RenderQueue::getLegacyOpacityObjects() const
{
	return mLegacyOpacityObjects;
}

const std::vector<std::shared_ptr<Mesh>>& RenderQueue::getLegacyTransparentObjects() const
{
	return mLegacyTransparentObjects;
}

const std::vector<std::shared_ptr<Mesh>>& RenderQueue::getPbrOpacityObjects() const
{
	return mPbrOpacityObjects;
}

const std::vector<std::shared_ptr<Mesh>>& RenderQueue::getPbrTransparentObjects() const
{
	return mPbrTransparentObjects;
}

void RenderQueue::projectObject(const std::shared_ptr<Object>& object)
{
	if (object->getType() == ObjectType::Mesh || object->getType() == ObjectType::InstancedMesh)
	{
		std::shared_ptr<Mesh> mesh = std::static_pointer_cast<Mesh>(object);
		const bool pbr = isPbrMesh(mesh);
		const bool transparent = isTransparentMesh(mesh);

		if (transparent)
		{
			mTransparentObjects.push_back(mesh);
			if (pbr)
			{
				mPbrTransparentObjects.push_back(mesh);
			}
			else
			{
				mLegacyTransparentObjects.push_back(mesh);
			}
		}
		else
		{
			mOpacityObjects.push_back(mesh);
			if (pbr)
			{
				mPbrOpacityObjects.push_back(mesh);
			}
			else
			{
				mLegacyOpacityObjects.push_back(mesh);
			}
		}
	}

	for (auto& child : object->getChildren())
	{
		projectObject(child);
	}
}

void RenderQueue::sortTransparentObjects(Camera* camera, std::vector<std::shared_ptr<Mesh>>& objects)
{
	if (camera == nullptr)
	{
		return;
	}

	std::sort(
		objects.begin(),
		objects.end(),
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

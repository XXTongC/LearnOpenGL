#include "engine/SceneComponent.h"

#include <algorithm>
#include <utility>

using namespace GLengine;

SceneComponent::SceneComponent(std::string name)
	: ActorComponent(std::move(name))
{
}

SceneComponent::~SceneComponent()
{
	detachFromParent();
	for (SceneComponent* child : mChildren)
	{
		if (child)
		{
			child->mParent = nullptr;
		}
	}
}

const Transform& SceneComponent::getRelativeTransform() const
{
	return mRelativeTransform;
}

void SceneComponent::setRelativeTransform(const Transform& transform)
{
	mRelativeTransform = transform;
}

SceneComponent* SceneComponent::getParent() const
{
	return mParent;
}

const std::vector<SceneComponent*>& SceneComponent::getChildren() const
{
	return mChildren;
}

void SceneComponent::attachTo(SceneComponent* parent)
{
	if (mParent == parent)
	{
		return;
	}

	detachFromParent();
	mParent = parent;
	if (mParent)
	{
		mParent->mChildren.push_back(this);
	}
}

void SceneComponent::detachFromParent()
{
	if (!mParent)
	{
		return;
	}

	auto& siblings = mParent->mChildren;
	siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
	mParent = nullptr;
}

#include "engine/Actor.h"

#include <utility>

using namespace GLengine;

Actor::Actor(std::string name)
	: EngineObject(std::move(name))
{
}

World* Actor::getWorld() const
{
	return mWorld;
}

Level* Actor::getLevel() const
{
	return mLevel;
}

SceneComponent* Actor::getRootComponent() const
{
	return mRootComponent;
}

void Actor::setRootComponent(SceneComponent* rootComponent)
{
	mRootComponent = rootComponent;
}

const std::vector<std::unique_ptr<ActorComponent>>& Actor::getComponents() const
{
	return mComponents;
}

void Actor::beginPlayInternal()
{
	if (mHasBegunPlay)
	{
		return;
	}

	mHasBegunPlay = true;
	beginPlay();
	for (const auto& component : mComponents)
	{
		if (component && component->isActive())
		{
			component->beginPlay();
		}
	}
}

void Actor::tickInternal(float deltaSeconds)
{
	tick(deltaSeconds);
	for (const auto& component : mComponents)
	{
		if (component && component->isActive() && component->canTick())
		{
			component->tick(deltaSeconds);
		}
	}
}

void Actor::endPlayInternal()
{
	for (const auto& component : mComponents)
	{
		if (component && component->isActive())
		{
			component->endPlay();
		}
	}
	endPlay();
	mHasBegunPlay = false;
}

void Actor::setOwningLevel(Level* level, World* world)
{
	mLevel = level;
	mWorld = world;
}

void Actor::registerComponent(std::unique_ptr<ActorComponent> component)
{
	if (!component)
	{
		return;
	}

	component->setOwner(this);
	component->onRegister();
	if (!mRootComponent)
	{
		mRootComponent = dynamic_cast<SceneComponent*>(component.get());
	}
	mComponents.push_back(std::move(component));
}

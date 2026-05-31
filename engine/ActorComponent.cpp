#include "engine/ActorComponent.h"

#include <utility>

using namespace GLengine;

ActorComponent::ActorComponent(std::string name)
	: EngineObject(std::move(name))
{
}

Actor* ActorComponent::getOwner() const
{
	return mOwner;
}

bool ActorComponent::isActive() const
{
	return mActive;
}

void ActorComponent::setActive(bool active)
{
	mActive = active;
}

bool ActorComponent::canTick() const
{
	return mCanTick;
}

void ActorComponent::setCanTick(bool canTick)
{
	mCanTick = canTick;
}

void ActorComponent::setOwner(Actor* owner)
{
	mOwner = owner;
}

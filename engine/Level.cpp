#include "engine/Level.h"

#include "engine/World.h"

#include <utility>

using namespace GLengine;

Level::Level(std::string name)
	: EngineObject(std::move(name))
{
}

World* Level::getWorld() const
{
	return mWorld;
}

const std::vector<std::unique_ptr<Actor>>& Level::getActors() const
{
	return mActors;
}

void Level::beginPlay()
{
	for (const auto& actor : mActors)
	{
		if (actor)
		{
			actor->beginPlayInternal();
		}
	}
}

void Level::tick(float deltaSeconds)
{
	for (const auto& actor : mActors)
	{
		if (actor)
		{
			actor->tickInternal(deltaSeconds);
		}
	}
}

void Level::endPlay()
{
	for (const auto& actor : mActors)
	{
		if (actor)
		{
			actor->endPlayInternal();
		}
	}
}

void Level::setWorld(World* world)
{
	mWorld = world;
	for (const auto& actor : mActors)
	{
		if (actor)
		{
			actor->setOwningLevel(this, mWorld);
		}
	}
}

void Level::addActor(std::unique_ptr<Actor> actor)
{
	if (!actor)
	{
		return;
	}

	actor->setOwningLevel(this, mWorld);
	mActors.push_back(std::move(actor));
}

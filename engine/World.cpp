#include "engine/World.h"

#include <utility>

using namespace GLengine;

World::World(std::string name)
	: EngineObject(std::move(name))
{
	createPersistentLevel();
}

Level& World::createPersistentLevel(std::string name)
{
	mPersistentLevel = std::make_unique<Level>(std::move(name));
	mPersistentLevel->setWorld(this);
	return *mPersistentLevel;
}

Level* World::getPersistentLevel() const
{
	return mPersistentLevel.get();
}

void World::beginPlay()
{
	if (mIsPlaying)
	{
		return;
	}

	mIsPlaying = true;
	mTickCount = 0;
	if (mPersistentLevel)
	{
		mPersistentLevel->beginPlay();
	}
}

void World::tick(float deltaSeconds)
{
	++mTickCount;
	if (mPersistentLevel)
	{
		mPersistentLevel->tick(deltaSeconds);
	}
}

void World::endPlay()
{
	if (!mIsPlaying)
	{
		return;
	}

	if (mPersistentLevel)
	{
		mPersistentLevel->endPlay();
	}
	mIsPlaying = false;
}

bool World::isPlaying() const
{
	return mIsPlaying;
}

int World::getTickCount() const
{
	return mTickCount;
}

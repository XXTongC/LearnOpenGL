#include "engine/Engine.h"

#include <utility>

#include "engine/EngineContext.h"
#include "engine/EngineLifecycleSnapshot.h"
#include "engine/EngineSubsystem.h"
#include "engine/Level.h"
#include "engine/World.h"

using namespace GLengine;

Engine::Engine()
	: mContext(std::make_unique<EngineContext>())
{
}

Engine::~Engine()
{
	shutdown();
}

bool Engine::initialize(const EngineDesc& desc)
{
	if (mInitialized)
	{
		return true;
	}

	mContext->desc = desc;
	mTickCount = 0;
	for (const auto& subsystem : mSubsystems)
	{
		if (subsystem && !subsystem->initialize(*mContext))
		{
			shutdown();
			return false;
		}
	}
	mInitialized = true;
	if (mActiveWorld)
	{
		mActiveWorld->beginPlay();
	}
	return true;
}

void Engine::tick(float deltaSeconds)
{
	if (!mInitialized)
	{
		return;
	}

	mContext->deltaSeconds = deltaSeconds;
	mContext->timeSeconds += static_cast<double>(deltaSeconds);
	++mTickCount;

	for (const auto& subsystem : mSubsystems)
	{
		if (subsystem)
		{
			subsystem->tick(*mContext);
		}
	}

	if (mActiveWorld)
	{
		mActiveWorld->tick(deltaSeconds);
	}
}

void Engine::shutdown()
{
	if (!mInitialized)
	{
		return;
	}

	if (mActiveWorld)
	{
		mActiveWorld->endPlay();
		mActiveWorld.reset();
	}

	for (auto iter = mSubsystems.rbegin(); iter != mSubsystems.rend(); ++iter)
	{
		if (*iter)
		{
			(*iter)->shutdown(*mContext);
		}
	}
	mInitialized = false;
}

void Engine::initializeSubsystemIfNeeded(EngineSubsystem& subsystem)
{
	if (mInitialized)
	{
		subsystem.initialize(*mContext);
	}
}

bool Engine::isInitialized() const
{
	return mInitialized;
}

EngineContext& Engine::getContext()
{
	return *mContext;
}

const EngineContext& Engine::getContext() const
{
	return *mContext;
}

int Engine::getTickCount() const
{
	return mTickCount;
}

EngineLifecycleSnapshot Engine::captureLifecycleSnapshot() const
{
	EngineLifecycleSnapshot snapshot{};
	snapshot.initialized = mInitialized;
	snapshot.runMode = mContext->desc.runMode;
	snapshot.viewportWidth = mContext->desc.viewportWidth;
	snapshot.viewportHeight = mContext->desc.viewportHeight;
	snapshot.timeSeconds = mContext->timeSeconds;
	snapshot.deltaSeconds = mContext->deltaSeconds;
	snapshot.engineTickCount = mTickCount;
	snapshot.subsystemCount = static_cast<int>(mSubsystems.size());
	snapshot.subsystemSummaries.reserve(mSubsystems.size());
	for (std::size_t index = 0; index < mSubsystems.size(); ++index)
	{
		const auto& subsystem = mSubsystems[index];
		EngineSubsystemLifecycleSummary summary{};
		summary.index = static_cast<int>(index);
		summary.name = subsystem ? subsystem->getDebugName() : "null";
		summary.initialized = subsystem && subsystem->isInitializedForDiagnostics();
		summary.tickCount = subsystem ? subsystem->getTickCountForDiagnostics() : 0;
		if (summary.initialized)
		{
			++snapshot.initializedSubsystemCount;
		}
		if (summary.tickCount > 0)
		{
			++snapshot.tickedSubsystemCount;
		}
		snapshot.subsystemSummaries.push_back(std::move(summary));
	}
	if (mActiveWorld)
	{
		snapshot.activeWorldPresent = true;
		snapshot.activeWorldPlaying = mActiveWorld->isPlaying();
		snapshot.activeWorldName = mActiveWorld->getName();
		snapshot.activeWorldPersistentId = mActiveWorld->getPersistentId();
		snapshot.activeWorldTickCount = mActiveWorld->getTickCount();

		const auto* persistentLevel = mActiveWorld->getPersistentLevel();
		if (persistentLevel)
		{
			snapshot.activeWorldActorCount = static_cast<int>(persistentLevel->getActors().size());
		}
	}
	return snapshot;
}

World& Engine::createWorld(std::string name)
{
	if (mActiveWorld)
	{
		mActiveWorld->endPlay();
	}

	mActiveWorld = std::make_unique<World>(std::move(name));
	if (mInitialized)
	{
		mActiveWorld->beginPlay();
	}
	return *mActiveWorld;
}

World* Engine::getActiveWorld() const
{
	return mActiveWorld.get();
}

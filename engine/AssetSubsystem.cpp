#include "engine/AssetSubsystem.h"

#include <memory>

#include "engine/AssetRegistry.h"

using namespace GLengine;

AssetSubsystem::AssetSubsystem()
	: mRegistry(std::make_unique<AssetRegistry>())
{
}

AssetSubsystem::~AssetSubsystem() = default;

bool AssetSubsystem::initialize(EngineContext&)
{
	mTickCount = 0;
	mInitialized = true;
	return true;
}

void AssetSubsystem::tick(EngineContext&)
{
	++mTickCount;
	// Asset lifetime is explicit for now; no background streaming/update work is owned here yet.
}

void AssetSubsystem::shutdown(EngineContext&)
{
	clear();
	mTickCount = 0;
	mInitialized = false;
}

const char* AssetSubsystem::getDebugName() const
{
	return "AssetSubsystem";
}

bool AssetSubsystem::isInitializedForDiagnostics() const
{
	return isInitialized();
}

int AssetSubsystem::getTickCountForDiagnostics() const
{
	return getTickCount();
}

AssetRegistry& AssetSubsystem::getRegistry()
{
	return *mRegistry;
}

const AssetRegistry& AssetSubsystem::getRegistry() const
{
	return *mRegistry;
}

void AssetSubsystem::clear()
{
	if (mRegistry)
	{
		mRegistry->clear();
	}
}

bool AssetSubsystem::isInitialized() const
{
	return mInitialized;
}

int AssetSubsystem::getTickCount() const
{
	return mTickCount;
}

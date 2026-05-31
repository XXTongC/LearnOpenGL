#include "engine/AssetSubsystem.h"

using namespace GLengine;

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
	return mRegistry;
}

const AssetRegistry& AssetSubsystem::getRegistry() const
{
	return mRegistry;
}

void AssetSubsystem::clear()
{
	mRegistry.clear();
}

bool AssetSubsystem::isInitialized() const
{
	return mInitialized;
}

int AssetSubsystem::getTickCount() const
{
	return mTickCount;
}

#include "engine/EngineObject.h"

#include <atomic>
#include <utility>

using namespace GLengine;

namespace
{
	std::atomic<ObjectId> gNextObjectId{ 1 };
}

EngineObject::EngineObject(std::string name)
	: mObjectId(gNextObjectId.fetch_add(1, std::memory_order_relaxed))
	, mName(std::move(name))
{
}

ObjectId EngineObject::getObjectId() const
{
	return mObjectId;
}

const std::string& EngineObject::getPersistentId() const
{
	return mPersistentId;
}

void EngineObject::setPersistentId(std::string persistentId)
{
	mPersistentId = std::move(persistentId);
}

const std::string& EngineObject::getName() const
{
	return mName;
}

void EngineObject::setName(std::string name)
{
	mName = std::move(name);
}

#pragma once

#include <cstdint>
#include <string>

namespace GLengine
{
	using ObjectId = std::uint64_t;

	class EngineObject
	{
	public:
		virtual ~EngineObject() = default;

		ObjectId getObjectId() const;
		const std::string& getPersistentId() const;
		void setPersistentId(std::string persistentId);
		const std::string& getName() const;
		void setName(std::string name);

	protected:
		explicit EngineObject(std::string name = {});

	private:
		ObjectId mObjectId{ 0 };
		std::string mPersistentId{};
		std::string mName{};
	};
}

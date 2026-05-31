#pragma once

#include <memory>
#include <string>

#include "engine\EngineObject.h"
#include "engine\Level.h"

namespace GLengine
{
	class World : public EngineObject
	{
	public:
		explicit World(std::string name = {});
		~World() override = default;

		Level& createPersistentLevel(std::string name = "Persistent Level");
		Level* getPersistentLevel() const;

		void beginPlay();
		void tick(float deltaSeconds);
		void endPlay();
		bool isPlaying() const;
		int getTickCount() const;

	private:
		std::unique_ptr<Level> mPersistentLevel{ nullptr };
		int mTickCount{ 0 };
		bool mIsPlaying{ false };
	};
}

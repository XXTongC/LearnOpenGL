#pragma once

#include <memory>
#include <string>

#include "engine\EngineObject.h"

namespace GLengine
{
	class Level;

	class World : public EngineObject
	{
	public:
		explicit World(std::string name = {});
		~World() override;

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

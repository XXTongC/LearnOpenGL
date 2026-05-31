#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "engine\EngineObject.h"

namespace GLengine
{
	class Actor;
	class World;

	class Level : public EngineObject
	{
	public:
		explicit Level(std::string name = {});
		~Level() override;

		World* getWorld() const;
		const std::vector<std::unique_ptr<Actor>>& getActors() const;

		template <class T, class... Args>
		T& spawnActor(Args&&... args)
		{
			static_assert(std::is_base_of_v<Actor, T>, "T must derive from Actor.");

			auto actor = std::make_unique<T>(std::forward<Args>(args)...);
			T& actorRef = *actor;
			addActor(std::move(actor));
			return actorRef;
		}

		void beginPlay();
		void tick(float deltaSeconds);
		void endPlay();

	private:
		friend class World;

		void setWorld(World* world);
		void addActor(std::unique_ptr<Actor> actor);

		World* mWorld{ nullptr };
		std::vector<std::unique_ptr<Actor>> mActors{};
	};
}

#pragma once

#include "engine/EngineObject.h"

namespace GLengine
{
	class Actor;

	class ActorComponent : public EngineObject
	{
	public:
		explicit ActorComponent(std::string name = {});
		~ActorComponent() override = default;

		Actor* getOwner() const;
		bool isActive() const;
		void setActive(bool active);
		bool canTick() const;
		void setCanTick(bool canTick);

		virtual void onRegister() {}
		virtual void beginPlay() {}
		virtual void tick(float deltaSeconds) {}
		virtual void endPlay() {}

	private:
		friend class Actor;

		void setOwner(Actor* owner);

		Actor* mOwner{ nullptr };
		bool mActive{ true };
		bool mCanTick{ false };
	};
}

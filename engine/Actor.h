#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "engine\ActorComponent.h"
#include "engine\EngineObject.h"
#include "engine\SceneComponent.h"

namespace GLengine
{
	class Level;
	class World;

	class Actor : public EngineObject
	{
	public:
		explicit Actor(std::string name = {});
		~Actor() override = default;

		World* getWorld() const;
		Level* getLevel() const;
		SceneComponent* getRootComponent() const;
		void setRootComponent(SceneComponent* rootComponent);

		const std::vector<std::unique_ptr<ActorComponent>>& getComponents() const;

		template <class T, class... Args>
		T& createComponent(Args&&... args)
		{
			static_assert(std::is_base_of_v<ActorComponent, T>, "T must derive from ActorComponent.");

			auto component = std::make_unique<T>(std::forward<Args>(args)...);
			T& componentRef = *component;
			registerComponent(std::move(component));
			return componentRef;
		}

		virtual void beginPlay() {}
		virtual void tick(float deltaSeconds) {}
		virtual void endPlay() {}

		void beginPlayInternal();
		void tickInternal(float deltaSeconds);
		void endPlayInternal();

	private:
		friend class Level;

		void setOwningLevel(Level* level, World* world);
		void registerComponent(std::unique_ptr<ActorComponent> component);

		World* mWorld{ nullptr };
		Level* mLevel{ nullptr };
		SceneComponent* mRootComponent{ nullptr };
		std::vector<std::unique_ptr<ActorComponent>> mComponents{};
		bool mHasBegunPlay{ false };
	};
}

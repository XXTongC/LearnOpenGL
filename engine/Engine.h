#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "engine\EngineSubsystem.h"

namespace GLengine
{
	struct EngineContext;
	struct EngineDesc;
	struct EngineLifecycleSnapshot;
	class World;

	class Engine
	{
	public:
		Engine();
		~Engine();

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		bool initialize(const EngineDesc& desc);
		void tick(float deltaSeconds);
		void shutdown();

		bool isInitialized() const;
		EngineContext& getContext();
		const EngineContext& getContext() const;
		int getTickCount() const;
		EngineLifecycleSnapshot captureLifecycleSnapshot() const;

		World& createWorld(std::string name = "Untitled World");
		World* getActiveWorld() const;

		template <class T, class... Args>
		T& addSubsystem(Args&&... args)
		{
			static_assert(std::is_base_of_v<EngineSubsystem, T>, "T must derive from EngineSubsystem.");

			auto subsystem = std::make_unique<T>(std::forward<Args>(args)...);
			T& subsystemRef = *subsystem;
			mSubsystems.push_back(std::move(subsystem));
			if (mInitialized)
			{
				initializeSubsystemIfNeeded(subsystemRef);
			}
			return subsystemRef;
		}

	private:
		void initializeSubsystemIfNeeded(EngineSubsystem& subsystem);

		std::unique_ptr<EngineContext> mContext{};
		std::unique_ptr<World> mActiveWorld{ nullptr };
		std::vector<std::unique_ptr<EngineSubsystem>> mSubsystems{};
		int mTickCount{ 0 };
		bool mInitialized{ false };
	};
}

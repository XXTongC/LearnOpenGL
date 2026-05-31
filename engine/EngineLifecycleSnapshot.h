#pragma once

#include <string>
#include <vector>

#include "engine/EngineRunMode.h"

namespace GLengine
{
	struct EngineSubsystemLifecycleSummary
	{
		int index{ 0 };
		std::string name{};
		bool initialized{ false };
		int tickCount{ 0 };
	};

	struct EngineLifecycleSnapshot
	{
		bool initialized{ false };
		EngineRunMode runMode{ EngineRunMode::Editor };
		int viewportWidth{ 0 };
		int viewportHeight{ 0 };
		double timeSeconds{ 0.0 };
		float deltaSeconds{ 0.0f };
		int engineTickCount{ 0 };
		int subsystemCount{ 0 };
		int initializedSubsystemCount{ 0 };
		int tickedSubsystemCount{ 0 };
		std::vector<EngineSubsystemLifecycleSummary> subsystemSummaries{};
		bool activeWorldPresent{ false };
		bool activeWorldPlaying{ false };
		std::string activeWorldName{};
		std::string activeWorldPersistentId{};
		int activeWorldTickCount{ 0 };
		int activeWorldActorCount{ 0 };
	};
}

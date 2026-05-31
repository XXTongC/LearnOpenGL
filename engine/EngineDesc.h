#pragma once

#include <string>

#include "engine/EngineRunMode.h"

namespace GLengine
{
	struct EngineDesc
	{
		EngineRunMode runMode{ EngineRunMode::Editor };
		int viewportWidth{ 1280 };
		int viewportHeight{ 720 };
		std::string projectRoot{};
		std::string startupMap{};
		bool enableGpuDebug{ true };
	};
}

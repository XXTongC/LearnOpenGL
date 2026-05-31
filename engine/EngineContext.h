#pragma once

#include "engine/EngineDesc.h"

namespace GLengine
{
	struct EngineContext
	{
		EngineDesc desc{};
		double timeSeconds{ 0.0 };
		float deltaSeconds{ 0.0f };
	};
}

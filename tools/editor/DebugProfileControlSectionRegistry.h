#pragma once

#include <functional>
#include <string>

#include "KeyedSectionRegistry.h"

namespace GL_EDITOR
{
	struct DebugControllerContext;

	using DebugProfileControlSectionDraw = std::function<void(const DebugControllerContext&)>;

	struct DebugProfileControlSection
	{
		std::string key{};
		int order = 0;
		DebugProfileControlSectionDraw draw{};
	};

	using DebugProfileControlSectionRegistry = KeyedSectionRegistry<DebugProfileControlSection, DebugControllerContext>;
}

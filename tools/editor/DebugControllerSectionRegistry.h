#pragma once

#include <functional>
#include <string>

#include "KeyedSectionRegistry.h"

namespace GL_EDITOR
{
	struct DebugControllerContext;

	using DebugControllerSectionDraw = std::function<void(const DebugControllerContext&)>;

	struct DebugControllerSection
	{
		std::string key{};
		DebugControllerSectionDraw draw{};
	};

	using DebugControllerSectionRegistry = KeyedSectionRegistry<DebugControllerSection, DebugControllerContext>;
}

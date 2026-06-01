#pragma once

#include "DebugControllerSectionRegistry.h"

namespace GL_EDITOR
{
	void registerDefaultDebugControllerSections(DebugControllerSectionRegistry& registry);
	const DebugControllerSectionRegistry& defaultDebugControllerSectionRegistry();
}

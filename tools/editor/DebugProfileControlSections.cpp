#include "DebugProfileControlSections.h"

#include "DebugPipelineProfileControlSections.h"
#include "DebugProfileControlSectionRegistry.h"
#include "DebugSceneProfileControlSections.h"

namespace
{
	GL_EDITOR::DebugProfileControlSectionRegistry makeDefaultDebugPipelineProfileControlSectionRegistry()
	{
		GL_EDITOR::DebugProfileControlSectionRegistry registry{};
		GL_EDITOR::registerDefaultDebugPipelineProfileControlSections(registry);
		return registry;
	}

	GL_EDITOR::DebugProfileControlSectionRegistry makeDefaultDebugSceneProfileControlSectionRegistry()
	{
		GL_EDITOR::DebugProfileControlSectionRegistry registry{};
		GL_EDITOR::registerDefaultDebugSceneProfileControlSections(registry);
		return registry;
	}
}

const GL_EDITOR::DebugProfileControlSectionRegistry& GL_EDITOR::defaultDebugPipelineProfileControlSectionRegistry()
{
	static const DebugProfileControlSectionRegistry registry = makeDefaultDebugPipelineProfileControlSectionRegistry();
	return registry;
}

const GL_EDITOR::DebugProfileControlSectionRegistry& GL_EDITOR::defaultDebugSceneProfileControlSectionRegistry()
{
	static const DebugProfileControlSectionRegistry registry = makeDefaultDebugSceneProfileControlSectionRegistry();
	return registry;
}

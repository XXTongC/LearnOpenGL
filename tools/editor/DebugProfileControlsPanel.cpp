#include "DebugProfileControlsPanel.h"

#include "DebugProfileControlSectionRegistry.h"
#include "DebugProfileControlSections.h"

void GL_EDITOR::drawDebugPipelineProfileControls(const DebugControllerContext& context)
{
	GL_EDITOR::defaultDebugPipelineProfileControlSectionRegistry().drawAll(context);
}

void GL_EDITOR::drawDebugSceneProfileControls(const DebugControllerContext& context)
{
	GL_EDITOR::defaultDebugSceneProfileControlSectionRegistry().drawAll(context);
}

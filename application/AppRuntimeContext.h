#pragma once

#include "RuntimeCameraLightState.h"
#include "RuntimeEngineAttachmentState.h"
#include "RuntimeProfileState.h"
#include "RuntimeRenderResourceState.h"

namespace GLframework
{
	struct AppRuntimeContext
	{
		GL_RUNTIME::RuntimeRenderResourceState renderResources{};
		GL_RUNTIME::RuntimeCameraLightState cameraLights{};
		GL_RUNTIME::RuntimeEngineAttachmentState engineAttachments{};
		GL_RUNTIME::RuntimeProfileState profiles{};
	};
}

#include "RuntimeContentCameraLifecycle.h"

#include "RuntimeCameraConfig.h"
#include "RuntimeCameraLifecycle.h"

namespace GL_RUNTIME
{
	void RuntimeContentCameraLifecycle::initializeCamera(
		GLframework::AppRuntimeContext& context,
		const RuntimeCameraConfig& config
	)
	{
		RuntimeCameraLifecycle::initializeDefaultCamera(context, config);
	}
}

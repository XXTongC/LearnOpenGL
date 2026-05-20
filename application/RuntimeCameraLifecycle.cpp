#include "RuntimeCameraLifecycle.h"

#include "../camera/gamecameracontrol.h"
#include "../camera/perspectivecamera.h"
#include "../tools/Logger/LogManager.h"

namespace GL_RUNTIME
{
	namespace
	{
		float calculateAspect(int width, int height)
		{
			if (width <= 0 || height <= 0)
			{
				return 1.0f;
			}

			return static_cast<float>(width) / static_cast<float>(height);
		}
	}

	void RuntimeCameraLifecycle::initializeDefaultCamera(
		GLframework::AppRuntimeContext& context,
		const RuntimeCameraConfig& config
	)
	{
		LogInfo(static_cast<std::string>(__func__) + "(): Camera preparing...");

		cleanup(context);
		context.camera = new PerspectiveCamera(
			config.fovy,
			calculateAspect(config.width, config.height),
			config.nearPlane,
			config.farPlane
		);
		context.cameracontrol = new GameCameraControl();
		context.cameracontrol->setCamera(context.camera);

		LogInfo("Camera prepared");
	}

	void RuntimeCameraLifecycle::cleanup(GLframework::AppRuntimeContext& context)
	{
		delete context.cameracontrol;
		context.cameracontrol = nullptr;

		delete context.camera;
		context.camera = nullptr;
	}
}

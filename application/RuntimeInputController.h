#pragma once

#include "camera/camera.h"
#include "camera/cameracontrol.h"

namespace GL_RUNTIME
{
	struct RuntimeInputContext
	{
		Camera* camera{ nullptr };
		CameraControl* cameraControl{ nullptr };
	};

	class RuntimeInputController
	{
	public:
		static void handleScroll(double offset, const RuntimeInputContext& context);
		static void handleKey(int key, int action, int mods, const RuntimeInputContext& context);
		static void handleMouse(int button, int action, double x, double y, const RuntimeInputContext& context);
		static void handleCursor(double x, double y, const RuntimeInputContext& context);

	private:
		static void applyTemporaryFovZoom(int button, int action, const RuntimeInputContext& context);
	};
}

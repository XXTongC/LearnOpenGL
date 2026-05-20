#include "RuntimeInputController.h"

#include "GL_ERROR_FIND.h"
#include "camera/gamecameracontrol.h"
#include "camera/perspectivecamera.h"
#include "core.h"

using namespace GL_RUNTIME;

void RuntimeInputController::handleScroll(double offset, const RuntimeInputContext& context)
{
	if (context.cameraControl == nullptr)
	{
		return;
	}

	context.cameraControl->onScroll(static_cast<float>(offset));
}

void RuntimeInputController::handleKey(int key, int action, int mods, const RuntimeInputContext& context)
{
	if (context.cameraControl == nullptr)
	{
		return;
	}

	GL_CALL(context.cameraControl->onKey(key, action, mods));
}

void RuntimeInputController::handleMouse(
	int button,
	int action,
	double x,
	double y,
	const RuntimeInputContext& context
)
{
	if (context.cameraControl == nullptr)
	{
		return;
	}

	context.cameraControl->onMouse(button, action, x, y);
	applyTemporaryFovZoom(button, action, context);
}

void RuntimeInputController::handleCursor(double x, double y, const RuntimeInputContext& context)
{
	if (context.cameraControl == nullptr)
	{
		return;
	}

	context.cameraControl->onCursor(x, y);
}

void RuntimeInputController::applyTemporaryFovZoom(int button, int action, const RuntimeInputContext& context)
{
	if (button != GLFW_MOUSE_BUTTON_MIDDLE)
	{
		return;
	}

	if (dynamic_cast<GameCameraControl*>(context.cameraControl) == nullptr)
	{
		return;
	}

	auto perspectiveCamera = dynamic_cast<PerspectiveCamera*>(context.camera);
	if (perspectiveCamera == nullptr)
	{
		return;
	}

	if (action == GLFW_PRESS)
	{
		perspectiveCamera->setFovy(perspectiveCamera->mFovy / 2.0f);
	}
	else if (action == GLFW_RELEASE)
	{
		perspectiveCamera->setFovy(perspectiveCamera->mFovy * 2.0f);
	}
}

#include "RuntimeWindowLifecycle.h"

#include <iostream>

#include "Application.h"
#include "AppRuntimeContext.h"
#include "RuntimeInputController.h"
#include "RuntimeWindowLifecycleTypes.h"
#include "RuntimeWindowRenderResourceAdapter.h"
#include "../tools/Logger/LogManager.h"

namespace GL_RUNTIME
{
	namespace
	{
		RuntimeWindowCallbackContext gCallbackContext{};

		RuntimeInputContext makeInputContext()
		{
			if (gCallbackContext.runtime == nullptr)
			{
				return {};
			}

			return {
				gCallbackContext.runtime->cameraLights.camera,
				gCallbackContext.runtime->cameraLights.cameracontrol
			};
		}

		void onScroll(double offset)
		{
			RuntimeInputController::handleScroll(offset, makeInputContext());
		}

		void onResize(int newWidth, int newHeight)
		{
			if (gCallbackContext.runtime == nullptr)
			{
				return;
			}

			const bool resizeAccepted = RuntimeWindowRenderResourceAdapter::applyResize(
				gCallbackContext.runtime->renderResources,
				gCallbackContext.runtime->cameraLights.camera,
				gCallbackContext.width,
				gCallbackContext.height,
				newWidth,
				newHeight
			);

#ifdef _DEBUG
			if (resizeAccepted)
			{
				std::cout << "OnResize" << std::endl;
			}
#endif
		}

		void onKeyboard(int key, int action, int mods)
		{
			RuntimeInputController::handleKey(key, action, mods, makeInputContext());
#ifdef _DEBUG
			std::cout << "OnKeyboardCallback Pressed: " << key << " " << action << " " << mods << std::endl;
#endif
		}

		void onMouse(int button, int action, int mods)
		{
			double x = 0.0;
			double y = 0.0;
			GL_APP->getCursorPosition(&x, &y);
#ifdef _DEBUG
			std::cout << "OnMouseCallback : " << button << " " << action << " " << mods << std::endl;
#endif
			RuntimeInputController::handleMouse(button, action, x, y, makeInputContext());
		}

		void onCursor(double xpos, double ypos)
		{
			RuntimeInputController::handleCursor(xpos, ypos, makeInputContext());
		}
	}

	bool RuntimeWindowLifecycle::initialize(
		const RuntimeWindowConfig& config,
		const RuntimeWindowCallbackContext& callbackContext
	)
	{
		LogInfo("Window Initializing...");
		if (!GL_APP->init(config.width, config.height))
		{
			return false;
		}

		gCallbackContext = callbackContext;
		GL_APP->setResizeCallback(onResize);
		GL_APP->setKeyboardCallback(onKeyboard);
		GL_APP->setMouseCallback(onMouse);
		GL_APP->setCursorCallback(onCursor);
		GL_APP->setScrollCallback(onScroll);

		LogInfo("Window Initialized");
		return true;
	}

	RuntimeWindowSnapshot RuntimeWindowLifecycle::captureSnapshot()
	{
		return {
			static_cast<int>(GL_APP->getWidth()),
			static_cast<int>(GL_APP->getHeight()),
			GL_APP->getWindow()
		};
	}

	void RuntimeWindowLifecycle::destroy()
	{
		GL_APP->destroy();
	}
}

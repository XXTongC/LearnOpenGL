#include "RuntimeGraphicsLifecycle.h"

#include <iostream>

#include "RuntimeViewport.h"
#include "core.h"

namespace GL_RUNTIME
{
	void RuntimeGraphicsLifecycle::reportWindowSetupPrompt()
	{
		std::cout << "Please set the window as x * y" << std::endl;
	}

	void RuntimeGraphicsLifecycle::initializeAfterWindow(const RuntimeGraphicsLifecycleConfig& config)
	{
		RuntimeViewport::applyViewport(config.viewportWidth, config.viewportHeight);
		GL_CALL(glClearColor(
			config.clearColorRed,
			config.clearColorGreen,
			config.clearColorBlue,
			config.clearColorAlpha
		));

		if (!config.reportOpenGLCapabilities)
		{
			return;
		}

		int vertexAttributeCount = 0;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &vertexAttributeCount);
		std::cout << "Maximum nr of vertex attributes supported: " << vertexAttributeCount << std::endl;
	}
}

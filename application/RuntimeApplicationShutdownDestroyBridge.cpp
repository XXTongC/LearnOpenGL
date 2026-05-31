#include "RuntimeApplicationShutdownDestroyBridge.h"

#include "RuntimeWindowLifecycle.h"

namespace GL_RUNTIME
{
	void RuntimeApplicationShutdownDestroyBridge::destroy()
	{
		RuntimeWindowLifecycle::destroy();
	}
}

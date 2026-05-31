#include "RuntimeApplicationConfig.h"

#include "RuntimeRendererBackendKeys.h"

namespace GL_RUNTIME
{
	RuntimeApplicationShellConfig::RuntimeApplicationShellConfig()
		: rendererBackendKey(RuntimeRendererBackendKeys::defaultBackendKey())
	{
	}
}

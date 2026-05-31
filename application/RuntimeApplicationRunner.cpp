#include "RuntimeApplicationRunner.h"

#include <utility>

#include "RuntimeApplicationConfig.h"
#include "RuntimeApplicationShell.h"
#include "RuntimeBootstrapper.h"
#include "RuntimeBootstrapperCallbacks.h"

namespace GL_RUNTIME
{
	int RuntimeApplicationRunner::run(RuntimeApplicationShellConfig config)
	{
		RuntimeApplicationShell shell{ std::move(config) };
		return RuntimeBootstrapper::run(shell.makeCallbacks());
	}
}

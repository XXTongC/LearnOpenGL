#include "RuntimeApplicationShell.h"

#include <memory>
#include <utility>

#include "RuntimeApplicationCallbackBinder.h"
#include "RuntimeApplicationConfig.h"
#include "RuntimeApplicationState.h"
#include "RuntimeBootstrapperCallbacks.h"

namespace GL_RUNTIME
{
	RuntimeApplicationShell::RuntimeApplicationShell()
		: mState(std::make_unique<RuntimeApplicationState>())
		, mConfig(std::make_unique<RuntimeApplicationShellConfig>())
	{
	}

	RuntimeApplicationShell::RuntimeApplicationShell(RuntimeApplicationShellConfig config)
		: mState(std::make_unique<RuntimeApplicationState>())
		, mConfig(std::make_unique<RuntimeApplicationShellConfig>(std::move(config)))
	{
	}

	RuntimeApplicationShell::~RuntimeApplicationShell() = default;
	RuntimeApplicationShell::RuntimeApplicationShell(RuntimeApplicationShell&&) noexcept = default;
	RuntimeApplicationShell& RuntimeApplicationShell::operator=(RuntimeApplicationShell&&) noexcept = default;

	RuntimeBootstrapperCallbacks RuntimeApplicationShell::makeCallbacks()
	{
		return RuntimeApplicationCallbackBinder::makeCallbacks(*mState, *mConfig);
	}
}

#pragma once

#include <memory>

#include "RuntimeApplicationConfig.h"

namespace GL_RUNTIME
{
	struct RuntimeBootstrapperCallbacks;
	struct RuntimeApplicationState;

	class RuntimeApplicationShell
	{
	public:
		RuntimeApplicationShell();
		explicit RuntimeApplicationShell(RuntimeApplicationShellConfig config);
		~RuntimeApplicationShell();

		RuntimeApplicationShell(const RuntimeApplicationShell&) = delete;
		RuntimeApplicationShell& operator=(const RuntimeApplicationShell&) = delete;
		RuntimeApplicationShell(RuntimeApplicationShell&&) noexcept;
		RuntimeApplicationShell& operator=(RuntimeApplicationShell&&) noexcept;

		RuntimeBootstrapperCallbacks makeCallbacks();

	private:
		std::unique_ptr<RuntimeApplicationState> mState{};
		RuntimeApplicationShellConfig mConfig{};
	};
}

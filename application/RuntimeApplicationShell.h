#pragma once

#include <memory>

namespace GL_RUNTIME
{
	struct RuntimeBootstrapperCallbacks;
	struct RuntimeApplicationShellConfig;
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
		std::unique_ptr<RuntimeApplicationShellConfig> mConfig{};
	};
}

#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;
	struct RuntimeApplicationState;
	struct RuntimeBootstrapperCallbacks;

	class RuntimeApplicationCallbackBinder
	{
	public:
		static RuntimeBootstrapperCallbacks makeCallbacks(
			RuntimeApplicationState& state,
			RuntimeApplicationShellConfig& config
		);
	};
}

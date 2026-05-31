#pragma once

namespace GL_RUNTIME
{
	struct RuntimeBootstrapperCallbacks;

	class RuntimeBootstrapper
	{
	public:
		static int run(const RuntimeBootstrapperCallbacks& callbacks);
	};
}

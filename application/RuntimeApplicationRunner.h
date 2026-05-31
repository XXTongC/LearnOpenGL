#pragma once

namespace GL_RUNTIME
{
	struct RuntimeApplicationShellConfig;

	class RuntimeApplicationRunner
	{
	public:
		static int run(RuntimeApplicationShellConfig config);
	};
}

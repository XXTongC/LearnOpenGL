#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	class RuntimeProfileLoader
	{
	public:
		static void loadAll(GLframework::AppRuntimeContext& context);
	};
}

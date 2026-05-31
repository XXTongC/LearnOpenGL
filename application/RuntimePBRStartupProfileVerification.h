#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	class RuntimePBRStartupProfileVerification
	{
	public:
		static void applyStartupProfile(GLframework::AppRuntimeContext& context);
	};
}

#pragma once

namespace GL_RUNTIME
{
	struct RuntimeGuiFrameContext;
	struct RuntimeGuiInitContext;

	class RuntimeGuiHost
	{
	public:
		static void initialize(const RuntimeGuiInitContext& context);
		static void renderFrame(const RuntimeGuiFrameContext& context);
	};
}

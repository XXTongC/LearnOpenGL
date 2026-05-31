#pragma once

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineConfig;

	class RuntimeSceneColorPass
	{
	public:
		static void execute(GLframework::AppRuntimeContext& context);
	};

	class RuntimeSceneResolvePass
	{
	public:
		static void execute(GLframework::AppRuntimeContext& context);
	};

	class RuntimeBloomPass
	{
	public:
		static void execute(GLframework::AppRuntimeContext& context);
	};

	class RuntimeScreenCompositePass
	{
	public:
		static void execute(
			GLframework::AppRuntimeContext& context,
			const RuntimeFramePipelineConfig& config
		);
	};
}

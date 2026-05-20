#pragma once

#include "AppRuntimeContext.h"
#include "RuntimeFramePipeline.h"

namespace GL_RUNTIME
{
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

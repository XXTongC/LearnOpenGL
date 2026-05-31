#pragma once

namespace GL_RUNTIME
{
	namespace RuntimeRendererBackendKeys
	{
		constexpr const char* runtimeFramePipelineBackendKey()
		{
			return "runtime-frame-pipeline-adapter";
		}

		constexpr const char* testNoOpBackendKey()
		{
			return "test-noop-renderer-backend";
		}

		constexpr const char* defaultBackendKey()
		{
			return runtimeFramePipelineBackendKey();
		}
	}
}

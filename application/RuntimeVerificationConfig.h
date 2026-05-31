#pragma once

#include <string>

#include "RuntimePBRVerificationConfig.h"

namespace GL_RUNTIME
{
	struct RuntimeEngineWorldVerificationConfig
	{
		bool enableSceneProbe{ false };
		bool enableMinimalScene{ false };
		bool enableEditorCreate{ false };
		bool enableScenePackageRoundTrip{ false };
	};

	struct RuntimeRendererVerificationConfig
	{
		bool enableGpuTimingProbe{ false };
	};

	struct RuntimeVerificationConfig
	{
		bool enabled{ false };
		int maxFrames{ 3 };
		int captureFrame{ 2 };
		std::string capturePath{ "out/pbr_verification.ppm" };
		RuntimePBRVerificationConfig pbr{};
		RuntimeEngineWorldVerificationConfig engineWorld{};
		RuntimeRendererVerificationConfig renderer{};
	};
}

#include "RuntimeApplicationShell.h"
#include "RuntimeBootstrapper.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "tools/Logger/LogManager.h"

#include <string>

int GLframework::PointLightShadow::MAX_POINT_LIGHTS = 2;

namespace
{
	bool hasArgument(int argc, char** argv, const std::string& expected)
	{
		for (int index = 1; index < argc; ++index)
		{
			if (expected == argv[index])
			{
				return true;
			}
		}

		return false;
	}

	GL_RUNTIME::RuntimeApplicationShellConfig makeShellConfig(int argc, char** argv)
	{
		GL_RUNTIME::RuntimeApplicationShellConfig config{};
		if (hasArgument(argc, argv, "--verify-pbr") || hasArgument(argc, argv, "--verify-pbr-ibl-debug"))
		{
			config.window = { 1280, 720 };
			config.enableGui = false;
			config.pbrVerification.enabled = true;
			config.pbrVerification.maxFrames = 3;
			config.pbrVerification.captureFrame = 2;
			config.pbrVerification.capturePath = "out/pbr_verification.ppm";
			if (hasArgument(argc, argv, "--verify-pbr-ibl-debug"))
			{
				config.pbrVerification.enableIblDebugPass = true;
				config.pbrVerification.capturePath = "out/pbr_ibl_debug_verification.ppm";
			}
		}

		return config;
	}
}

int main(int argc, char** argv)
{
	LogManager::getInstance().setMinLevel(LogManager::Level::info);
	GL_RUNTIME::RuntimeApplicationShell shell{ makeShellConfig(argc, argv) };
	return GL_RUNTIME::RuntimeBootstrapper::run(shell.makeCallbacks());
}

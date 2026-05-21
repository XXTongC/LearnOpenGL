#include "RuntimeApplicationShell.h"
#include "RuntimeBootstrapper.h"
#include "RuntimePBRVerificationArgs.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "tools/Logger/LogManager.h"

int GLframework::PointLightShadow::MAX_POINT_LIGHTS = 2;

int main(int argc, char** argv)
{
	LogManager::getInstance().setMinLevel(LogManager::Level::info);
	GL_RUNTIME::RuntimeApplicationShell shell{ GL_RUNTIME::makeShellConfigFromArguments(argc, argv) };
	return GL_RUNTIME::RuntimeBootstrapper::run(shell.makeCallbacks());
}

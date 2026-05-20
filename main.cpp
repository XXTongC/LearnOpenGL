#include "RuntimeApplicationShell.h"
#include "RuntimeBootstrapper.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "tools/Logger/LogManager.h"

int GLframework::PointLightShadow::MAX_POINT_LIGHTS = 2;

int main()
{
	LogManager::getInstance().setMinLevel(LogManager::Level::info);
	GL_RUNTIME::RuntimeApplicationShell shell{};
	return GL_RUNTIME::RuntimeBootstrapper::run(shell.makeCallbacks());
}

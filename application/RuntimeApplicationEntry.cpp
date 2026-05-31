#include "RuntimeApplicationEntry.h"

#include "RuntimeApplicationConfig.h"
#include "RuntimeApplicationRunner.h"
#include "RuntimeVerificationArgs.h"
#include "../tools/Logger/LogManager.h"

namespace GL_RUNTIME
{
	int RuntimeApplicationEntry::run(int argc, char** argv)
	{
		LogManager::getInstance().setMinLevel(LogManager::Level::info);
		return RuntimeApplicationRunner::run(makeShellConfigFromArguments(argc, argv));
	}
}

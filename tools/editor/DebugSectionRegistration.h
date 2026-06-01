#pragma once

#include <cassert>
#include <utility>

namespace GL_EDITOR
{
	template <typename Registry>
	void registerRequiredDebugSection(Registry& registry, typename Registry::SectionType section)
	{
		const bool registered = registry.registerSection(std::move(section));
		(void)registered;
		assert(registered && registry.lastRegistrationFailure().empty());
	}
}

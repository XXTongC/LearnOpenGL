#include "DebugProfileControlSectionRegistry.h"

#include <utility>

namespace GL_EDITOR
{
	bool DebugProfileControlSectionRegistry::registerSection(DebugProfileControlSection section)
	{
		if (section.key.empty() || !section.draw)
		{
			return false;
		}

		for (const auto& existingSection : mSections)
		{
			if (existingSection.key == section.key)
			{
				return false;
			}
		}

		mSections.push_back(std::move(section));
		return true;
	}

	std::size_t DebugProfileControlSectionRegistry::drawAll(const DebugControllerContext& context) const
	{
		std::size_t drawnSections = 0;
		for (const auto& section : mSections)
		{
			section.draw(context);
			++drawnSections;
		}

		return drawnSections;
	}
}

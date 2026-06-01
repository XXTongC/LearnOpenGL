#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace GL_EDITOR
{
	template <typename Section, typename Context>
	class KeyedSectionRegistry
	{
	public:
		bool registerSection(Section section)
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

		std::size_t drawAll(const Context& context) const
		{
			std::size_t drawnSections = 0;
			for (const auto& section : mSections)
			{
				section.draw(context);
				++drawnSections;
			}

			return drawnSections;
		}

	private:
		std::vector<Section> mSections{};
	};
}

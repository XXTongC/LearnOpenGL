#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
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
			mLastRegistrationFailure.clear();

			if (section.key.empty())
			{
				mLastRegistrationFailure = "section key is empty";
				return false;
			}

			if (!section.draw)
			{
				mLastRegistrationFailure = "section draw callback is empty: " + section.key;
				return false;
			}

			for (const auto& existingSection : mSections)
			{
				if (existingSection.key == section.key)
				{
					mLastRegistrationFailure = "duplicate section key: " + section.key;
					return false;
				}
			}

			const auto insertPosition = std::upper_bound(
				mSections.begin(),
				mSections.end(),
				section.order,
				[](const int order, const Section& existingSection)
				{
					return order < existingSection.order;
				}
			);
			mSections.insert(insertPosition, std::move(section));
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

		std::size_t sectionCount() const
		{
			return mSections.size();
		}

		const std::string& lastRegistrationFailure() const
		{
			return mLastRegistrationFailure;
		}

	private:
		std::vector<Section> mSections{};
		std::string mLastRegistrationFailure{};
	};
}

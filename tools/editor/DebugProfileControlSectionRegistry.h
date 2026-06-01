#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace GL_EDITOR
{
	struct DebugControllerContext;

	using DebugProfileControlSectionDraw = std::function<void(const DebugControllerContext&)>;

	struct DebugProfileControlSection
	{
		std::string key{};
		DebugProfileControlSectionDraw draw{};
	};

	class DebugProfileControlSectionRegistry
	{
	public:
		bool registerSection(DebugProfileControlSection section);
		std::size_t drawAll(const DebugControllerContext& context) const;

	private:
		std::vector<DebugProfileControlSection> mSections{};
	};
}

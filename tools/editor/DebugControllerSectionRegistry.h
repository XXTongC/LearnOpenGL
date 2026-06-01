#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace GL_EDITOR
{
	struct DebugControllerContext;

	using DebugControllerSectionDraw = std::function<void(const DebugControllerContext&)>;

	struct DebugControllerSection
	{
		std::string key{};
		DebugControllerSectionDraw draw{};
	};

	class DebugControllerSectionRegistry
	{
	public:
		bool registerSection(DebugControllerSection section);
		std::size_t drawAll(const DebugControllerContext& context) const;

	private:
		std::vector<DebugControllerSection> mSections{};
	};
}

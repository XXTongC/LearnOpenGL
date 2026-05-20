#pragma once

#include <string>

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GLframework
{
	enum class ToneMappingMode
	{
		Exposure = 0,
		Reinhard = 1,
	};

	struct PostProcessSettings
	{
		float exposure{ 1.0f };
		ToneMappingMode toneMappingMode{ ToneMappingMode::Exposure };

		bool bloomEnabled{ true };
		float bloomThreshold{ 1.0f };
		float bloomIntensity{ 0.04f };
		int bloomIterations{ 6 };

		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder);
	};

	class PostProcessSettingsStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, PostProcessSettings& settings);
		static bool saveToFile(const std::string& path, const PostProcessSettings& settings);
	};
}

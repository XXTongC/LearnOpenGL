#pragma once

#include <string>

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GLframework
{
	struct RendererFramePassProfile
	{
		RendererFramePassProfile();

		std::string defaultPassOrder{};
		std::string globalMaterialOverridePassOrder{};
		int pbrGBufferDebugMode{ 0 };
		float pbrGBufferDebugIntensity{ 1.0f };
		int iblDebugMode{ 0 };
		float iblDebugMipLevel{ 0.0f };
		float iblDebugIntensity{ 1.0f };

		void resetToDefaults();
		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder);
	};

	class RendererFramePassProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, RendererFramePassProfile& profile);
		static bool saveToFile(const std::string& path, const RendererFramePassProfile& profile);
	};
}

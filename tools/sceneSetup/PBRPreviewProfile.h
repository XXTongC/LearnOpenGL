#pragma once

#include <string>

#include "../../materials/pbrMaterial/PBRMaterialProfile.h"
#include "../../third_party/glm/glm.hpp"

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GL_SCENE
{
	struct PBRPreviewProfile
	{
		PBRPreviewProfile();

		bool enabled{ true };
		glm::vec3 position{ 2.2f, -3.9f, 2.0f };
		float radius{ 0.75f };
		int segments{ 48 };
		int rings{ 24 };
		bool useMaterialGrid{ false };
		int gridColumns{ 5 };
		int gridRows{ 5 };
		float gridSpacing{ 1.1f };
		float gridRadius{ 0.42f };
		float gridMetallicMin{ 0.0f };
		float gridMetallicMax{ 1.0f };
		float gridRoughnessMin{ 0.08f };
		float gridRoughnessMax{ 1.0f };

		std::string materialProfilePath{};
		GLframework::PBRMaterialProfile material{};

		std::string normalMapPath{ "Texture/normal/normal_map.png" };
		unsigned int normalMapUnit{ 4 };

		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder);
	};

	class PBRPreviewProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, PBRPreviewProfile& profile);
		static bool saveToFile(const std::string& path, const PBRPreviewProfile& profile);
		static bool applyMaterialProfileReference(PBRPreviewProfile& profile);
	};
}

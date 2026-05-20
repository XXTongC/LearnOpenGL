#pragma once

#include <string>

#include "../../third_party/glm/glm.hpp"

namespace GL_SCENE
{
	struct PBRPreviewProfile
	{
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

		glm::vec3 albedo{ 0.9f, 0.42f, 0.18f };
		float metallic{ 0.2f };
		float roughness{ 0.35f };
		float ao{ 1.0f };
		bool useIBL{ true };
		float iblDiffuseStrength{ 1.0f };
		float iblSpecularStrength{ 1.0f };

		std::string normalMapPath{ "Texture/normal/normal_map.png" };
		unsigned int normalMapUnit{ 4 };
	};

	class PBRPreviewProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, PBRPreviewProfile& profile);
		static bool saveToFile(const std::string& path, const PBRPreviewProfile& profile);
	};
}

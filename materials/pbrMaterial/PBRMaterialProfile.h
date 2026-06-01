#pragma once

#include <string>

#include "../../third_party/glm/glm.hpp"

namespace GLframework
{
	class PBRMaterial;

	struct PBRMaterialProfile
	{
		glm::vec3 albedo{ 1.0f, 1.0f, 1.0f };
		glm::vec3 emissiveColor{ 0.0f, 0.0f, 0.0f };
		float metallic{ 0.0f };
		float roughness{ 0.5f };
		float ao{ 1.0f };
		float emissiveIntensity{ 0.0f };
		bool useAlphaMask{ false };
		float alphaCutoff{ 0.5f };
		bool useIBL{ false };
		float iblDiffuseStrength{ 1.0f };
		float iblSpecularStrength{ 1.0f };

		void applyTo(PBRMaterial& material) const;
		void copyFrom(const PBRMaterial& material);
	};

	class PBRMaterialProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, PBRMaterialProfile& profile);
		static bool saveToFile(const std::string& path, const PBRMaterialProfile& profile);
	};
}

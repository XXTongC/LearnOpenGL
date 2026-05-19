#pragma once

#include <memory>
#include <string>

#include "framework/texture.h"

namespace GLframework
{
	struct EnvironmentProfile
	{
		std::string hdrEquirectangularPath{};
		unsigned int hdrTextureUnit{ 9 };
		bool precomputeOnPrepare{ false };
		bool useProceduralEnvironment{ false };
		unsigned int proceduralWidth{ 512 };
		unsigned int proceduralHeight{ 256 };
		float proceduralSkyIntensity{ 1.5f };
		float proceduralGroundIntensity{ 0.15f };
		float proceduralSunIntensity{ 4.0f };

		bool hasHdrSource() const;
		bool hasEnvironmentSource() const;
	};

	class EnvironmentTextureLoader
	{
	public:
		static std::shared_ptr<Texture> loadEquirectangular(const EnvironmentProfile& profile);
		static std::shared_ptr<Texture> loadHdrEquirectangular(const EnvironmentProfile& profile);
		static std::shared_ptr<Texture> createProceduralEquirectangular(const EnvironmentProfile& profile);
	};

	class EnvironmentProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, EnvironmentProfile& profile);
		static bool saveToFile(const std::string& path, const EnvironmentProfile& profile);
	};
}

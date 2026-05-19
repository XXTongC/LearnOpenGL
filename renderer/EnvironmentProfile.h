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

		bool hasHdrSource() const;
	};

	class EnvironmentTextureLoader
	{
	public:
		static std::shared_ptr<Texture> loadHdrEquirectangular(const EnvironmentProfile& profile);
	};
}

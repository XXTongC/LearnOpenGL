#pragma once

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
	};
}

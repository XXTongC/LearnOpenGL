#pragma once

namespace GLframework
{
	enum class MaterialType
	{
		PhongMaterial,
		PBRMaterial,
		WhiteMaterial,
		DepthMaterial,
		OpacityMaskMaterial,
		ScreenMaterial,
		CubeMaterial,
		CubeSphereMaterial,
		PhongEnvMaterial,
		PhongEnvSphereMaterial,
		PhongInstanceMaterial,
		PhongNormalMaterial,
		GrassInstanceMaterial,
		PhongParallaxMaterial,
		PhongShadowMaterial,
		PhongCSMShadowMaterial,
		PhongPointShadowMaterial,
	};

	enum class PreStencilType
	{
		Normal,
		Outlining,
		Custom,
	};
}

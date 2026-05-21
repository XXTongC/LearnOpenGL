#include "ShaderLibrary.h"

#include <iostream>

void GLframework::ShaderLibrary::initialize()
{
	mMaterialShaders[MaterialType::PhongMaterial] = createShader("shaders/phong/phong_V2.vert", "shaders/phong/phong_V2.frag");
	mMaterialShaders[MaterialType::PBRMaterial] = createShader("shaders/pbr/pbr.vert", "shaders/pbr/pbr.frag");
	mMaterialShaders[MaterialType::WhiteMaterial] = createShader("shaders/white/white.vert", "shaders/white/white.frag");
	mMaterialShaders[MaterialType::DepthMaterial] = createShader("shaders/depth/depth.vert", "shaders/depth/depth.frag");
	mMaterialShaders[MaterialType::OpacityMaskMaterial] = createShader("shaders/opacityMask/phongOpacityMask.vert", "shaders/opacityMask/phongOpacityMask.frag");
	mMaterialShaders[MaterialType::ScreenMaterial] = createShader("shaders/screen/screen.vert", "shaders/screen/screen.frag");
	mMaterialShaders[MaterialType::CubeMaterial] = createShader("shaders/cube/cube.vert", "shaders/cube/cube.frag");
	mMaterialShaders[MaterialType::CubeSphereMaterial] = createShader("shaders/cube/cube.vert", "shaders/cube/cubeSphere.frag");
	mMaterialShaders[MaterialType::PhongEnvMaterial] = createShader("shaders/phongEnv/phongEnv.vert", "shaders/phongEnv/phongEnv_V2.frag");
	mMaterialShaders[MaterialType::PhongEnvSphereMaterial] = createShader("shaders/phongEnv/phongEnv.vert", "shaders/phongEnv/phongEnvSphere.frag");
	mMaterialShaders[MaterialType::PhongInstanceMaterial] = createShader("shaders/phongInstance/phongInstance.vert", "shaders/phongInstance/phongInstance.frag");
	mMaterialShaders[MaterialType::GrassInstanceMaterial] = createShader("shaders/grassInstance/grassInstance.vert", "shaders/grassInstance/grassInstance.frag");
	mMaterialShaders[MaterialType::PhongNormalMaterial] = createShader("shaders/phongNormal/phongNormal_V2.vert", "shaders/phongNormal/phongNormal_V2.frag");
	mMaterialShaders[MaterialType::PhongParallaxMaterial] = createShader("shaders/phongParallax/phongParallax_V2.vert", "shaders/phongParallax/phongParallax_V2.frag");
	mMaterialShaders[MaterialType::PhongShadowMaterial] = createShader("shaders/phong/phongShadow.vert", "shaders/phong/phongShadow.frag");
	mMaterialShaders[MaterialType::PhongCSMShadowMaterial] = createShader("shaders/phongCSMShadow/phongCSMShadow.vert", "shaders/phongCSMShadow/phongCSMShadow.frag");
	mMaterialShaders[MaterialType::PhongPointShadowMaterial] = createShader("shaders/phongPointShadow/phongPointShadow.vert", "shaders/phongPointShadow/phongPointShadow.frag");

	mShadowShader = createShader("shaders/shadow/shadow.vert", "shaders/shadow/shadow.frag");
	mShadowDistanceShader = createShader("shaders/shadowDistance/shadowDistance.vert", "shaders/shadowDistance/shadowDistance.frag");
	mEquirectangularToCubemapShader = createShader("shaders/ibl/capture.vert", "shaders/ibl/equirectangular_to_cubemap.frag");
	mIrradianceConvolutionShader = createShader("shaders/ibl/capture.vert", "shaders/ibl/irradiance_convolution.frag");
	mPrefilterShader = createShader("shaders/ibl/capture.vert", "shaders/ibl/prefilter.frag");
	mBrdfLutShader = createShader("shaders/ibl/brdf_lut.vert", "shaders/ibl/brdf_lut.frag");
	mIblDebugShader = createShader("shaders/diagnostics/ibl_debug.vert", "shaders/diagnostics/ibl_debug.frag");
	mPbrGBufferShader = createShader("shaders/pbr/pbr_gbuffer.vert", "shaders/pbr/pbr_gbuffer.frag");
	mPbrDeferredLightingShader = createShader("shaders/pbr/pbr_deferred_lighting.vert", "shaders/pbr/pbr_deferred_lighting.frag");
	mPbrDeferredTiledLightDebugShader = createShader("shaders/diagnostics/pbr_deferred_tiled_light_debug.vert", "shaders/diagnostics/pbr_deferred_tiled_light_debug.frag");
	mPbrGBufferDebugShader = createShader("shaders/diagnostics/pbr_gbuffer_debug.vert", "shaders/diagnostics/pbr_gbuffer_debug.frag");
	mPbrAlphaShadowShader = createShader("shaders/pbr/pbr_alpha_shadow.vert", "shaders/pbr/pbr_alpha_shadow.frag");
	mPbrAlphaPointShadowShader = createShader("shaders/pbr/pbr_alpha_point_shadow.vert", "shaders/pbr/pbr_alpha_point_shadow.frag");
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::get(MaterialType type) const
{
	const auto it = mMaterialShaders.find(type);
	if (it == mMaterialShaders.end())
	{
		std::cerr << "Unknown material type to pick shader\n";
		return nullptr;
	}

	return it->second;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getShadowShader() const
{
	return mShadowShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getShadowDistanceShader() const
{
	return mShadowDistanceShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getEquirectangularToCubemapShader() const
{
	return mEquirectangularToCubemapShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getIrradianceConvolutionShader() const
{
	return mIrradianceConvolutionShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getPrefilterShader() const
{
	return mPrefilterShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getBrdfLutShader() const
{
	return mBrdfLutShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getIblDebugShader() const
{
	return mIblDebugShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getPbrGBufferShader() const
{
	return mPbrGBufferShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getPbrDeferredLightingShader() const
{
	return mPbrDeferredLightingShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getPbrDeferredTiledLightDebugShader() const
{
	return mPbrDeferredTiledLightDebugShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getPbrGBufferDebugShader() const
{
	return mPbrGBufferDebugShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getPbrAlphaShadowShader() const
{
	return mPbrAlphaShadowShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::getPbrAlphaPointShadowShader() const
{
	return mPbrAlphaPointShadowShader;
}

std::shared_ptr<GLframework::Shader> GLframework::ShaderLibrary::createShader(const char* vertexPath, const char* fragmentPath)
{
	return std::make_shared<Shader>(vertexPath, fragmentPath);
}

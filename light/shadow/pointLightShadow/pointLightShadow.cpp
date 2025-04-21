#include "pointLightShadow.h"
#include "../perspectivecamera.h"
using namespace GLframework;
std::shared_ptr<Texture> PointLightShadow::mSharedDepthTexture = nullptr;

PointLightShadow::PointLightShadow(int maxLights,int shadowMapIndex)
{
    mShadowMapIndex = shadowMapIndex;
    mRenderTarget = Framebuffer::createShadowFBO(1024*6, 1024);
}

void PointLightShadow::setMAX_POINT_LIGHT(int value)
{
    MAX_POINT_LIGHTS = value;
}

int PointLightShadow::getMAX_POINT_LIGHT()
{
    return MAX_POINT_LIGHTS;
}

PointLightShadow::~PointLightShadow()
{
	
}
PointLightShadow::PointLightShadow()
{
    mCamera = std::make_shared<PerspectiveCamera>(90.0f, 1.0, 0.1f, 1000.0f);
    mRenderTarget = Framebuffer::createPointLightShadowFBO(1024, 1024, getMAX_POINT_LIGHT() * 6);
}

void PointLightShadow::setRenderTargetSize(int width, int height)
{
    if (mRenderTarget != nullptr) mRenderTarget.reset();
    mRenderTarget = Framebuffer::createPointLightShadowFBO(width,height, getMAX_POINT_LIGHT() * 6);
}

void PointLightShadow::initializeSharedDepthTexture(int width, int height, int maxLights)
{
    MAX_POINT_LIGHTS = maxLights;
	int layers = maxLights * 6; // 6 faces per cubemap
	mSharedDepthTexture = Texture::createTexture2DArray(width, height, layers, 0, GL_DEPTH_COMPONENT32F);
}




std::shared_ptr<Texture> PointLightShadow::getSharedDepthTexture()
{
    return mSharedDepthTexture;
}

void PointLightShadow::setShadowMapIndex(int value)
{
    mShadowMapIndex = value;
}

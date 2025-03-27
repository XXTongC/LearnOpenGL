#include "directionalLightShadow.h"
#include "../../../orthographiccamera.h"
using namespace GLframework;



DirectionalLightShadow::~DirectionalLightShadow()
{
	
}


DirectionalLightShadow::DirectionalLightShadow()
{
	float size = 10.0;
	mCamera =  std::make_shared<OrthographicCamera>(-size, size, -size, size, 0.1f, 80.0f);
	mRenderTarget = Framebuffer::createShadowFBO(1024,1024);
}


glm::mat4 DirectionalLightShadow::getLightMatrix(glm::mat4 lightModelMatrix)
{
	auto viewMatrix = glm::inverse(lightModelMatrix);
	auto projectionMatrix = mCamera->getProjectionMatrix();

	return projectionMatrix * viewMatrix;
}

void DirectionalLightShadow::setRenderTargetSize(int width, int height)
{
	if (mRenderTarget != nullptr) mRenderTarget.reset();
	mRenderTarget = Framebuffer::createShadowFBO(width, height);
}


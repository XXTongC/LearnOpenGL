#include "ShadowResourceBinder.h"

#include <string>

#include "camera/camera.h"
#include "framework/shader.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"

using namespace GLframework;

void ShadowResourceBinder::bindCSMShadowResources(
	const std::shared_ptr<Shader>& shader,
	Camera* camera,
	const std::shared_ptr<DirectionalLight>& dirLight,
	int textureUnit
)
{
	const auto dirCSMShadow = std::static_pointer_cast<DirectionalLightCSMShadow>(dirLight->getShadow());

	shader->setInt("csmLayerCount", dirCSMShadow->getLayerCount());
	std::vector<float> layers;
	dirCSMShadow->generateCascadeLayers(layers, camera->mNear, camera->mFar);
	shader->setFloatArray("csmLayers", layers.data(), static_cast<int>(layers.size()));

	shader->setInt("shadowMapSampler", textureUnit);
	dirCSMShadow->mRenderTarget->getDepthAttachment()->setUnit(textureUnit);
	dirCSMShadow->mRenderTarget->getDepthAttachment()->Bind();

	auto lightMatrices = dirCSMShadow->getLightMatrix(camera, dirLight->getDirection(), layers);
	shader->setMat4Array("lightMatrices", lightMatrices.data(), static_cast<int>(lightMatrices.size()));

	shader->setFloat("lightSize", dirCSMShadow->mLightSize);
	shader->setMat4("lightViewMatrix", glm::inverse(dirLight->getModelMatrix()));
	shader->setFloat("bias", dirCSMShadow->mBias);
	shader->setFloat("diskTightness", dirCSMShadow->mDiskTightness);
	shader->setFloat("pcfRadius", dirCSMShadow->mPcfRadius);
}

void ShadowResourceBinder::bindPointShadowResources(
	const std::shared_ptr<Shader>& shader,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	int textureUnit
)
{
	shader->setInt("pointShadowMaps", textureUnit);
	PointLightShadow::getSharedDepthTexture()->setUnit(textureUnit);
	PointLightShadow::getSharedDepthTexture()->Bind();

	for (size_t i = 0; i < pointLights.size(); i++)
	{
		const auto& pointLight = pointLights[i];
		const auto& pointShadow = std::static_pointer_cast<PointLightShadow>(pointLight->getShadow());
		std::string baseName = "pointLights[" + std::to_string(i) + "]";

		shader->setVector3(baseName + ".color", pointLight->getColor());
		shader->setVector3(baseName + ".position", pointLight->getPosition());
		shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
		shader->setFloat(baseName + ".k2", pointLight->getK2());
		shader->setFloat(baseName + ".k1", pointLight->getK1());
		shader->setFloat(baseName + ".k0", pointLight->getK0());
		shader->setFloat(baseName + ".far", pointShadow->mCamera->mFar);
		shader->setFloat(baseName + ".near", pointShadow->mCamera->mNear);
	}
	shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());
}

void ShadowResourceBinder::bindDirectionalFallbackShadow(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<DirectionalLight>& dirLight
)
{
	const auto dirShadow = dirLight->getShadow();

	glm::mat4 directionalLightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 80.0f);
	glm::mat4 directionalLightView = glm::lookAt(
		dirLight->getPosition(),
		dirLight->getPosition() + glm::vec3(1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	shader->setMat4("directionalLightSpaceMatrix", directionalLightProjection * directionalLightView);

	shader->setFloat("bias", dirShadow->mBias);
	shader->setFloat("diskTightness", dirShadow->mDiskTightness);
	shader->setFloat("pcfRadius", dirShadow->mPcfRadius);
}

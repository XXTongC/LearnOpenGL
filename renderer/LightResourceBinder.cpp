#include "LightResourceBinder.h"

#include <string>

#include "framework/shader.h"
#include "light/ambientLight.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "light/spotLight.h"

using namespace GLframework;

void LightResourceBinder::bindForwardLights(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::shared_ptr<SpotLight>& spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const std::shared_ptr<AmbientLight>& ambient
)
{
	shader->setVector3("spotLight.position", spotLight->getPosition());
	shader->setVector3("spotLight.color", spotLight->getColor());
	shader->setFloat("spotLight.specularIntensity", spotLight->getSpecularIntensity());
	shader->setVector3("spotLight.targetDirection", spotLight->getDirection());
	shader->setFloat("spotLight.innerLine", glm::cos(glm::radians(spotLight->getInnerAngle())));
	shader->setFloat("spotLight.outLine", glm::cos(glm::radians(spotLight->getOutAngle())));

	shader->setVector3("directionalLight.color", dirLight->getColor());
	shader->setVector3("directionalLight.direction", dirLight->getDirection());
	shader->setFloat("directionalLight.specularIntensity", dirLight->getSpecularIntensity());
	shader->setFloat("directionalLight.intensity", dirLight->getIntensity());

	for (size_t i = 0; i < pointLights.size(); i++)
	{
		const auto& pointLight = pointLights[i];
		std::string baseName = "pointLights[" + std::to_string(i) + "]";

		shader->setVector3(baseName + ".color", pointLight->getColor());
		shader->setVector3(baseName + ".position", pointLight->getPosition());
		shader->setFloat(baseName + ".specularIntensity", pointLight->getSpecularIntensity());
		shader->setFloat(baseName + ".k2", pointLight->getK2());
		shader->setFloat(baseName + ".k1", pointLight->getK1());
		shader->setFloat(baseName + ".k0", pointLight->getK0());
	}
	shader->setInt("POINT_LIGHT_NUM", PointLightShadow::getMAX_POINT_LIGHT());

	shader->setVector3("ambientColor", ambient->getColor());
}

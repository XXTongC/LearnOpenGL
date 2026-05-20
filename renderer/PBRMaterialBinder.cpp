#include "PBRMaterialBinder.h"

#include "camera/camera.h"
#include "renderer/LightResourceBinder.h"
#include "renderer/PBRIBLResourceBinder.h"
#include "renderer/PBRSurfaceResourceBinder.h"
#include "renderer/ShadowResourceBinder.h"

using namespace GLframework;

namespace
{
	void setMVPMatrices(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh, Camera* camera)
	{
		shader->setMat4("modelMatrix", mesh->getModelMatrix());
		shader->setMat4("viewMatrix", camera->getViewMatrix());
		shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
	}

	void setNormalMatrix(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh)
	{
		shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrix()))));
	}

	void setCommonMaterialUniforms(const std::shared_ptr<Shader>& shader, const std::shared_ptr<PBRMaterial>& material, Camera* camera)
	{
		shader->setFloat("opacity", material->getOpacity());
		shader->setFloat("time", static_cast<float>(glfwGetTime()));
		shader->setFloat("speed", 0.5f);
		shader->setVector3("cameraPosition", camera->mPosition);
	}

}

bool PBRMaterialBinder::bind(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<PBRMaterial>& material,
	const std::shared_ptr<Mesh>& mesh,
	const MaterialBindingContext& context
)
{
	if (!shader || !material || !mesh || context.camera == nullptr)
	{
		return false;
	}

	setCommonMaterialUniforms(shader, material, context.camera);
	setMVPMatrices(shader, mesh, context.camera);
	setNormalMatrix(shader, mesh);
	LightResourceBinder::bindForwardLights(shader, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
	ShadowResourceBinder::bindCSMShadowResources(shader, context.camera, context.dirLight, 8);
	PBRSurfaceResourceBinder::bind(shader, material);
	PBRIBLResourceBinder::bind(shader, material, context.environmentTargets);
	return true;
}

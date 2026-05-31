#include "PBRObjectUniformBinder.h"

#include "camera/camera.h"
#include "framework/shader.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "mesh/mesh.h"
#include "renderer/MaterialBindingContext.h"

using namespace GLframework;

namespace
{
	void bindMatrices(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh, Camera* camera)
	{
		shader->setMat4("modelMatrix", mesh->getModelMatrix());
		shader->setMat4("viewMatrix", camera->getViewMatrix());
		shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
		shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrix()))));
	}

	void bindCommonUniforms(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<PBRMaterial>& material,
		Camera* camera
	)
	{
		shader->setFloat("opacity", material->getOpacity());
		shader->setFloat("time", static_cast<float>(glfwGetTime()));
		shader->setFloat("speed", 0.5f);
		shader->setVector3("cameraPosition", camera->mPosition);
	}
}

bool PBRObjectUniformBinder::bind(
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

	bindCommonUniforms(shader, material, context.camera);
	bindMatrices(shader, mesh, context.camera);
	return true;
}

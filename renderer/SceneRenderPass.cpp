#include "SceneRenderPass.h"

#include <iostream>

#include "mesh/instancedMesh.h"
#include "renderer/MaterialBinder.h"
#include "renderer/RenderState.h"

using namespace GLframework;

void SceneRenderPass::render(
	const std::vector<std::shared_ptr<Mesh>>& opacityObjects,
	const std::vector<std::shared_ptr<Mesh>>& transparentObjects,
	Camera* camera,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::shared_ptr<SpotLight>& spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const std::shared_ptr<AmbientLight>& ambient,
	const std::shared_ptr<Material>& globalMaterial,
	const ShaderLibrary& shaderLibrary
) const
{
	for (const auto& mesh : opacityObjects)
	{
		renderObject(mesh, camera, dirLight, spotLight, pointLights, ambient, globalMaterial, shaderLibrary);
	}

	for (const auto& mesh : transparentObjects)
	{
		renderObject(mesh, camera, dirLight, spotLight, pointLights, ambient, globalMaterial, shaderLibrary);
	}
}

void SceneRenderPass::renderObject(
	const std::shared_ptr<Mesh>& mesh,
	Camera* camera,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::shared_ptr<SpotLight>& spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const std::shared_ptr<AmbientLight>& ambient,
	const std::shared_ptr<Material>& globalMaterial,
	const ShaderLibrary& shaderLibrary
) const
{
	const std::shared_ptr<Material> material = globalMaterial != nullptr ? globalMaterial : mesh->getMaterial();
	if (material == nullptr)
	{
		std::cout << "SceneRenderPass: missing material\n";
		return;
	}

	RenderState::applyMaterialState(*material);
	auto shader = shaderLibrary.get(material->getMaterialType());
	if (shader == nullptr)
	{
		return;
	}

	shader->begin();
	if (!MaterialBinder::bind(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient))
	{
		std::cout << "SceneRenderPass: unsupported material\n";
	}

	drawMesh(mesh);
	shader->end();
}

void SceneRenderPass::drawMesh(const std::shared_ptr<Mesh>& mesh) const
{
	auto geometry = mesh->getGeometry();
	glBindVertexArray(geometry->getVao());

	if (mesh->getType() == ObjectType::InstancedMesh)
	{
		std::shared_ptr<InstancedMesh> instancedMesh = std::static_pointer_cast<InstancedMesh>(mesh);
		glDrawElementsInstanced(
			GL_TRIANGLES,
			geometry->getIndicesCount(),
			GL_UNSIGNED_INT,
			nullptr,
			instancedMesh->getInstanceCount()
		);
	}
	else
	{
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
	}

	glBindVertexArray(0);
}

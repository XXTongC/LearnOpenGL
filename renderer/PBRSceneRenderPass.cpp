#include "PBRSceneRenderPass.h"

#include <iostream>

#include "materials/material.h"
#include "mesh/instancedMesh.h"
#include "renderer/MaterialBinder.h"
#include "renderer/RenderState.h"

using namespace GLframework;

int PBRSceneRenderPass::render(
	const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
	const std::vector<std::shared_ptr<Mesh>>& pbrTransparentObjects,
	Camera* camera,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::shared_ptr<SpotLight>& spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const std::shared_ptr<AmbientLight>& ambient,
	const ShaderLibrary& shaderLibrary,
	const EnvironmentRenderTargets* environmentTargets
) const
{
	int drawCalls = 0;
	for (const auto& mesh : pbrOpacityObjects)
	{
		if (renderObject(mesh, camera, dirLight, spotLight, pointLights, ambient, shaderLibrary, environmentTargets))
		{
			++drawCalls;
		}
	}

	for (const auto& mesh : pbrTransparentObjects)
	{
		if (renderObject(mesh, camera, dirLight, spotLight, pointLights, ambient, shaderLibrary, environmentTargets))
		{
			++drawCalls;
		}
	}

	return drawCalls;
}

bool PBRSceneRenderPass::renderObject(
	const std::shared_ptr<Mesh>& mesh,
	Camera* camera,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::shared_ptr<SpotLight>& spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const std::shared_ptr<AmbientLight>& ambient,
	const ShaderLibrary& shaderLibrary,
	const EnvironmentRenderTargets* environmentTargets
) const
{
	const auto material = mesh ? mesh->getMaterial() : nullptr;
	if (!material || material->getMaterialType() != MaterialType::PBRMaterial)
	{
		return false;
	}

	RenderState::applyMaterialState(*material);
	auto shader = shaderLibrary.get(MaterialType::PBRMaterial);
	if (shader == nullptr)
	{
		return false;
	}

	shader->begin();
	if (!MaterialBinder::bind(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient, environmentTargets))
	{
		std::cout << "PBRSceneRenderPass: unsupported PBR material\n";
	}

	drawMesh(mesh);
	shader->end();
	return true;
}

void PBRSceneRenderPass::drawMesh(const std::shared_ptr<Mesh>& mesh) const
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

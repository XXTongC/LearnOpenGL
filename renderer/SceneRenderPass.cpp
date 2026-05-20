#include "SceneRenderPass.h"

#include <iostream>

#include "renderer/MaterialBinder.h"
#include "renderer/MeshDraw.h"
#include "renderer/RenderState.h"

using namespace GLframework;

int SceneRenderPass::render(
	const std::vector<std::shared_ptr<Mesh>>& opacityObjects,
	const std::vector<std::shared_ptr<Mesh>>& transparentObjects,
	Camera* camera,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::shared_ptr<SpotLight>& spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const std::shared_ptr<AmbientLight>& ambient,
	const std::shared_ptr<Material>& globalMaterial,
	const ShaderLibrary& shaderLibrary,
	const EnvironmentRenderTargets* environmentTargets
) const
{
	int drawCalls = 0;
	for (const auto& mesh : opacityObjects)
	{
		if (renderObject(mesh, camera, dirLight, spotLight, pointLights, ambient, globalMaterial, shaderLibrary, environmentTargets))
		{
			++drawCalls;
		}
	}

	for (const auto& mesh : transparentObjects)
	{
		if (renderObject(mesh, camera, dirLight, spotLight, pointLights, ambient, globalMaterial, shaderLibrary, environmentTargets))
		{
			++drawCalls;
		}
	}

	return drawCalls;
}

bool SceneRenderPass::renderObject(
	const std::shared_ptr<Mesh>& mesh,
	Camera* camera,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::shared_ptr<SpotLight>& spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const std::shared_ptr<AmbientLight>& ambient,
	const std::shared_ptr<Material>& globalMaterial,
	const ShaderLibrary& shaderLibrary,
	const EnvironmentRenderTargets* environmentTargets
) const
{
	const std::shared_ptr<Material> material = globalMaterial != nullptr ? globalMaterial : mesh->getMaterial();
	if (material == nullptr)
	{
		std::cout << "SceneRenderPass: missing material\n";
		return false;
	}

	RenderState::applyMaterialState(*material);
	auto shader = shaderLibrary.get(material->getMaterialType());
	if (shader == nullptr)
	{
		return false;
	}

	shader->begin();
	if (!MaterialBinder::bind(shader, material, mesh, camera, dirLight, spotLight, pointLights, ambient, environmentTargets))
	{
		std::cout << "SceneRenderPass: unsupported material\n";
	}

	const bool drawn = MeshDraw::drawIndexed(mesh);
	shader->end();
	return drawn;
}

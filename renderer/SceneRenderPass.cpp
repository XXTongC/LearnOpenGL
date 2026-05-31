#include "SceneRenderPass.h"

#include <iostream>

#include "framework/shader.h"
#include "materials/material.h"
#include "mesh/mesh.h"
#include "renderer/MaterialBinder.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/MeshDraw.h"
#include "renderer/RenderState.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

int SceneRenderPass::render(
	const std::vector<std::shared_ptr<Mesh>>& opacityObjects,
	const std::vector<std::shared_ptr<Mesh>>& transparentObjects,
	const std::shared_ptr<Material>& globalMaterial,
	const ShaderLibrary& shaderLibrary,
	const MaterialBindingContext& bindingContext
) const
{
	int drawCalls = 0;
	for (const auto& mesh : opacityObjects)
	{
		if (renderObject(mesh, globalMaterial, shaderLibrary, bindingContext))
		{
			++drawCalls;
		}
	}

	for (const auto& mesh : transparentObjects)
	{
		if (renderObject(mesh, globalMaterial, shaderLibrary, bindingContext))
		{
			++drawCalls;
		}
	}

	return drawCalls;
}

bool SceneRenderPass::renderObject(
	const std::shared_ptr<Mesh>& mesh,
	const std::shared_ptr<Material>& globalMaterial,
	const ShaderLibrary& shaderLibrary,
	const MaterialBindingContext& bindingContext
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
	if (!MaterialBinder::bind(shader, material, mesh, bindingContext))
	{
		std::cout << "SceneRenderPass: unsupported material\n";
	}

	const bool drawn = MeshDraw::drawIndexed(mesh);
	shader->end();
	return drawn;
}

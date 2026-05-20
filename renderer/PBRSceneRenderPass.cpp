#include "PBRSceneRenderPass.h"

#include <iostream>

#include "materials/material.h"
#include "renderer/MaterialBinder.h"
#include "renderer/MeshDraw.h"
#include "renderer/RenderState.h"

using namespace GLframework;

int PBRSceneRenderPass::render(
	const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
	const std::vector<std::shared_ptr<Mesh>>& pbrTransparentObjects,
	const ShaderLibrary& shaderLibrary,
	const MaterialBindingContext& bindingContext
) const
{
	int drawCalls = 0;
	for (const auto& mesh : pbrOpacityObjects)
	{
		if (renderObject(mesh, shaderLibrary, bindingContext))
		{
			++drawCalls;
		}
	}

	for (const auto& mesh : pbrTransparentObjects)
	{
		if (renderObject(mesh, shaderLibrary, bindingContext))
		{
			++drawCalls;
		}
	}

	return drawCalls;
}

bool PBRSceneRenderPass::renderObject(
	const std::shared_ptr<Mesh>& mesh,
	const ShaderLibrary& shaderLibrary,
	const MaterialBindingContext& bindingContext
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
	if (!MaterialBinder::bind(shader, material, mesh, bindingContext))
	{
		std::cout << "PBRSceneRenderPass: unsupported PBR material\n";
	}

	const bool drawn = MeshDraw::drawIndexed(mesh);
	shader->end();
	return drawn;
}

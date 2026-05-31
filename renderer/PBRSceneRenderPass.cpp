#include "PBRSceneRenderPass.h"

#include <iostream>

#include "framework/shader.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "mesh/mesh.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/MeshDraw.h"
#include "renderer/PBRMaterialBinder.h"
#include "renderer/RenderState.h"
#include "renderer/ShaderLibrary.h"

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
	if (!PBRMaterialBinder::bind(
		shader,
		std::static_pointer_cast<PBRMaterial>(material),
		mesh,
		bindingContext
	))
	{
		std::cout << "PBRSceneRenderPass: PBR material binding failed\n";
	}

	const bool drawn = MeshDraw::drawIndexed(mesh);
	shader->end();
	return drawn;
}

#include "PBRDepthPrepass.h"

#include "framework/shader.h"
#include "materials/material.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "mesh/mesh.h"
#include "renderer/DepthPrepassBinder.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/MeshDraw.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

int PBRDepthPrepass::render(
	const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
	const MaterialBindingContext& bindingContext,
	const ShaderLibrary& shaderLibrary
) const
{
	if (bindingContext.camera == nullptr || pbrOpacityObjects.empty())
	{
		return 0;
	}

	auto shader = shaderLibrary.get(MaterialType::DepthMaterial);
	if (shader == nullptr)
	{
		return 0;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	int drawCalls = 0;
	shader->begin();
	if (!DepthPrepassBinder::bindFrame(shader, bindingContext))
	{
		shader->end();
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		return 0;
	}

	for (const auto& mesh : pbrOpacityObjects)
	{
		if (renderObject(mesh, shader))
		{
			++drawCalls;
		}
	}
	shader->end();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	return drawCalls;
}

bool PBRDepthPrepass::renderObject(
	const std::shared_ptr<Mesh>& mesh,
	const std::shared_ptr<Shader>& shader
) const
{
	const auto material = mesh ? mesh->getMaterial() : nullptr;
	if (!material || material->getMaterialType() != MaterialType::PBRMaterial)
	{
		return false;
	}

	const auto pbrMaterial = std::static_pointer_cast<PBRMaterial>(material);
	if (pbrMaterial->mUseAlphaMask)
	{
		return false;
	}

	if (!DepthPrepassBinder::bindObject(shader, mesh))
	{
		return false;
	}

	return MeshDraw::drawIndexed(mesh);
}

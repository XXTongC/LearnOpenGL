#include "PBRDepthPrepass.h"

#include "materials/material.h"
#include "renderer/MeshDraw.h"

using namespace GLframework;

int PBRDepthPrepass::render(
	const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
	Camera* camera,
	const ShaderLibrary& shaderLibrary
) const
{
	if (camera == nullptr || pbrOpacityObjects.empty())
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
	shader->setMat4("viewMatrix", camera->getViewMatrix());
	shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
	shader->setFloat("near", camera->mNear);
	shader->setFloat("far", camera->mFar);
	for (const auto& mesh : pbrOpacityObjects)
	{
		if (renderObject(mesh, camera, shader))
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
	Camera*,
	const std::shared_ptr<Shader>& shader
) const
{
	const auto material = mesh ? mesh->getMaterial() : nullptr;
	if (!material || material->getMaterialType() != MaterialType::PBRMaterial)
	{
		return false;
	}

	shader->setMat4("modelMatrix", mesh->getModelMatrix());
	return MeshDraw::drawIndexed(mesh);
}

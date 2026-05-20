#include "DepthPrepassBinder.h"

#include "camera/camera.h"

using namespace GLframework;

bool DepthPrepassBinder::bindFrame(const std::shared_ptr<Shader>& shader, const MaterialBindingContext& context)
{
	if (!shader || context.camera == nullptr)
	{
		return false;
	}

	shader->setMat4("viewMatrix", context.camera->getViewMatrix());
	shader->setMat4("projectionMatrix", context.camera->getProjectionMatrix());
	shader->setFloat("near", context.camera->mNear);
	shader->setFloat("far", context.camera->mFar);
	return true;
}

bool DepthPrepassBinder::bindObject(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh)
{
	if (!shader || !mesh)
	{
		return false;
	}

	shader->setMat4("modelMatrix", mesh->getModelMatrix());
	return true;
}

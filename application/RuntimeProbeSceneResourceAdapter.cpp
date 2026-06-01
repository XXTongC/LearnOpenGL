#include "RuntimeProbeSceneResourceAdapter.h"

#include "../framework/geometry.h"
#include "../framework/scene.h"
#include "../materials/MaterialTypes.h"
#include "../renderer/renderer.h"
#include "RuntimeRenderResourceState.h"

bool GL_RUNTIME::RuntimeProbeSceneResourceAdapter::hasOffScreenSceneAndRenderer(
	const RuntimeRenderResourceState& renderResources
)
{
	return renderResources.sceneOffScreen() != nullptr
		&& renderResources.renderer() != nullptr;
}

std::shared_ptr<GLframework::Geometry> GL_RUNTIME::RuntimeProbeSceneResourceAdapter::createPbrSphereGeometry(
	const RuntimeRenderResourceState& renderResources,
	float radius,
	int sectorCount,
	int stackCount
)
{
	if (!renderResources.renderer())
	{
		return nullptr;
	}

	return GLframework::Geometry::createSphere(
		renderResources.renderer()->getShader(GLframework::MaterialType::PBRMaterial),
		radius,
		sectorCount,
		stackCount
	);
}

std::shared_ptr<GLframework::Geometry> GL_RUNTIME::RuntimeProbeSceneResourceAdapter::createPbrPlaneGeometry(
	const RuntimeRenderResourceState& renderResources,
	float width,
	float height
)
{
	if (!renderResources.renderer())
	{
		return nullptr;
	}

	return GLframework::Geometry::createPlane(
		renderResources.renderer()->getShader(GLframework::MaterialType::PBRMaterial),
		width,
		height
	);
}

void GL_RUNTIME::RuntimeProbeSceneResourceAdapter::addOffScreenSceneChild(
	const RuntimeRenderResourceState& renderResources,
	const std::shared_ptr<GLframework::Object>& object
)
{
	if (!renderResources.sceneOffScreen() || !object)
	{
		return;
	}

	renderResources.sceneOffScreen()->addChild(object);
}

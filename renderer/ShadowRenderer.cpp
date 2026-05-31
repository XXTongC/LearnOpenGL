#include "ShadowRenderer.h"

#include "camera/camera.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "mesh/mesh.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

ShadowRenderStats ShadowRenderer::render(
	Camera* camera,
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	ShaderLibrary& shaderLibrary
)
{
	ShadowRenderStats stats{};
	const auto directionalStats = mDirectionalPass.render(camera, meshes, dirLight, shaderLibrary);
	const auto pointStats = mPointPass.render(meshes, pointLights, shaderLibrary);

	stats.directionalLayerCount = directionalStats.directionalLayerCount;
	stats.directionalDrawCalls = directionalStats.directionalDrawCalls;
	stats.directionalAlphaMaskedDrawCalls = directionalStats.directionalAlphaMaskedDrawCalls;
	stats.pointLightCount = pointStats.pointLightCount;
	stats.pointFaceCount = pointStats.pointFaceCount;
	stats.pointDrawCalls = pointStats.pointDrawCalls;
	stats.pointAlphaMaskedDrawCalls = pointStats.pointAlphaMaskedDrawCalls;
	return stats;
}

#include "PBRMaterialBinder.h"

#include "renderer/LightResourceBinder.h"
#include "renderer/PBRIBLResourceBinder.h"
#include "renderer/PBRObjectUniformBinder.h"
#include "renderer/PBRShadowResourceBinder.h"
#include "renderer/PBRSurfaceResourceBinder.h"

using namespace GLframework;

bool PBRMaterialBinder::bind(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<PBRMaterial>& material,
	const std::shared_ptr<Mesh>& mesh,
	const MaterialBindingContext& context
)
{
	if (!shader || !material || !mesh || context.camera == nullptr)
	{
		return false;
	}

	PBRObjectUniformBinder::bind(shader, material, mesh, context);
	LightResourceBinder::bindForwardLights(shader, context.dirLight, context.spotLight, context.getPointLights(), context.ambient);
	PBRShadowResourceBinder::bind(shader, context);
	PBRSurfaceResourceBinder::bind(shader, material);
	PBRIBLResourceBinder::bind(shader, material, context.environmentTargets);
	return true;
}

#include "depthMaterial.h"
using namespace GLframework;

DepthMaterial::DepthMaterial():Material()
{
	setMaterialType(MaterialType::DepthMaterial);
}

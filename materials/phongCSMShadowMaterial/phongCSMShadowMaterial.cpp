#include "phongCSMShadowMaterial.h"

using namespace GLframework;

PhongSurfaceEditControls PhongCSMShadowMaterial::surfaceEditControls()
{
	return PhongSurfaceEditControls{
		&mShiness,
		&mDiffuse,
		&mSpecularMask
	};
}

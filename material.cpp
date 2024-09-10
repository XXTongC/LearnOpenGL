#include "material.h"

using namespace GLframework;

MaterialType Material::getMaterialType() const
{
	return mType;
}


void Material::setMaterialType(MaterialType type)
{
	mType = type;
}



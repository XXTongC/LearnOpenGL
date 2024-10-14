#include "material.h"

using namespace GLframework;

float Material::getFactor() const
{
	return mFactor;
}

float Material::getUnit() const
{
	return mUnit;
}

void Material::setFactor(float value)
{
	mFactor = value;
}

void Material::setUnit(float value)
{
	mUnit = value;
}

bool Material::getPolygonOffsetState() const
{
	return mPolygonOffsetState;
}

unsigned Material::getPolygonOffsetType() const
{
	return mPolygonOffsetType;
}

void Material::setPolygonOffsetState(bool value)
{
	mPolygonOffsetState = value;
}

void Material::setPolygonOffsetType(unsigned PolygonOffSetType)
{
	mPolygonOffsetType = PolygonOffSetType;
}


MaterialType Material::getMaterialType() const
{
	return mType;
}

GLenum Material::getDepthFunc() const
{
	return mDepthFunc;
}

bool Material::getDepthTest() const
{
	return mDepthTest;
}

bool Material::getDepthWrite() const
{
	return mDepthWrite;
}

void Material::setDepthFunc(GLenum value)
{
	mDepthFunc = value;
}

void Material::setDepthTest(bool value)
{
	mDepthTest = value;
}

void Material::setDepthWrite(bool value)
{
	mDepthWrite = value;
}

void Material::setMaterialType(MaterialType type)
{
	mType = type;
}



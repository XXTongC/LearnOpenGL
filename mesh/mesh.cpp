#include "mesh.h"

using namespace GLframework;



void Mesh::setGeometry(std::shared_ptr<Geometry> geometry)
{
	mGeometry = std::move(geometry);
}

void Mesh::setMaterial(std::shared_ptr<Material> material)
{
	mMaterial = material;
}

std::shared_ptr<Geometry> Mesh::getGeometry() const
{
	return mGeometry;
}

std::shared_ptr<Material> Mesh::getMaterial() const
{
	return mMaterial;
}

#pragma once
#include "object.h"
#include "geometry.h"
#include "material.h"

namespace GLframework
{
	class Mesh:public Object
	{
	public:
		Mesh(Geometry* geometry,Material* material)
			:mGeometry(geometry),mMaterial(material)
		{}
		~Mesh();
		void setGeometry(Geometry* geometry);
		void setMaterial(Material* material);
	private:
		Geometry* mGeometry{ nullptr };
		Material* mMaterial{ nullptr };

	};


}
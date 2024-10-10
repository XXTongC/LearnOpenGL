#pragma once
#include "object.h"
#include "geometry.h"
#include "material.h"

namespace GLframework
{
	class Mesh:public Object
	{
	public:
		Mesh(std::shared_ptr<Geometry> geometry,std::shared_ptr<Material> material)
			:mGeometry(geometry),mMaterial(material)
		{
			this->setType(ObjectType::Mesh);
		}
		~Mesh(){}
		void setGeometry(std::shared_ptr<Geometry> geometry);
		void setMaterial(std::shared_ptr<Material> material);
		std::shared_ptr<Geometry> getGeometry() const;
		std::shared_ptr<Material> getMaterial() const;
	private:
		std::shared_ptr<Geometry> mGeometry{ nullptr };
		std::shared_ptr<Material> mMaterial{ nullptr };

	};


}
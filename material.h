#pragma once
#include "core.h"

namespace GLframework
{
	enum class MaterialType
	{
		PhongMaterial,
		WhiteMaterial,
	};

	class Material
	{
	public:
		Material(){}
		~Material(){}
		MaterialType getMaterialType() const;
		void setMaterialType(MaterialType type);

	private:
		MaterialType mType;
	};

}

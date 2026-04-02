#pragma once
#include "core.h"
#include "material.h"

namespace GLframework
{
	class WhiteMaterial: public  Material
	{
	public:
		WhiteMaterial()
			:Material()
		{
			setMaterialType(MaterialType::WhiteMaterial);
		}
		~WhiteMaterial(){}

	};
}


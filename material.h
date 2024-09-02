#pragma once
#include "core.h"

namespace GLframework
{
	enum class MaterialType
	{
		PhongMaterial
	};

	class Material
	{
	public:
		Material();
		~Material();

	public:
		MaterialType mType;
	};

}

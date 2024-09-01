#pragma once
#include "core.h"

namespace GLframework
{
	enum class MaterialType
	{
		PhongMaterial
	};

	class Materal
	{
	public:
		Materal();
		~Materal();

	public:
		MaterialType mType;
	};

}

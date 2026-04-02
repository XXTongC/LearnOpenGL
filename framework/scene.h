#pragma once
#include "core.h"
#include "object.h"
namespace GLframework
{
	class Scene : public Object
	{
	public:
		Scene()
		{
			setType(ObjectType::Scene);
		}
		~Scene(){}
	};

}

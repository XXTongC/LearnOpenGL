#pragma once
#include "core.h"

namespace GLframework
{
	class Tools
	{
	public:
		//������󣬽⹹����λ�á���ת��ϢXYZ��������Ϣ
		static void decompose(glm::mat4 matrix,glm::vec3& position,glm::vec3& eulerAngle,glm::vec3& scale);
	};
}

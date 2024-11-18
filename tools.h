#pragma once
#include "core.h"
#include "object.h"

namespace GLframework
{
	class Tools
	{
	public:
		//������󣬽⹹����λ�á���ת��ϢXYZ��������Ϣ
		static void decompose(glm::mat4 matrix,glm::vec3& position,glm::vec3& eulerAngle,glm::vec3& scale);
		//���ڸ�һ�������ģ�͸���ͳһ��Blend����
		static void setModelBlend(std::shared_ptr<Object> obj, bool blend, float opacity);
	};
}

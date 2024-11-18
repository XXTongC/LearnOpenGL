#pragma once
#include "core.h"
#include "object.h"

namespace GLframework
{
	class Tools
	{
	public:
		//传入矩阵，解构其中位置、旋转信息XYZ、缩放信息
		static void decompose(glm::mat4 matrix,glm::vec3& position,glm::vec3& eulerAngle,glm::vec3& scale);
		//用于给一个读入的模型赋予统一的Blend设置
		static void setModelBlend(std::shared_ptr<Object> obj, bool blend, float opacity);
	};
}

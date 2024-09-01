#pragma once
#include "core.h"

namespace GLframework
{
	class Object
	{
	public:
		Object(){}
		Object(glm::vec3 position = glm::vec3{0.0f}, float anglex= 0.0f, float angley = 0.0f, float anglez = 0.0f, glm::vec3 scale =glm::vec3{1.0f})
			:mPosition(position),mAngleX(anglex),mAngleY(angley),mAngleZ(anglez),mScale(scale)
		{}
		~Object();

		void setPosition(glm::vec3 pos);

		//ÔöÁ¿Ðý×ª
		void rotateX(float angle);
		void rotateY(float angle);
		void rotateZ(float angle);

		void setScale(glm::vec3 scale);

		glm::mat4 getModleMatrix();

	protected:
		glm::vec3 mPosition{ 0.0f };
		float mAngleX{ 0.0f };
		float mAngleY{ 0.0f };
		float mAngleZ{ 0.0f };

		glm::vec3 mScale{ 1.0f };

	};
}
#pragma once
#include "core.h"

class Camera
{
public:
	Camera(glm::vec3 position = { 0.0f, 0.0f, 1.0f }, glm::vec3 up= { 0.0f,1.0f,0.0f }, glm::vec3 right= { 1.0f,0.0f,0.0f })
		:mPosition(position),mUp(up),mRight(right)
	{
		front = glm::cross(mUp, mRight);
		center = mPosition + front;
	}
	~Camera();

	glm::mat4 getViewMatrix() const;
	virtual glm::mat4 getProjectionMatrix() const;

public:
	glm::vec3 mPosition;
	glm::vec3 mUp;
	glm::vec3 mRight;
	glm::vec3 front;
	glm::vec3 center;

};

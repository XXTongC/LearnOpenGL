#include "object.h"

using namespace GLframework;

Object::~Object()
{
	
}

glm::mat4 Object::getModleMatrix()
{
	//Ë³Ðò£ºËõ·Å -> Ðý×ª -> Æ½ÒÆ
	glm::mat4 transform{ 1.0f };

	transform = glm::scale(transform, mScale);

	//Ðý×ªË³Ðò£ºpitch yaw roll
	transform = glm::rotate(transform, glm::radians(mAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(mAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(mAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));

	transform = glm::translate(glm::mat4(1.0f),mPosition) * transform;
	return transform;
}

void Object::rotateX(float angle)
{
	mAngleX += angle;
}

void Object::rotateY(float angle)
{
	mAngleY += angle;
}

void Object::rotateZ(float angle)
{
	mAngleY += angle;
}

void Object::setPosition(glm::vec3 pos)
{
	mPosition = pos;
}

void Object::setScale(glm::vec3 scale)
{
	mScale = scale;
}







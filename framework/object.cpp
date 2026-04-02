#include "object.h"

using namespace GLframework;

glm::vec3 Object::getDirection() const
{
	auto modelMatrix = glm::mat3(this->getModelMatrix());
	glm::vec3 direction = glm::normalize(-modelMatrix[2]);
	return direction;
}

float Object::getAngleX() const
{
	return mAngleX;
}

float Object::getAngleY() const
{
	return mAngleY;
}

float Object::getAngleZ() const
{
	return mAngleZ;
}

glm::vec3 Object::getPosition() const
{
	return mPosition;
}

glm::mat4 Object::getModelMatrix() const
{
	glm::mat4 parentModleMatrix{ 1.0f };
	if(mParent!=nullptr)
	{
		parentModleMatrix = mParent->getModelMatrix();
	}

	//˳������ -> ��ת -> ƽ��
	glm::mat4 transform{ 1.0f };

	transform = glm::scale(transform, mScale);

	//��ת˳��pitch yaw roll
	transform = glm::rotate(transform, glm::radians(mAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(mAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(mAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));

	transform = parentModleMatrix * glm::translate(glm::mat4(1.0f),mPosition) * transform;
	return transform;
}

glm::vec3 Object::getScale() const
{
	return mScale;
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
	mAngleZ += angle;
}

void Object::setPosition(glm::vec3 pos)
{
	mPosition = pos;
}

void Object::setScale(glm::vec3 scale)
{
	mScale = scale;
}

void Object::setName(const std::string& name)
{
	mName = name;
}

std::string Object::getName() const
{
	return mName;
}

void Object::addChild(std::shared_ptr<Object> obj)
{
	// 1. ����Ƿ�����������������
	if(std::find(mChildren.begin(), mChildren.end(), obj) !=mChildren.end())
	{
		std::cout << "obj has been added in this Object. \n";
		return;
	}

	// 2. ���뺢��
	mChildren.push_back(obj);
	// 3. �����¼���ĺ������ĸ�����˭
	obj->mParent = this->getShared();
}

std::vector<std::shared_ptr<Object>> Object::getChildren()
{
	return mChildren;
}

std::shared_ptr<Object> Object::getParent()
{
	return mParent;
}

void Object::addParent(std::shared_ptr<Object> obj)
{
	mParent = obj;
}

ObjectType Object::getType() const
{
	return mType;
}

void Object::setType(ObjectType type)
{
	mType = type;
}

void Object::setAngleX(float angle)
{
	mAngleX = angle;
}

void Object::setAngleY(float angle)
{
	mAngleY = angle;
}

void Object::setAngleZ(float angle)
{
	mAngleZ = angle;
}


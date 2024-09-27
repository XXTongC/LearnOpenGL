#include "object.h"

using namespace GLframework;

glm::vec3 Object::getPosition() const
{
	return mPosition;
}

glm::mat4 Object::getModleMatrix()
{
	glm::mat4 parentModleMatrix{ 1.0f };
	if(mParent!=nullptr)
	{
		parentModleMatrix = mParent->getModleMatrix();
	}

	//顺序：缩放 -> 旋转 -> 平移
	glm::mat4 transform{ 1.0f };

	transform = glm::scale(transform, mScale);

	//旋转顺序：pitch yaw roll
	transform = glm::rotate(transform, glm::radians(mAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(mAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::rotate(transform, glm::radians(mAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));

	transform = parentModleMatrix * glm::translate(glm::mat4(1.0f),mPosition) * transform;
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

void Object::addChild(std::shared_ptr<Object> obj)
{
	// 1. 检查是否曾经加入过这个孩子
	if(std::find(mChildren.begin(), mChildren.end(), obj) !=mChildren.end())
	{
		std::cout << "obj has been added in this Object. \n";
		return;
	}

	// 2. 加入孩子
	mChildren.push_back(obj);
	// 3. 告诉新加入的孩子他的父亲是谁
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






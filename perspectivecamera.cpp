#include "perspectivecamera.h"

PerspectiveCamera::~PerspectiveCamera()
{
	
}

void PerspectiveCamera::setFovy(float value)
{
	mFovy = value;
}

void PerspectiveCamera::setAspect(float value)
{
	//mAspect = value;
}

void PerspectiveCamera::scale(float deltaScale)
{
	auto front = glm::cross(mUp, mRight);
	mPosition += front * deltaScale;
}


glm::mat4 PerspectiveCamera::getProjectionMatrix() const
{
	//ע��mFovy�ǽǶȣ���Ҫת��Ϊ������
	return glm::perspective(glm::radians(mFovy), mAspect, mNear, mFar);
}



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

float PerspectiveCamera::getAspect() const
{
	return mAspect;
}

float PerspectiveCamera::getFovy() const
{
	return mFovy;
}


glm::mat4 PerspectiveCamera::getProjectionMatrix() const
{
	//注意mFovy是角度，需要转换为弧度制
	return glm::perspective(glm::radians(mFovy), mAspect, mNear, mFar);
}



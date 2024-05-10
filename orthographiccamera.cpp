#include "orthographiccamera.h"

OrthographicCamera::~OrthographicCamera()
{
	
}

glm::mat4 OrthographicCamera::getProjectionMatrix() const
{
	return glm::ortho(mLeft, mRight, mBottom, mTop, mNear, mFar);
}

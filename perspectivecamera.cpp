#include "perspectivecamera.h"

PerspectiveCamera::~PerspectiveCamera()
{
	
}

glm::mat4 PerspectiveCamera::getProjectionMatrix() const
{
	//ע��mFovy�ǽǶȣ���Ҫת��Ϊ������
	return glm::perspective(glm::radians(mFovy), mAspect, mNear, mFar);
}



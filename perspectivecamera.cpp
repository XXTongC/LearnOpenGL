#include "perspectivecamera.h"

PerspectiveCamera::~PerspectiveCamera()
{
	
}

glm::mat4 PerspectiveCamera::getProjectionMatrix() const
{
	//注意mFovy是角度，需要转换为弧度制
	return glm::perspective(glm::radians(mFovy), mAspect, mNear, mFar);
}



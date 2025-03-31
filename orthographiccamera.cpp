#include "orthographiccamera.h"
#include <cmath>
OrthographicCamera::~OrthographicCamera()
{
	
}

void OrthographicCamera::scale(float deltaScale)
{
	mScale -= deltaScale;
} 

glm::mat4 OrthographicCamera::getProjectionMatrix() const
{
	const auto scale = static_cast<float>(std::pow(2.0, mScale));
	return glm::ortho(mL * scale, mR * scale, mB * scale, mT * scale, mNear, mFar);
}

#pragma once
#include "camera.h"

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(glm::vec3 position = { 0.0f, 0.0f, 1.0f }, glm::vec3 up = { 0.0f,1.0f,0.0f }, glm::vec3 right = { 1.0f,0.0f,0.0f },
		float mfovy = 0.0f, float maspect = 0.0f, float mnear=0.0f, float mfar=0.0f)
		:Camera(position, up, right),mFovy(mfovy),mAspect(maspect), mNear(mnear), mFar(mfar)
	{}
	~PerspectiveCamera();
	glm::mat4 getProjectionMatrix() const override;
public:
	float mFovy;
	float mAspect;
	float mNear;
	float mFar;

};

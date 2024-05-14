#pragma once
#include "camera.h"

class OrthographicCamera : public Camera
{
public:
	OrthographicCamera(glm::vec3 position = { 0.0f, 0.0f, 5.0f }, glm::vec3 up = { 0.0f,1.0f,0.0f }, glm::vec3 right = { 1.0f,0.0f,0.0f },
		float mleft = 0.0f, float mright = 0.0f, float mtop=0.0f, float mbottom=0.0f, float mnear=0.0f, float mfar=0.0f)
		:Camera(position, up, right),mLeft(mleft),mRight(mright),mTop(mtop),mBottom(mbottom),mNear(mnear),mFar(mfar)
	{}
	OrthographicCamera(float mleft = 0.0f, float mright = 0.0f, float mtop = 0.0f, float mbottom = 0.0f, float mnear = 0.0f, float mfar = 0.0f)
		:Camera(), mLeft(mleft), mRight(mright), mTop(mtop), mBottom(mbottom), mNear(mnear), mFar(mfar)
	{}
	~OrthographicCamera();
	glm::mat4 getProjectionMatrix() const override;
	void scale(float deltaScale) override;

public:
	float mLeft;
	float mRight;
	float mTop;
	float mBottom;
	float mNear;
	float mFar;
	float mScale{ 0.0f };	//¼ÇÂ¼µ±Ç°scale

};

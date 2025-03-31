#pragma once
#include "camera.h"

class OrthographicCamera : public Camera
{
public:
	OrthographicCamera(glm::vec3 position = { 0.0f, 0.0f, 5.0f }, glm::vec3 up = { 0.0f,1.0f,0.0f }, glm::vec3 right = { 1.0f,0.0f,0.0f }
		,float mleft = 0.0f, float mright = 0.0f, float mtop=0.0f, float mbottom=0.0f, float mnear = 0.0f, float mfar = 0.0f)
		:Camera(position, up, right),mL(mleft),mR(mright),mT(mtop),mB(mbottom)
	{
		mNear = mnear;
		mFar = mfar;
	}
	OrthographicCamera(float mleft = 0.0f, float mright = 0.0f, float mtop = 0.0f, float mbottom = 0.0f,  float mnear = 0.0f, float mfar = 0.0f)
		:Camera(), mL(mleft), mR(mright), mT(mtop), mB(mbottom)
	{
		mNear = mnear;
		mFar = mfar;
	}
	~OrthographicCamera() override;
	glm::mat4 getProjectionMatrix() const override;
	void scale(float deltaScale) override;

public:
	float mL;
	float mR;
	float mT;
	float mB;

	float mScale{ 0.0f };	//¼ÇÂ¼µ±Ç°scale

};

#pragma once
#include "cameracontrol.h"

class TrackBallCameraControl:public CameraControl
{
public:
	TrackBallCameraControl(bool l = false, bool r = false, bool m = false, float x = 0.0f, float y = 0.0f, float sensitivity = 0.2f,float movespeed = 0.005f)
		:CameraControl(l,r,m,x,y,sensitivity),mMoveSpeed(movespeed)
	{}

	~TrackBallCameraControl();

	//��һ������Ƿ�����Ҫ��д�ĸ��ຯ��
	void onCursor(double xpos, double ypos) override;
	void onKey(int key, int action, int mods) override;
	void onScroll(float offset) override;
private:
	void pitch(float angle);
	void yaw(float angle);
private:
	float mMoveSpeed;
};
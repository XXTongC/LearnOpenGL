#pragma once
#include "cameracontrol.h"

class TrackBallCameraControl:public CameraControl
{
public:
	TrackBallCameraControl(bool l = false, bool r = false, bool m = false, float x = 0.0f, float y = 0.0f, float sensitivity = 0.2f,float movespeed = 0.005f)
		:CameraControl(l,r,m,x,y,sensitivity),mMoveSpeed(movespeed)
	{}

	~TrackBallCameraControl();

	//第一步检查是否有需要重写的父类函数
	void onCursor(double xpos, double ypos) override;
	void onKey(int key, int action, int mods) override;
	void onScroll(float offset) override;
private:
	void pitch(float angle);
	void yaw(float angle);
private:
	float mMoveSpeed;
};
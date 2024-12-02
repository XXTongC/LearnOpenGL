#pragma once
#include "cameracontrol.h"
#include "core.h"

class GameCameraControl : public CameraControl
{
public:
	GameCameraControl(bool l = false, bool r = false, bool m = false, float x = 0.0f, float y = 0.0f, float sensitivity = 0.2f)
		:CameraControl(l, r, m, x, y, sensitivity)
	{}
	~GameCameraControl();
	void update() override;
	void onCursor(double xpos, double ypos) override;
	void setMoveSpeed(float speed) { mMoveSpeed = speed; }
private:
	void pitch(float angle);
	void yaw(float angle);
private:
	float mPicth{ 0.0f };
	float mMoveSpeed{ 0.01f };
};
#pragma once
#include <map>
#include "camera.h"
class CameraControl
{
protected:
	//鼠标状态
	bool mLeftMouseDown;
	bool mRightMouseDown;
	bool mMiddleMouseDown;
	bool mMiddleMouseRelease;
	//鼠标当前位置
	float mCurrentX;
	float mCurrentY;
	//键盘相关按下与否状态
	std::map<int, bool> mKeyMap;
	//敏感度
	float mSensitivity;
	//当前控制的摄像机
	Camera* mCamera = nullptr;
	//记录相机缩放速度
	float mScaleSpeed = 0.2f;
public:
	CameraControl(bool l = false, bool r = false, bool m = false, float x = 0.0f, float y=0.0f, float sensitivity = 0.2f)
		:mLeftMouseDown(l), mRightMouseDown(r), mMiddleMouseDown(m), mCurrentX(x), mCurrentY(y), mSensitivity(sensitivity)
	{
		mMiddleMouseRelease = false;
	}
	~CameraControl();
	void setSensitivity(float value);
	void setCamera(Camera* camera);
	void setScaleSpeed(float speed);
	virtual void onScroll(float offset);	//+1/-1
	virtual void onMouse(int button, int action, double xpos, double ypos);
	virtual void onCursor(double xpos, double ypos);
	virtual void onKey(int key, int action, int mods);
	//每一帧更新行为放到这个函数中
	virtual void update();
};
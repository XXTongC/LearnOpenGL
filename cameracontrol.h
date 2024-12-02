#pragma once
#include <map>
#include "camera.h"
class CameraControl
{
protected:
	//���״̬
	bool mLeftMouseDown;
	bool mRightMouseDown;
	bool mMiddleMouseDown;
	bool mMiddleMouseRelease;
	//��굱ǰλ��
	float mCurrentX;
	float mCurrentY;
	//������ذ������״̬
	std::map<int, bool> mKeyMap;
	//���ж�
	float mSensitivity;
	//��ǰ���Ƶ������
	Camera* mCamera = nullptr;
	//��¼��������ٶ�
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
	//ÿһ֡������Ϊ�ŵ����������
	virtual void update();
};
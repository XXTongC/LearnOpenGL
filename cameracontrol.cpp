#include "cameracontrol.h"


CameraControl::~CameraControl()
{

}

void CameraControl::setScaleSpeed(float speed)
{
	mScaleSpeed = speed;
}


void CameraControl::onScroll(float offset)
{
	
}


void CameraControl::onMouse(int button, int action, double xpos, double ypos)
{
	//std::cout << "Control OnMouse" << std::endl;
	//���жϵ�ǰ�İ����Ƿ���
	bool preesed = action == GLFW_PRESS ? true : false;
	bool release = action == GLFW_RELEASE ? true : false;
	//���������Ҫ��¼��ǰλ��
	if(preesed)
	{
		mCurrentX = xpos;
		mCurrentY = ypos;
	}

	//���ݰ�����갴����ͬ�����İ���״̬
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
		mLeftMouseDown = preesed;
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		mRightMouseDown = preesed;
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		mMiddleMouseDown = preesed;
		mMiddleMouseRelease = release;
		break;
	}
}

void CameraControl::onKey(int key, int action, int mods)
{
	//std::cout << "Control OnKey" << std::endl;
	//�����̧���ǰ���
	if (action == GLFW_REPEAT) return;
	bool preesed = action == GLFW_PRESS ? true : false;
	//��¼��keyMap
	mKeyMap[key] = preesed;
}

void CameraControl::onCursor(double xpos, double ypos)
{
	//std::cout << "Control OnCursor" << std::endl;
}

void CameraControl::update()
{
	//std::cout << "update" << std::endl;
}

void CameraControl::setCamera(Camera* camera)
{
	mCamera = camera;
}


void CameraControl::setSensitivity(float value)
{
	mSensitivity = value;
}


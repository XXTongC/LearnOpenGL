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
	//先判断当前的按键是否按下
	bool preesed = action == GLFW_PRESS ? true : false;
	bool release = action == GLFW_RELEASE ? true : false;
	//如果按下需要记录当前位置
	if(preesed)
	{
		mCurrentX = xpos;
		mCurrentY = ypos;
	}

	//根据按下鼠标按键不同，更改按键状态
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
	//检查是抬起还是按下
	if (action == GLFW_REPEAT) return;
	bool preesed = action == GLFW_PRESS ? true : false;
	//记录在keyMap
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


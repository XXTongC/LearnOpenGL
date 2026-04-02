#include "trackBallCameraControl.h"

TrackBallCameraControl::~TrackBallCameraControl()
{
	
}

void TrackBallCameraControl::onScroll(float offset)
{
	mCamera->scale(offset * mScaleSpeed);
}


void TrackBallCameraControl::onKey(int key, int action, int mods)
{
	CameraControl::onKey(key, action, mods);
	
}

void TrackBallCameraControl::yaw(float angle)
{
	auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3{ 0.0f,1.0f,0.0f });
	mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f);
	mCamera->mRight = mat * glm::vec4(mCamera->mRight, 0.0f);
	mCamera->mPosition = mat * glm::vec4(mCamera->mPosition, 1.0f);
}


void TrackBallCameraControl::pitch(float angle)
{
	//绕着mRight向量的旋转
	auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), mCamera->mRight);
	//影响当前相机的up向量和position
	mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f);	//vec3 <-- vec4, 赋值x, y, z
	mCamera->mPosition = mat * glm::vec4(mCamera->mPosition, 1.0f);
}


void TrackBallCameraControl::onCursor(double xpos, double ypos)
{
	if(mLeftMouseDown)
	{
		//当鼠标被按下时才会调整相机的各类参数
		//1 计算经线和纬线旋转的增量角度（正负性）
		float deltaX = (xpos - mCurrentX) * mSensitivity;
		float deltaY = (ypos - mCurrentY) * mSensitivity;

		//2 分开pitch和yaw角的各自计算
		//并且记得反转一下x,y以适应主观变换
		pitch(-deltaY);
		yaw(-deltaX);
	}
	else if(mMiddleMouseDown)
	{
		float deltaX = (xpos - mCurrentX) * mMoveSpeed;
		float deltaY = (ypos - mCurrentY) * mMoveSpeed;
		//x轴方向需要进行反转以适应主观变换
		mCamera->mPosition += mCamera->mUp * deltaY;
		mCamera->mPosition -= mCamera->mRight * deltaX;
	}
	mCurrentX = xpos;
	mCurrentY = ypos;
}


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
	//����mRight��������ת
	auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), mCamera->mRight);
	//Ӱ�쵱ǰ�����up������position
	mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f);	//vec3 <-- vec4, ��ֵx, y, z
	mCamera->mPosition = mat * glm::vec4(mCamera->mPosition, 1.0f);
}


void TrackBallCameraControl::onCursor(double xpos, double ypos)
{
	if(mLeftMouseDown)
	{
		//����걻����ʱ�Ż��������ĸ������
		//1 ���㾭�ߺ�γ����ת�������Ƕȣ������ԣ�
		float deltaX = (xpos - mCurrentX) * mSensitivity;
		float deltaY = (ypos - mCurrentY) * mSensitivity;

		//2 �ֿ�pitch��yaw�ǵĸ��Լ���
		//���Ҽǵ÷�תһ��x,y����Ӧ���۱任
		pitch(-deltaY);
		yaw(-deltaX);
	}
	else if(mMiddleMouseDown)
	{
		float deltaX = (xpos - mCurrentX) * mMoveSpeed;
		float deltaY = (ypos - mCurrentY) * mMoveSpeed;
		//x�᷽����Ҫ���з�ת����Ӧ���۱任
		mCamera->mPosition += mCamera->mUp * deltaY;
		mCamera->mPosition -= mCamera->mRight * deltaX;
	}
	mCurrentX = xpos;
	mCurrentY = ypos;
}


#include "gamecameracontrol.h"

GameCameraControl::~GameCameraControl()
{
	
}

void GameCameraControl::update()
{
	//���յ��ƶ�����
	glm::vec3 direction(0.0f);
	auto front = glm::cross(mCamera->mUp, mCamera->mRight);
	auto right = mCamera->mRight;
	auto up = glm::vec3{0.0f,1.0f,0.0f};
	if(mKeyMap[GLFW_KEY_W])
		direction += front;
	if (mKeyMap[GLFW_KEY_S])
		direction -= front;
	if (mKeyMap[GLFW_KEY_A])
		direction -= right;
	if (mKeyMap[GLFW_KEY_D])
		direction += right;
	if (mKeyMap[GLFW_KEY_SPACE])
		direction += up;
	if (mKeyMap[GLFW_KEY_LEFT_CONTROL])
		direction -= up;
	if (glm::length(direction) != 0.0f)
	{
		direction = glm::normalize(direction);
		mCamera->mPosition += direction * mMoveSpeed;
	}
}


void GameCameraControl::pitch(float angle)
{
	mPicth += angle;
	if(mPicth>89.0f||mPicth<-89.0f)
	{
		mPicth -= angle;
		return;
	}
	//gamecameracontrol�У�pitch����Ӱ��mPosition
	auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), mCamera->mRight);
	mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f);
}

void GameCameraControl::yaw(float angle)
{
	auto mat = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3{ 0.0f,1.0f,0.0f });
	mCamera->mUp = mat * glm::vec4(mCamera->mUp, 0.0f);
	mCamera->mRight = mat * glm::vec4(mCamera->mRight, 0.0f);
}


void GameCameraControl::onCursor(double xpos, double ypos)
{
	//1 ���㾭�ߺ�γ����ת�������Ƕȣ������ԣ�
	float deltaX = (xpos - mCurrentX) * mSensitivity;
	float deltaY = (ypos - mCurrentY) * mSensitivity;

	//2 �ֿ�pitch��yaw�ǵĸ��Լ���
	//���Ҽǵ÷�תһ��x,y����Ӧ���۱任
	if (mRightMouseDown)
	{
		pitch(-deltaY);
		yaw(-deltaX);
	}
	

	mCurrentX = xpos;
	mCurrentY = ypos;
}

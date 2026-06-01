#include "PBRCameraRigProfile.h"

#include "../../camera/camera.h"
#include "../../camera/perspectivecamera.h"

void GL_SCENE::PBRCameraRigProfile::applyTo(Camera* camera) const
{
	if (!camera)
	{
		return;
	}

	camera->mPosition = position;
	camera->mUp = up;
	camera->mRight = right;
	camera->mNear = nearPlane;
	camera->mFar = farPlane;

	if (auto perspectiveCamera = dynamic_cast<PerspectiveCamera*>(camera))
	{
		perspectiveCamera->setFovy(fovy);
	}
}

void GL_SCENE::PBRCameraRigProfile::copyFrom(const Camera* camera)
{
	if (!camera)
	{
		return;
	}

	position = camera->mPosition;
	up = camera->mUp;
	right = camera->mRight;
	nearPlane = camera->mNear;
	farPlane = camera->mFar;

	if (const auto perspectiveCamera = dynamic_cast<const PerspectiveCamera*>(camera))
	{
		fovy = perspectiveCamera->getFovy();
	}
}

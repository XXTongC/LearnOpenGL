#include "PBRCameraRigProfile.h"

#include <array>
#include <string>

#include "../../camera/camera.h"
#include "../../camera/perspectivecamera.h"
#include "../inspector/PropertySchema.h"

namespace
{
	std::array<std::string, 3> xyzKeys(const std::string& prefix)
	{
		return { prefix + "X", prefix + "Y", prefix + "Z" };
	}
}

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

void GL_SCENE::PBRCameraRigProfile::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	builder.addSection("Camera Rig");
	builder.addConfigVec3(xyzKeys("position"), "Position", &position);
	builder.addConfigVec3(xyzKeys("up"), "Up", &up);
	builder.addConfigVec3(xyzKeys("right"), "Right", &right);
	builder.addConfigFloat("fovy", "Fovy", &fovy, 1.0f, 179.0f);
	builder.addConfigFloat("nearPlane", "Near", &nearPlane, 0.001f, 100.0f);
	builder.addConfigFloat("farPlane", "Far", &farPlane, 1.0f, 10000.0f);
}

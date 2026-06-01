#include "PBRCameraRigProfileConfig.h"

#include <array>
#include <string>

#include "PBRCameraRigProfile.h"
#include "../inspector/PropertySchema.h"

namespace
{
	std::array<std::string, 3> xyzKeys(const std::string& prefix)
	{
		return { prefix + "X", prefix + "Y", prefix + "Z" };
	}
}

void GL_SCENE::buildPBRCameraRigProfileConfigSchema(
	GL_EDITOR::PropertyBuilder& builder,
	PBRCameraRigProfile& profile
)
{
	builder.addSection("Camera Rig");
	builder.addConfigVec3(xyzKeys("position"), "Position", &profile.position);
	builder.addConfigVec3(xyzKeys("up"), "Up", &profile.up);
	builder.addConfigVec3(xyzKeys("right"), "Right", &profile.right);
	builder.addConfigFloat("fovy", "Fovy", &profile.fovy, 1.0f, 179.0f);
	builder.addConfigFloat("nearPlane", "Near", &profile.nearPlane, 0.001f, 100.0f);
	builder.addConfigFloat("farPlane", "Far", &profile.farPlane, 1.0f, 10000.0f);
}

#pragma once

#include <memory>
#include <string>

#include "PropertySchema.h"

class Camera;

namespace GLframework
{
	class Light;
	class Object;
	class Shadow;
}

namespace GL_EDITOR
{
	std::string getCameraTypeName(Camera* camera);
	std::string getObjectDisplayName(const std::shared_ptr<GLframework::Object>& object);
	std::string getObjectTypeName(const std::shared_ptr<GLframework::Object>& object);
	std::string getShadowTypeName(const std::shared_ptr<GLframework::Shadow>& shadow);
	PropertyBuilder buildCameraPropertySchema(Camera* camera);
	PropertyBuilder buildLightPropertySchema(const std::shared_ptr<GLframework::Light>& light);
	PropertyBuilder buildObjectTransformPropertySchema(const std::shared_ptr<GLframework::Object>& object);
	PropertyBuilder buildShadowPropertySchema(const std::shared_ptr<GLframework::Shadow>& shadow);
}

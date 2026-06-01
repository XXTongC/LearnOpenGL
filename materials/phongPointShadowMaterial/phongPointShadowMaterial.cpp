#include "phongPointShadowMaterial.h"
#include "../../tools/inspector/MaterialInspector.h"
#include "../../tools/inspector/PropertySchema.h"
using namespace GLframework;

PhongPointShadowMaterial::PhongPointShadowMaterial()
{
	//setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
	setMaterialType(MaterialType::PhongPointShadowMaterial);
}

PhongPointShadowMaterial::~PhongPointShadowMaterial()
{

}

void PhongPointShadowMaterial::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	Material::visitEditableProperties(builder);

	builder.addSection("Surface");
	builder.addFloat("Shininess", &mShiness, 0.0f, 256.0f, "%.1f");
	builder.addText("Diffuse", [this]() { return GL_EDITOR::describeTexture(mDiffuse); });
	builder.addText("Specular Mask", [this]() { return GL_EDITOR::describeTexture(mSpecularMask); });
}

#include "phongMaterial.h"
#include "tools/inspector/MaterialInspector.h"
using namespace GLframework;

PhongMaterial::PhongMaterial()
{
	//setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png",1);
	setMaterialType(MaterialType::PhongMaterial);
}

PhongMaterial::~PhongMaterial()
{
	
}

void PhongMaterial::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	Material::visitEditableProperties(builder);

	builder.addSection("Surface");
	builder.addFloat("Shininess", &mShiness, 0.0f, 256.0f, "%.1f");
	builder.addText("Diffuse", [this]() { return GL_EDITOR::describeTexture(mDiffuse); });
	builder.addText("Specular Mask", [this]() { return GL_EDITOR::describeTexture(mSpecularMask); });
}



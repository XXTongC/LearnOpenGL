#include "PBRMaterial.h"

#include "tools/inspector/MaterialInspector.h"

using namespace GLframework;

PBRMaterial::PBRMaterial()
{
	setMaterialType(MaterialType::PBRMaterial);
}

void PBRMaterial::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	Material::visitEditableProperties(builder);

	builder.addSection("PBR Surface");
	builder.addColor3("Albedo", &mAlbedo);
	builder.addFloat("Metallic", &mMetallic, 0.0f, 1.0f);
	builder.addFloat("Roughness", &mRoughness, 0.04f, 1.0f);
	builder.addFloat("AO", &mAo, 0.0f, 1.0f);
	builder.addColor3("Emissive Color", &mEmissiveColor);
	builder.addFloat("Emissive Intensity", &mEmissiveIntensity, 0.0f, 20.0f);

	builder.addText("Albedo Map", [this]() { return GL_EDITOR::describeTexture(mAlbedoMap); });
	builder.addText("Metallic Map", [this]() { return GL_EDITOR::describeTexture(mMetallicMap); });
	builder.addText("Roughness Map", [this]() { return GL_EDITOR::describeTexture(mRoughnessMap); });
	builder.addText("AO Map", [this]() { return GL_EDITOR::describeTexture(mAoMap); });
	builder.addText("Normal Map", [this]() { return GL_EDITOR::describeTexture(mNormalMap); });
	builder.addText("Emissive Map", [this]() { return GL_EDITOR::describeTexture(mEmissiveMap); });
}

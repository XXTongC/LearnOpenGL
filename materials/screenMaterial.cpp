#include "screenMaterial.h"
#include "tools/inspector/MaterialInspector.h"
using namespace GLframework;

void ScreenMaterial::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	Material::visitEditableProperties(builder);

	builder.addSection("Post Process");
	builder.addFloat("Exposure", &mExposure, 0.0f, 4.0f);
	builder.addText("Screen Texture", [this]() { return GL_EDITOR::describeTexture(mScreenTexture); });
	builder.addText("Depth Stencil Texture", [this]() { return GL_EDITOR::describeTexture(mDepthStencilTexture); });
}


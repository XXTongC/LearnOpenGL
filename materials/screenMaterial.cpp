#include "screenMaterial.h"
#include "tools/inspector/MaterialInspector.h"
#include "tools/inspector/PropertySchema.h"
using namespace GLframework;

void ScreenMaterial::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	Material::visitEditableProperties(builder);

	builder.addSection("Post Process Inputs");
	builder.addText("Screen Texture", [this]() { return GL_EDITOR::describeTexture(mScreenTexture); });
	builder.addText("Bloom Texture", [this]() { return GL_EDITOR::describeTexture(mBloomTexture); });
	builder.addText("Depth Stencil Texture", [this]() { return GL_EDITOR::describeTexture(mDepthStencilTexture); });
}


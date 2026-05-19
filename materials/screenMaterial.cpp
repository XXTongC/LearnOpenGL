#include "screenMaterial.h"
#include "tools/inspector/MaterialInspector.h"
using namespace GLframework;

void ScreenMaterial::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	Material::visitEditableProperties(builder);

	builder.addSection("Post Process");
	builder.addFloat("Exposure", &mSettings.exposure, 0.0f, 4.0f);
	builder.addInt(
		"Tone Mapping Mode",
		[this]() { return static_cast<int>(mSettings.toneMappingMode); },
		[this](int value) { mSettings.toneMappingMode = value == 1 ? ToneMappingMode::Reinhard : ToneMappingMode::Exposure; },
		0,
		1
	);
	builder.addBool("Bloom Enabled", &mSettings.bloomEnabled);
	builder.addFloat("Bloom Threshold", &mSettings.bloomThreshold, 0.0f, 20.0f);
	builder.addFloat("Bloom Intensity", &mSettings.bloomIntensity, 0.0f, 2.0f);
	builder.addInt("Bloom Iterations", &mSettings.bloomIterations, 0, 20);
	builder.addText("Screen Texture", [this]() { return GL_EDITOR::describeTexture(mScreenTexture); });
	builder.addText("Bloom Texture", [this]() { return GL_EDITOR::describeTexture(mBloomTexture); });
	builder.addText("Depth Stencil Texture", [this]() { return GL_EDITOR::describeTexture(mDepthStencilTexture); });
}


#include "RuntimeFramePipelineProfileConfig.h"

#include "RuntimeFramePipelineProfile.h"
#include "../tools/inspector/PropertySchema.h"

void GL_RUNTIME::buildRuntimeFramePipelineProfileConfigSchema(
	GL_EDITOR::PropertyBuilder& builder,
	RuntimeFramePipelineProfile& profile
)
{
	builder.addSection("Frame Pipeline Plan");
	builder.addConfigString("passOrder", "Pass Order", &profile.passOrder);
	builder.addSection("Frame Pipeline Passes");
	builder.addConfigBool("sceneColorPassEnabled", "Scene Color Pass", &profile.sceneColorPassEnabled);
	builder.addConfigBool("sceneResolvePassEnabled", "Scene Resolve Pass", &profile.sceneResolvePassEnabled);
	builder.addConfigBool("bloomPassEnabled", "Bloom Pass", &profile.bloomPassEnabled);
	builder.addConfigBool("screenCompositePassEnabled", "Screen Composite Pass", &profile.screenCompositePassEnabled);
}

#include "PBRMaterialProfileConfig.h"

#include "PBRMaterialProfile.h"
#include "tools/inspector/PropertySchema.h"

namespace
{
	void addMaterialProfileSurfaceProperties(
		GL_EDITOR::PropertyBuilder& builder,
		GLframework::PBRMaterialProfile& profile
	)
	{
		builder.addSection("PBR Surface");
		builder.addConfigColor3({ "albedoR", "albedoG", "albedoB" }, "Albedo", &profile.albedo);
		builder.addConfigFloat("metallic", "Metallic", &profile.metallic, 0.0f, 1.0f);
		builder.addConfigFloat("roughness", "Roughness", &profile.roughness, 0.04f, 1.0f);
		builder.addConfigFloat("ao", "AO", &profile.ao, 0.0f, 1.0f);
		builder.addConfigColor3({ "emissiveR", "emissiveG", "emissiveB" }, "Emissive Color", &profile.emissiveColor);
		builder.addConfigFloat("emissiveIntensity", "Emissive Intensity", &profile.emissiveIntensity, 0.0f, 20.0f);

		builder.addSection("PBR Alpha Mask");
		builder.addConfigBool("useAlphaMask", "Use Alpha Mask", &profile.useAlphaMask);
		builder.addConfigFloat("alphaCutoff", "Alpha Cutoff", &profile.alphaCutoff, 0.0f, 1.0f);
	}

	void addMaterialProfileIblProperties(
		GL_EDITOR::PropertyBuilder& builder,
		GLframework::PBRMaterialProfile& profile
	)
	{
		builder.addSection("IBL");
		builder.addConfigBool("useIBL", "Use IBL", &profile.useIBL);
		builder.addConfigFloat("iblDiffuseStrength", "IBL Diffuse Strength", &profile.iblDiffuseStrength, 0.0f, 5.0f);
		builder.addConfigFloat("iblSpecularStrength", "IBL Specular Strength", &profile.iblSpecularStrength, 0.0f, 5.0f);
	}
}

void GLframework::buildPBRMaterialProfileConfigSchema(
	GL_EDITOR::PropertyBuilder& builder,
	PBRMaterialProfile& profile
)
{
	addMaterialProfileSurfaceProperties(builder, profile);
	addMaterialProfileIblProperties(builder, profile);
}

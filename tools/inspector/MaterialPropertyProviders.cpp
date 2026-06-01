#include "MaterialPropertyProviders.h"

#include "../../materials/grassInstanceMaterial/grassInstanceMaterial.h"
#include "../../materials/material.h"
#include "../../materials/pbrMaterial/PBRMaterial.h"
#include "../../materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h"
#include "../../materials/phongMaterial.h"
#include "../../materials/phongPointShadowMaterial/phongPointShadowMaterial.h"
#include "../../materials/screenMaterial.h"
#include "MaterialInspector.h"

namespace
{
	bool canBuildAnyMaterial(const GL_EDITOR::MaterialPropertyProviderContext&)
	{
		return true;
	}

	template <typename TMaterial>
	bool canBuildMaterialType(const GL_EDITOR::MaterialPropertyProviderContext& context)
	{
		return dynamic_cast<TMaterial*>(&context.material) != nullptr;
	}

	template <typename TMaterial>
	TMaterial* asMaterialType(const GL_EDITOR::MaterialPropertyProviderContext& context)
	{
		return dynamic_cast<TMaterial*>(&context.material);
	}

	void addTextureText(
		GL_EDITOR::PropertyBuilder& builder,
		const char* label,
		const std::shared_ptr<GLframework::Texture>* texture
	)
	{
		builder.addText(label, [texture]()
		{
			return GL_EDITOR::describeTexture(texture ? *texture : nullptr);
		});
	}

	void buildRenderStateProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::MaterialPropertyProviderContext& context
	)
	{
		auto& material = context.material;
		builder.addSection("Render State");
		builder.addBool(
			"Depth Test",
			[&material]() { return material.getDepthTest(); },
			[&material](bool value) { material.setDepthTest(value); }
		);
		builder.addBool(
			"Depth Write",
			[&material]() { return material.getDepthWrite(); },
			[&material](bool value) { material.setDepthWrite(value); }
		);
		builder.addBool(
			"Blend",
			[&material]() { return material.getColorBlendState(); },
			[&material](bool value) { material.setColorBlendState(value); }
		);
		builder.addBool(
			"Face Culling",
			[&material]() { return material.getFaceCullingState(); },
			[&material](bool value) { material.setFaceCullingState(value); }
		);
		builder.addFloat(
			"Opacity",
			[&material]() { return material.getOpacity(); },
			[&material](float value) { material.setOpacity(value); },
			0.0f,
			1.0f
		);
		builder.addFloat(
			"Polygon Offset Factor",
			[&material]() { return material.getFactor(); },
			[&material](float value) { material.setFactor(value); },
			-8.0f,
			8.0f
		);
		builder.addFloat(
			"Polygon Offset Unit",
			[&material]() { return material.getUnit(); },
			[&material](float value) { material.setUnit(value); },
			-8.0f,
			8.0f
		);
	}

	template <typename TMaterial>
	void buildPhongSurfaceProperties(GL_EDITOR::PropertyBuilder& builder, TMaterial& material)
	{
		builder.addSection("Surface");
		builder.addFloat("Shininess", material.shininessControl(), 0.0f, 256.0f, "%.1f");
		addTextureText(builder, "Diffuse", &material.diffuseTexture());
		addTextureText(builder, "Specular Mask", &material.specularMaskTexture());
	}

	template <typename TMaterial>
	void buildPhongMaterialProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::MaterialPropertyProviderContext& context
	)
	{
		if (auto* material = asMaterialType<TMaterial>(context))
		{
			buildPhongSurfaceProperties(builder, *material);
		}
	}

	void addPbrFloatUniformProperty(
		GL_EDITOR::PropertyBuilder& builder,
		const GLframework::PBRFloatUniformSlot& slot
	)
	{
		builder.addFloat(slot.label, slot.value, slot.minValue, slot.maxValue);
	}

	void buildPbrMaterialProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::MaterialPropertyProviderContext& context
	)
	{
		auto* material = asMaterialType<GLframework::PBRMaterial>(context);
		if (!material)
		{
			return;
		}

		builder.addSection("PBR Surface");
		const auto vec3UniformSlots = material->getVec3UniformSlots();
		const auto surfaceFloatUniformSlots = material->getSurfaceFloatUniformSlots();
		builder.addColor3(vec3UniformSlots[0].label, vec3UniformSlots[0].value);
		addPbrFloatUniformProperty(builder, surfaceFloatUniformSlots[0]);
		addPbrFloatUniformProperty(builder, surfaceFloatUniformSlots[1]);
		addPbrFloatUniformProperty(builder, surfaceFloatUniformSlots[2]);
		builder.addColor3(vec3UniformSlots[1].label, vec3UniformSlots[1].value);
		addPbrFloatUniformProperty(builder, surfaceFloatUniformSlots[3]);

		builder.addSection("PBR Alpha Mask");
		builder.addBool("Use Alpha Mask", material->useAlphaMaskControl());
		builder.addFloat("Alpha Cutoff", material->alphaCutoffControl(), 0.0f, 1.0f);

		builder.addSection("PBR Texture Channels");
		builder.addInt("Metallic Map Channel", material->metallicMapChannelControl(), 0, 3);
		builder.addInt("Roughness Map Channel", material->roughnessMapChannelControl(), 0, 3);
		builder.addInt("AO Map Channel", material->aoMapChannelControl(), 0, 3);

		builder.addSection("PBR IBL");
		builder.addBool("Use IBL", material->useIblControl());
		for (const auto& slot : material->getIblFloatUniformSlots())
		{
			addPbrFloatUniformProperty(builder, slot);
		}

		builder.addSection("PBR Textures");
		for (const auto& slot : material->getTextureSlots())
		{
			addTextureText(builder, slot.label, slot.texture);
		}
	}

	void buildGrassInstanceMaterialProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::MaterialPropertyProviderContext& context
	)
	{
		auto* material = asMaterialType<GLframework::GrassInstanceMaterial>(context);
		if (!material)
		{
			return;
		}

		builder.addSection("Surface");
		builder.addFloat("Shininess", material->shininessControl(), 0.0f, 256.0f, "%.1f");
		builder.addFloat("UV Scale", material->Control_UVScale(), 0.0f, 100.0f);
		builder.addFloat("Brightness", material->Control_Brightness(), 0.0f, 8.0f);
		addTextureText(builder, "Diffuse", &material->diffuseTexture());
		addTextureText(builder, "Opacity Mask", &material->opacityMaskTexture());
		addTextureText(builder, "Cloud Mask", &material->cloudMaskTexture());

		builder.addSection("Wind");
		builder.addFloat("Wind Scale", material->Control_WindScale(), -0.12f, 0.12f, "%.4f");
		builder.addFloat("Phase Scale", material->Control_PhaseScale(), 0.0f, 10.0f);
		builder.addVec3(
			"Wind Direction",
			[material]() { return *material->Control_WindDirection(); },
			[material](glm::vec3 value) { material->setWindDirection(value); }
		);

		builder.addSection("Cloud");
		builder.addFloat("Cloud Lerp", material->Control_CloudLerp(), 0.0f, 1.0f);
		builder.addFloat("Cloud UV Scale", material->Control_CloudUVScale(), 0.0f, 100.0f);
		builder.addFloat("Cloud Speed", material->Control_CloudSpeed(), 0.0f, 3.0f);
		builder.addColor3(
			"Cloud White Color",
			[material]() { return *material->Control_CloudWhiteColor(); },
			[material](glm::vec3 value) { material->seCloudWhiteColor(value); }
		);
		builder.addColor3(
			"Cloud Black Color",
			[material]() { return *material->Control_CloudBlackColor(); },
			[material](glm::vec3 value) { material->setCloudBlackColor(value); }
		);
	}

	void buildScreenMaterialProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::MaterialPropertyProviderContext& context
	)
	{
		auto* material = asMaterialType<GLframework::ScreenMaterial>(context);
		if (!material)
		{
			return;
		}

		builder.addSection("Post Process Inputs");
		addTextureText(builder, "Screen Texture", &material->screenTexture());
		addTextureText(builder, "Bloom Texture", &material->bloomTexture());
		addTextureText(builder, "Depth Stencil Texture", &material->depthStencilTexture());
	}

	GL_EDITOR::MaterialPropertyProviderRegistry buildDefaultMaterialPropertyProviderRegistry()
	{
		GL_EDITOR::MaterialPropertyProviderRegistry registry{};
		registry.registerProvider({
			"render-state",
			canBuildAnyMaterial,
			buildRenderStateProperties
		});
		registry.registerProvider({
			"phong-material",
			canBuildMaterialType<GLframework::PhongMaterial>,
			buildPhongMaterialProperties<GLframework::PhongMaterial>
		});
		registry.registerProvider({
			"phong-csm-shadow-material",
			canBuildMaterialType<GLframework::PhongCSMShadowMaterial>,
			buildPhongMaterialProperties<GLframework::PhongCSMShadowMaterial>
		});
		registry.registerProvider({
			"phong-point-shadow-material",
			canBuildMaterialType<GLframework::PhongPointShadowMaterial>,
			buildPhongMaterialProperties<GLframework::PhongPointShadowMaterial>
		});
		registry.registerProvider({
			"grass-instance-material",
			canBuildMaterialType<GLframework::GrassInstanceMaterial>,
			buildGrassInstanceMaterialProperties
		});
		registry.registerProvider({
			"screen-material",
			canBuildMaterialType<GLframework::ScreenMaterial>,
			buildScreenMaterialProperties
		});
		registry.registerProvider({
			"pbr-material",
			canBuildMaterialType<GLframework::PBRMaterial>,
			buildPbrMaterialProperties
		});
		return registry;
	}
}

namespace GL_EDITOR
{
	const MaterialPropertyProviderRegistry& getDefaultMaterialPropertyProviderRegistry()
	{
		static const auto registry = buildDefaultMaterialPropertyProviderRegistry();
		return registry;
	}
}

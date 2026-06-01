#include "MaterialPropertyProviders.h"

#include "../../materials/material.h"

namespace
{
	bool canBuildLegacyMaterialProperties(const GL_EDITOR::MaterialPropertyProviderContext&)
	{
		return true;
	}

	void buildLegacyMaterialProperties(
		GL_EDITOR::PropertyBuilder& builder,
		const GL_EDITOR::MaterialPropertyProviderContext& context
	)
	{
		context.material.visitEditableProperties(builder);
	}

	GL_EDITOR::MaterialPropertyProviderRegistry buildDefaultMaterialPropertyProviderRegistry()
	{
		GL_EDITOR::MaterialPropertyProviderRegistry registry{};
		registry.registerProvider({
			"legacy-visit-editable-properties",
			canBuildLegacyMaterialProperties,
			buildLegacyMaterialProperties
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

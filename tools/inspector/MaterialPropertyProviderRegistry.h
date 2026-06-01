#pragma once

#include <functional>
#include <string>
#include <vector>

#include "PropertySchema.h"

namespace GLframework
{
	class Material;
}

namespace GL_EDITOR
{
	struct MaterialPropertyProviderContext
	{
		GLframework::Material& material;
	};

	using MaterialPropertyCanBuild = std::function<bool(const MaterialPropertyProviderContext&)>;
	using MaterialPropertyBuild = std::function<void(PropertyBuilder&, const MaterialPropertyProviderContext&)>;

	struct MaterialPropertyProvider
	{
		std::string key{};
		MaterialPropertyCanBuild canBuild{};
		MaterialPropertyBuild build{};
	};

	class MaterialPropertyProviderRegistry
	{
	public:
		bool registerProvider(MaterialPropertyProvider provider);
		const MaterialPropertyProvider* findProvider(const MaterialPropertyProviderContext& context) const;
		bool buildFirst(PropertyBuilder& builder, const MaterialPropertyProviderContext& context) const;

	private:
		std::vector<MaterialPropertyProvider> mProviders{};
	};
}

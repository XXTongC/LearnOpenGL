#pragma once

#include <memory>
#include <string>

namespace GLframework
{
	class Object;
}

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState;

	class RuntimeAssetImportService
	{
	public:
		static std::shared_ptr<GLframework::Object> loadPbrAsset(
			const RuntimeRenderResourceState& renderResources,
			const std::string& path
		);
	};
}

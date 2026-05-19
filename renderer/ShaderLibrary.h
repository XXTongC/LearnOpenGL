#pragma once

#include <map>
#include <memory>

#include "material.h"
#include "shader.h"

namespace GLframework
{
	class ShaderLibrary
	{
	public:
		void initialize();
		std::shared_ptr<Shader> get(MaterialType type) const;
		std::shared_ptr<Shader> getShadowShader() const;
		std::shared_ptr<Shader> getShadowDistanceShader() const;
		std::shared_ptr<Shader> getEquirectangularToCubemapShader() const;
		std::shared_ptr<Shader> getIrradianceConvolutionShader() const;
		std::shared_ptr<Shader> getPrefilterShader() const;
		std::shared_ptr<Shader> getBrdfLutShader() const;

	private:
		std::shared_ptr<Shader> createShader(const char* vertexPath, const char* fragmentPath);

		std::map<MaterialType, std::shared_ptr<Shader>> mMaterialShaders{};
		std::shared_ptr<Shader> mShadowShader{ nullptr };
		std::shared_ptr<Shader> mShadowDistanceShader{ nullptr };
		std::shared_ptr<Shader> mEquirectangularToCubemapShader{ nullptr };
		std::shared_ptr<Shader> mIrradianceConvolutionShader{ nullptr };
		std::shared_ptr<Shader> mPrefilterShader{ nullptr };
		std::shared_ptr<Shader> mBrdfLutShader{ nullptr };
	};
}

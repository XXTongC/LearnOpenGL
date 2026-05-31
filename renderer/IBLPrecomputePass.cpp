#include "IBLPrecomputePass.h"

#include "framework/texture.h"
#include "renderer/MeshDraw.h"

using namespace GLframework;

namespace
{
	struct FramebufferViewportScope
	{
		FramebufferViewportScope()
		{
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
			glGetIntegerv(GL_VIEWPORT, previousViewport.data());
		}

		~FramebufferViewportScope()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);
			glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
		}

		GLint previousFramebuffer{ 0 };
		std::array<GLint, 4> previousViewport{};
	};

	glm::mat4 createCaptureProjection()
	{
		return glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	}
}

bool IBLPrecomputePass::captureEnvironmentMap(
	const std::shared_ptr<Texture>& equirectangularMap,
	EnvironmentRenderTargets& targets,
	const std::shared_ptr<Mesh>& captureCube,
	const ShaderLibrary& shaderLibrary
) const
{
	if (!equirectangularMap || !targets.isInitialized())
	{
		return false;
	}

	const auto shader = shaderLibrary.getEquirectangularToCubemapShader();
	if (!shader)
	{
		return false;
	}

	FramebufferViewportScope scope;
	shader->begin();
	shader->setMat4("projectionMatrix", createCaptureProjection());
	shader->setInt("equirectangularMap", static_cast<int>(equirectangularMap->getUnit()));
	equirectangularMap->Bind();

	const bool rendered = renderCubemapFaces(
		targets,
		targets.getEnvironmentMap(),
		captureCube,
		shader,
		targets.getSettings().environmentSize
	);

	if (rendered)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, targets.getEnvironmentMap()->getTexture());
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	shader->end();
	return rendered;
}

bool IBLPrecomputePass::computeIrradianceMap(
	EnvironmentRenderTargets& targets,
	const std::shared_ptr<Mesh>& captureCube,
	const ShaderLibrary& shaderLibrary
) const
{
	if (!targets.isInitialized())
	{
		return false;
	}

	const auto shader = shaderLibrary.getIrradianceConvolutionShader();
	if (!shader)
	{
		return false;
	}

	FramebufferViewportScope scope;
	shader->begin();
	shader->setMat4("projectionMatrix", createCaptureProjection());
	shader->setInt("environmentMap", static_cast<int>(targets.getEnvironmentMap()->getUnit()));
	targets.getEnvironmentMap()->Bind();

	const bool rendered = renderCubemapFaces(
		targets,
		targets.getIrradianceMap(),
		captureCube,
		shader,
		targets.getSettings().irradianceSize
	);

	shader->end();
	return rendered;
}

bool IBLPrecomputePass::computePrefilterMap(
	EnvironmentRenderTargets& targets,
	const std::shared_ptr<Mesh>& captureCube,
	const ShaderLibrary& shaderLibrary
) const
{
	if (!targets.isInitialized())
	{
		return false;
	}

	const auto shader = shaderLibrary.getPrefilterShader();
	if (!shader)
	{
		return false;
	}

	FramebufferViewportScope scope;
	shader->begin();
	shader->setMat4("projectionMatrix", createCaptureProjection());
	shader->setInt("environmentMap", static_cast<int>(targets.getEnvironmentMap()->getUnit()));
	targets.getEnvironmentMap()->Bind();

	const unsigned int maxMipLevels = targets.getMaxPrefilterMipLevels();
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipSize = targets.getSettings().prefilterSize >> mip;
		if (mipSize == 0)
		{
			mipSize = 1;
		}
		const float roughness = maxMipLevels > 1
			? static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1)
			: 0.0f;
		shader->setFloat("roughness", roughness);

		if (!renderCubemapFaces(targets, targets.getPrefilterMap(), captureCube, shader, mipSize, mip))
		{
			shader->end();
			return false;
		}
	}

	shader->end();
	return true;
}

bool IBLPrecomputePass::computeBrdfLut(
	EnvironmentRenderTargets& targets,
	const std::shared_ptr<Mesh>& screenQuad,
	const ShaderLibrary& shaderLibrary
) const
{
	if (!targets.isInitialized())
	{
		return false;
	}

	const auto shader = shaderLibrary.getBrdfLutShader();
	if (!shader)
	{
		return false;
	}

	FramebufferViewportScope scope;
	targets.attachBrdfLutForCapture();
	glViewport(
		0,
		0,
		static_cast<GLsizei>(targets.getSettings().brdfLutSize),
		static_cast<GLsizei>(targets.getSettings().brdfLutSize)
	);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader->begin();
	const bool rendered = MeshDraw::drawIndexed(screenQuad);
	shader->end();
	return rendered;
}

std::array<glm::mat4, 6> IBLPrecomputePass::createCaptureViews() const
{
	return {
		glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};
}

bool IBLPrecomputePass::renderCubemapFaces(
	EnvironmentRenderTargets& targets,
	const std::shared_ptr<Texture>& outputCubemap,
	const std::shared_ptr<Mesh>& captureCube,
	const std::shared_ptr<Shader>& shader,
	unsigned int captureSize,
	unsigned int mipLevel
) const
{
	if (!outputCubemap || !captureCube || !shader || captureSize == 0)
	{
		return false;
	}

	const auto views = createCaptureViews();
	for (size_t face = 0; face < views.size(); ++face)
	{
		targets.attachCubemapFaceForCapture(outputCubemap, static_cast<unsigned int>(face), mipLevel, captureSize);
		glViewport(0, 0, static_cast<GLsizei>(captureSize), static_cast<GLsizei>(captureSize));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader->setMat4("viewMatrix", views[face]);

		if (!MeshDraw::drawIndexed(captureCube))
		{
			return false;
		}
	}

	return true;
}

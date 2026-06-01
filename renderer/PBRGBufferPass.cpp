#include "PBRGBufferPass.h"

#include "core.h"
#include "framework/shader.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "mesh/mesh.h"
#include "renderer/MaterialBindingContext.h"
#include "renderer/MeshDraw.h"
#include "renderer/PBRGBufferRenderTargets.h"
#include "renderer/PBRObjectUniformBinder.h"
#include "renderer/PBRSurfaceResourceBinder.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

namespace
{
	struct FramebufferViewportScope
	{
		FramebufferViewportScope()
		{
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
			glGetIntegerv(GL_VIEWPORT, previousViewport);
		}

		~FramebufferViewportScope()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);
			glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
		}

		GLint previousFramebuffer{ 0 };
		GLint previousViewport[4]{};
	};

	bool canRenderMesh(const std::shared_ptr<Mesh>& mesh)
	{
		return mesh
			&& mesh->getMaterial()
			&& mesh->getMaterial()->getMaterialType() == MaterialType::PBRMaterial;
	}
}

PBRGBufferPassStats PBRGBufferPass::render(
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const MaterialBindingContext& context,
	ShaderLibrary& shaderLibrary,
	PBRGBufferRenderTargets& targets
)
{
	PBRGBufferPassStats stats{};
	if (meshes.empty() || context.camera == nullptr)
	{
		return stats;
	}

	GLint viewport[4]{};
	glGetIntegerv(GL_VIEWPORT, viewport);
	const auto targetWidth = static_cast<unsigned int>(viewport[2]);
	const auto targetHeight = static_cast<unsigned int>(viewport[3]);

	FramebufferViewportScope scope{};
	if (!targets.ensureSize(targetWidth, targetHeight))
	{
		return stats;
	}

	stats.ready = targets.isComplete();
	stats.targetWidth = static_cast<int>(targets.getWidth());
	stats.targetHeight = static_cast<int>(targets.getHeight());

	const auto shader = shaderLibrary.getPbrGBufferShader();
	if (!shader)
	{
		return stats;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, targets.getFbo());
	glViewport(0, 0, targets.getWidth(), targets.getHeight());
	const unsigned int attachments[5]{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4
	};
	glDrawBuffers(5, attachments);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_BLEND);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);
	const float clearColor[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
	const float clearDepth{ 1.0f };
	glClearBufferfv(GL_COLOR, 0, clearColor);
	glClearBufferfv(GL_COLOR, 1, clearColor);
	glClearBufferfv(GL_COLOR, 2, clearColor);
	glClearBufferfv(GL_COLOR, 3, clearColor);
	glClearBufferfv(GL_COLOR, 4, clearColor);
	glClearBufferfv(GL_DEPTH, 0, &clearDepth);

	shader->begin();
	for (const auto& mesh : meshes)
	{
		if (!canRenderMesh(mesh))
		{
			continue;
		}

		const auto material = std::static_pointer_cast<PBRMaterial>(mesh->getMaterial());
		if (!PBRObjectUniformBinder::bind(shader, material, mesh, context))
		{
			continue;
		}
		PBRSurfaceResourceBinder::bind(shader, material);
		shader->setInt("useIBL", material->iblState().useIbl ? 1 : 0);
		for (const auto& slot : material->getIblFloatUniformSlots())
		{
			shader->setFloat(slot.uniformName, slot.value ? *slot.value : 0.0f);
		}
		if (MeshDraw::drawIndexed(mesh))
		{
			++stats.drawCalls;
		}
	}
	shader->end();
	return stats;
}

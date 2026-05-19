#include "RenderState.h"

#include "core.h"
#include "material.h"

void GLframework::RenderState::applyMaterialState(const Material& material)
{
	applyDepthState(material);
	applyPolygonOffsetState(material);
	applyStencilState(material);
	applyColorBlendState(material);
	applyFaceCullingState(material);
}

void GLframework::RenderState::applyDepthState(const Material& material)
{
	if (material.getDepthTest())
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(material.getDepthFunc());
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	glDepthMask(material.getDepthWrite() ? GL_TRUE : GL_FALSE);
}

void GLframework::RenderState::applyPolygonOffsetState(const Material& material)
{
	if (material.getPolygonOffsetState())
	{
		glEnable(material.getPolygonOffsetType());
		glPolygonOffset(material.getFactor(), material.getUnit());
	}
	else
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
	}
}

void GLframework::RenderState::applyStencilState(const Material& material)
{
	if (material.getStencilState())
	{
		glEnable(GL_STENCIL_TEST);
		glStencilOp(material.getSFail(), material.getZFail(), material.getZPass());
		glStencilMask(material.getStencilMask());
		glStencilFunc(material.getStencilFunc(), material.getStencilRef(), material.getStencilFuncMask());
	}
	else
	{
		glDisable(GL_STENCIL_TEST);
	}
}

void GLframework::RenderState::applyColorBlendState(const Material& material)
{
	if (material.getColorBlendState())
	{
		glEnable(GL_BLEND);
		glBlendFunc(material.getSFactor(), material.getDFactor());
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void GLframework::RenderState::applyFaceCullingState(const Material& material)
{
	if (material.getFaceCullingState())
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(material.getFrontFace());
		glCullFace(material.getCullFace());
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

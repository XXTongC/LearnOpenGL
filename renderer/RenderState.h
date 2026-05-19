#pragma once

namespace GLframework
{
	class Material;

	namespace RenderState
	{
		void applyMaterialState(const Material& material);
		void applyDepthState(const Material& material);
		void applyPolygonOffsetState(const Material& material);
		void applyStencilState(const Material& material);
		void applyColorBlendState(const Material& material);
		void applyFaceCullingState(const Material& material);
	}
}

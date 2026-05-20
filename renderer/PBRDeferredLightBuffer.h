#pragma once

#include "core.h"
#include "renderer/MaterialBindingContext.h"

namespace GLframework
{
	struct PBRDeferredLightBufferStats
	{
		bool bound{ false };
		int pointLightCount{ 0 };
		int maxPointLightCount{ 0 };
		unsigned int bindingPoint{ 0 };
	};

	class PBRDeferredLightBuffer
	{
	public:
		~PBRDeferredLightBuffer();

		PBRDeferredLightBufferStats bind(const MaterialBindingContext& context);

		static constexpr int maxPointLights()
		{
			return 16;
		}

		static constexpr unsigned int bindingPoint()
		{
			return 3;
		}

	private:
		void ensureBuffer();

		unsigned int mBuffer{ 0 };
	};
}

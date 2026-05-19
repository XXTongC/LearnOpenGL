#pragma once

#include <memory>

#include "framebuffer/framebuffer.h"

namespace GLframework
{
	class PostProcessPass
	{
	public:
		void resolveMultisample(
			const std::shared_ptr<Framebuffer>& src,
			const std::shared_ptr<Framebuffer>& dst
		) const;
	};
}

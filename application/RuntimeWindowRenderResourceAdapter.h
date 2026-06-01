#pragma once

class Camera;

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState;

	class RuntimeWindowRenderResourceAdapter
	{
	public:
		static bool applyResize(
			RuntimeRenderResourceState& renderResources,
			Camera* camera,
			int* width,
			int* height,
			int newWidth,
			int newHeight
		);
	};
}

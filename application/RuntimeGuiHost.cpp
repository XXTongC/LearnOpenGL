#include "RuntimeGuiHost.h"

#include "../third_party/GLFW/glfw3.h"
#include "../third_party/imgui/imgui.h"
#include "../third_party/imgui/imgui_impl_glfw.h"
#include "../third_party/imgui/imgui_impl_opengl3.h"
#include "../tools/Logger/LogManager.h"
#include "RuntimeGuiHostTypes.h"
#include "RuntimeViewport.h"

namespace GL_RUNTIME
{
	void RuntimeGuiHost::initialize(const RuntimeGuiInitContext& context)
	{
		LogInfo("GUI Initializing...");
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(context.window, true);
		ImGui_ImplOpenGL3_Init(context.glslVersion);
		LogInfo("GUI Initialized");
	}

	void RuntimeGuiHost::renderFrame(const RuntimeGuiFrameContext& context)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (context.drawPanels)
		{
			context.drawPanels();
		}

		ImGui::Render();

		int displayWidth = 0;
		int displayHeight = 0;
		glfwGetFramebufferSize(context.window, &displayWidth, &displayHeight);
		RuntimeViewport::applyViewport(displayWidth, displayHeight);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

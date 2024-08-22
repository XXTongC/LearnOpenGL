#pragma once
#define GL_APP GL_APPLICATION::Application::getInstance()
#include <cstdint>
#include "texture.h"


extern GLframework::Texture* textureGrass;
extern GLframework::Texture* textureLand;
extern GLframework::Texture* textureNoise;
class GLFWwindow;

using KeyboardCallback = void(*) (int key, int action, int mods);
using ResizeCallback = void(*) (int width, int height);
using MouseCallback = void(*) (int button, int action, int mods);
using CursorCallback = void(*) (double xpos, double ypos);
using ScrollCallback = void(*) (double offset);

namespace GL_APPLICATION
{
	class Application
	{
	public:
		~Application();
		void test();
		void setKeyboardCallback(KeyboardCallback callback) { mKeyboardCallback = callback; }
		void setResizeCallback(ResizeCallback callback) { mResizeCallback = callback; }
		void setMouseCallback(MouseCallback callback) { mMouseCallback = callback; }
		void setCursorCallback(CursorCallback callback) { mCursorCallback = callback; }
		void setScrollCallback(ScrollCallback callback) { mScrollCallback = callback; }
		uint32_t getHeight() const { return mHeight; }
		uint32_t getWidth() const { return mWidth; }
		GLFWwindow* getWindow() const { return mWindow; }
		void getCursorPosition(double* x, double* y);
		static Application* getInstance();

		bool init(const int& width = 800, const int& height = 600);
		bool update();
		void destory();


	private:
		static Application* mInstance;
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void framebufferSizecallback(GLFWwindow* window, int width, int height);
		static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void cursorCallback(GLFWwindow* window, double xpos, double ypos);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		uint32_t mWidth{ 0 };
		uint32_t mHeight{ 0 };
		GLFWwindow* mWindow{ nullptr };
		MouseCallback mMouseCallback{ nullptr };
		ResizeCallback mResizeCallback{ nullptr };
		KeyboardCallback mKeyboardCallback{ nullptr };
		CursorCallback mCursorCallback{ nullptr };
		ScrollCallback mScrollCallback{ nullptr };
		Application();
		

	};
}

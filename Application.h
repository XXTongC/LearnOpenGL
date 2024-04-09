#pragma once
#define GL_APP GL_APPLICATION::Application::getInstance()
#include <cstdint>

class GLFWwindow;

using KeyboardCallback = void(*)(int key, int action, int mods);
using ResizeCallback = void(*)(int width, int height);
using MouseCallback = void(*) (int button, int action, int mods);

namespace GL_APPLICATION
{
	class Application
	{
	public:
		~Application();
		static Application* getInstance();
		GLFWwindow* getWindow() const { return mWindow; }
		void test();
		uint32_t getWidth() const { return mWidth; }
		void setKeyboardCallback(KeyboardCallback callback) { mKeyboardCallback = callback; }
		void setResizeCallback(ResizeCallback callback) { mResizeCallback = callback; }
		void setMouseCallback(MouseCallback callback) { mMouseCallback = callback; }
		uint32_t getHeight() const { return mHeight; }
		bool init(const int& width = 800,const int & height=600);
		bool update();
		void destory();
		

	private:
		static Application* mInstance;
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void framebufferSizecallback(GLFWwindow* window, int width, int height);
		static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
		uint32_t mWidth{ 0 };
		uint32_t mHeight{ 0 };
		GLFWwindow* mWindow{ nullptr };
		MouseCallback mMouseCallback{ nullptr };
		ResizeCallback mResizeCallback{ nullptr };
		KeyboardCallback mKeyboardCallback{ nullptr };
		Application();
		

	};
}

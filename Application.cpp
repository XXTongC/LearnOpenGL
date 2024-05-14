#include "Application.h"
#include "GL_ERROR_FIND.h"
#include "core.h"
#include <iostream>


//几个静态函数的实现
//初始化静态函数的实现
GL_APPLICATION::Application* GL_APPLICATION::Application::mInstance = nullptr;

void GL_APPLICATION::Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	std::cout << "Scroll" << std::endl;
	Application* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
	if(self->mScrollCallback!=nullptr)
	{
		self->mScrollCallback(yoffset);
	}
}


//窗口大小静态函数实现
void GL_APPLICATION::Application::framebufferSizecallback(GLFWwindow* window, int width, int height)
{
	std::cout << "Resize" << std::endl;
	Application* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
	if(self->mResizeCallback!=nullptr)
	{
		self->mResizeCallback(width, height);
		self->mWidth = width;
		self->mHeight = height;
	}
	
	/*if(Application::getInstance()->mResizeCallback!=nullptr)
		Application::getInstance()->mResizeCallback(width, height);*/
}
//键盘响应静态函数实现
void GL_APPLICATION::Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::cout << "KeyboardCallback" << std::endl;
	Application* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
	if (self->mKeyboardCallback != nullptr)
		self->mKeyboardCallback(key, action, mods);
}

//鼠标响应静态函数实现
void GL_APPLICATION::Application::mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	std::cout << "mouseCallback" << std::endl;
	Application* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
	if (self->mMouseCallback != nullptr)
		self->mMouseCallback(button, action, mods);
}

//位置响应静态函数实现
void GL_APPLICATION::Application::cursorCallback(GLFWwindow* window, double xpos, double ypos)
{
	//std::cout << "cursorCallback" << std::endl;
	Application* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
	if (self->mCursorCallback != nullptr)
		self->mCursorCallback(xpos, ypos);
}


GL_APPLICATION::Application* GL_APPLICATION::Application::getInstance()
{
	if (mInstance == nullptr)
		mInstance = new Application();
	return mInstance;
}

void GL_APPLICATION::Application::test()
{
	std::cout << "APP TEST" << std::endl;
}

void GL_APPLICATION::Application::getCursorPosition(double* x, double* y)
{
	glfwGetCursorPos(mWindow, x, y);
}

GL_APPLICATION::Application::Application()
{
	
}



GL_APPLICATION::Application::~Application()
{
	
}
bool GL_APPLICATION::Application::init(const int& width, const int& height)
{
	mWidth = width, mHeight = height;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	mWindow = glfwCreateWindow(mWidth,mHeight , "MyFirstWindow", NULL, NULL);
	if (mWindow == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(mWindow);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	//窗口大小变化响应

	glfwSetFramebufferSizeCallback(mWindow, framebufferSizecallback);
	glfwSetWindowUserPointer(mWindow, this);	//将此时的this指针存到mWindow对象里
	//键盘响应
	glfwSetKeyCallback(mWindow, keyCallback);
	glfwSetMouseButtonCallback(mWindow, mouseCallback);
	glfwSetCursorPosCallback(mWindow, cursorCallback);
	glfwSetScrollCallback(mWindow, scrollCallback);
	return true;
}
bool GL_APPLICATION::Application::update()
{
	if (glfwWindowShouldClose(mWindow))
		return false;
	glfwPollEvents();
	glfwSwapBuffers(mWindow);

	return true;
}
void GL_APPLICATION::Application::destory()
{
	glfwTerminate();
}



#include "core.h"
#include "GL_ERROR_FIND.h"


void GL_ERROR_FIND::findError()
{
	GLenum errorCode = glGetError();
	std::string errorInfo = "NO_ERROR";
	if (errorCode != GL_NO_ERROR)
	{
		switch (errorCode)
		{
		case GL_INVALID_ENUM:
			errorInfo = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			errorInfo = "GL_INVALID_VAULE";
			break;
		case GL_INVALID_INDEX:
			errorInfo = "GL_INVALID_INDEX";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			errorInfo = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_INVALID_OPERATION:
			errorInfo = "GL_INVALID_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			errorInfo = "GL_OUT_OF_MEMORY";
			
			break;
		default:
			errorInfo = "UNKNOWN_ERROR";
			break;
		}
		std::cout << errorInfo << std::endl;
		assert(false);
	}
}

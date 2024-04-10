#pragma once
#include "core.h"
#include <string>
namespace GLframework
{
	class Shader
	{
	public:
		Shader(const char* vertexPath,const char* fragmentPath);
		~Shader();
		GLuint getProgram() const;
		void setFloat(const std::string& name, float value);
		void setInt(const std::string& name, int value);
		void setVector3(const std::string& name, float x, float y, float z);
		void setVector3(const std::string& name, const float* values);

		void begin();	//开始使用当前shader
		void end();		//解除当前shader
	private:
		GLuint mProgram{ 0 };
		void checkShaderErrors(GLuint target, const std::string& type);
	};
}

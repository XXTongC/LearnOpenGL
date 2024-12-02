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
		void setVector3(const std::string& name, glm::vec3 vec);
		void setMat4(const std::string& name, const glm::mat4 values);
		void setMat4Array(const std::string& name, const glm::mat4 values[],int count);
		void setMat3(const std::string& name, const glm::mat3 values);
		void begin();	
		void end();		
	private:
		GLuint mProgram{ 0 };
		void checkShaderErrors(GLuint target, const std::string& type);
	};
}

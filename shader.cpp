#include "shader.h"
#include "GL_ERROR_FIND.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::string GLframework::Shader::loadShader(const std::string& filePath)
{
	std::ifstream file(filePath);
	std::stringstream shaderStream;
	std::string line;
	while (std::getline(file,line))
	{
		//	1 find if there is a "#include"
		if(line.find("#include")!=std::string::npos)
		{
			//find the path of include
			auto start = line.find("\"");
			auto end = line.find_last_of("\"");
			std::string includeFile = line.substr(start+1, end - start-1);

			//find current file path
			auto lastSlashPosition = filePath.find_last_of("/\\");
			auto folder = filePath.substr(0, lastSlashPosition + 1);
			auto totalPath = folder + includeFile;

			shaderStream<<loadShader(totalPath);
		}else
		{
			shaderStream << line << "\n";
		}
	}

	return shaderStream.str();


}

void GLframework::Shader::setMat4Array(const std::string& name, const glm::mat4 values[], int count)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniformMatrix4fv(location,count,GL_FALSE,glm::value_ptr(values[0])));
}

void GLframework::Shader::setFloatArray(const std::string& name, float* value, int count)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniform1fv(location, count, value));
}



void GLframework::Shader::setVector3(const std::string& name, glm::vec3 vec)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniform3f(location, vec.x,vec.y,vec.z));
}

void GLframework::Shader::setMat4(const std::string& name, const glm::mat4 values)
{
	GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniformMatrix4fv(location, 1,GL_FALSE, glm::value_ptr(values)));
}

void GLframework::Shader::setMat3(const std::string& name, const glm::mat3 values)
{
	GLint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(values)));
}


void GLframework::Shader::setVector3(const std::string& name, const float* values)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	//第二个参数表示有几个vec3
	GL_CALL(glUniform3fv(location, 1,values));
}


void GLframework::Shader::setVector3(const std::string& name, float x, float y, float z)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniform3f(location,x, y, z));
}


void GLframework::Shader::setInt(const std::string& name, int value)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniform1i(location, value));
}


void GLframework::Shader::setFloat(const std::string& name, float value)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniform1f(location, value));
	//std::cout << value << std::endl;
}


GLuint GLframework::Shader::getProgram() const
{
	return mProgram;
}

GLframework::Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	//声明装入shader代码字符串的两个string
	std::string vertexCode;
	std::string fragmentCode;

	try
	{
		vertexCode = loadShader(vertexPath);
		fragmentCode = loadShader(fragmentPath);
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR: Shader File Error: " << e.what() << std::endl;
	}
	
	const char* vertexShaderSource = vertexCode.c_str();
	const char* fragmentShaderSource = fragmentCode.c_str();

	//创建和初始化两个shader
	GLuint vertexShader{ 0 }, fragmentShader{ 0 };
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	

	//将shader代码（glgf代码）注入shader中
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

	
	//执行shader代码编译
	glCompileShader(vertexShader);
	checkShaderErrors(vertexShader, "COMPILE");
	//检查vertex编译结果
	glCompileShader(fragmentShader);
	checkShaderErrors(fragmentShader, "COMPILE");


	//创建容器
	//GLuint program{ 0 };
	mProgram = glCreateProgram();
	//将vs与fs放置于容器中
	glAttachShader(mProgram, vertexShader);
	glAttachShader(mProgram, fragmentShader);

	//执行容器的链接，最终使得shader程序可运行
	glLinkProgram(mProgram);
	//检查链接结果
	checkShaderErrors(mProgram, "LINK");

	//此时vertexShader和fragmentShader已经失去作用，需要销毁
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//启用混合模式
	GL_CALL(glEnable(GL_BLEND));
}

GLframework::Shader::~Shader()
{
	
}


void GLframework::Shader::begin()
{
	GL_CALL(glUseProgram(mProgram));
}

void GLframework::Shader::end()
{
	GL_CALL(glUseProgram(0));
}


void GLframework::Shader::checkShaderErrors(GLuint target, const std::string& type)
{
	int success{ 0 };
	char infoLog[1024];
	if(type == "COMPILE")
	{
		glGetShaderiv(target, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(target, 1024, NULL, infoLog);
			std::cout << "Shader Compile Error("<<type<< "): " << infoLog << std::endl;
		}
	}
	else if(type=="LINK")
	{
		glGetProgramiv(target, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(target, 1024, NULL, infoLog);
			std::cout << "Shader Link Error(" << type << "): " << infoLog << std::endl;
		}
	}
	else
	{
		std::cout << "Error: Check shader errors Type is wrong" << std::endl;
	}
}




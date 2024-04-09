#include "shader.h"
#include "GL_ERROR_FIND.h"
#include <fstream>
#include <sstream>
#include <iostream>

void GLShaderwork::Shader::setVector3(const std::string& name, const float* values)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	//第二个参数表示有几个vec3
	GL_CALL(glUniform3fv(location, 1,values));
}


void GLShaderwork::Shader::setVector3(const std::string& name, float x, float y, float z)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniform3f(location,x, y, z));
}


void GLShaderwork::Shader::setInt(const std::string& name, int value)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniform1i(location, value));
}


void GLShaderwork::Shader::setFloat(const std::string& name, float value)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	GL_CALL(glUniform1f(location, value));
	//std::cout << value << std::endl;
}


GLuint GLShaderwork::Shader::getProgram() const
{
	return mProgram;
}

GLShaderwork::Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	//声明装入shader代码字符串的两个string
	std::string vertexCode;
	std::string fragmentCode;
	//声明用于读取vs和fs文件的inFileStream
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	//保证ifstream遇到问题时抛出异常
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		//1 打开文件
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);

		//2 将文件输入流当中 的字符串输入到stringstream里面
		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		//3 关闭文件
		vShaderFile.close();
		fShaderFile.close();

		//4 将字符串从stringStream当中读取出来，转入到code String中
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

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

GLShaderwork::Shader::~Shader()
{
	
}


void GLShaderwork::Shader::begin()
{
	GL_CALL(glUseProgram(mProgram));
}

void GLShaderwork::Shader::end()
{
	GL_CALL(glUseProgram(0));
}


void GLShaderwork::Shader::checkShaderErrors(GLuint target, const std::string& type)
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




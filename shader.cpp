#include "shader.h"
#include "GL_ERROR_FIND.h"
#include <fstream>
#include <sstream>
#include <iostream>

void GLShaderwork::Shader::setVector3(const std::string& name, const float* values)
{
	GLuint location = GL_CALL(glGetUniformLocation(mProgram, name.c_str()));
	//�ڶ���������ʾ�м���vec3
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
	//����װ��shader�����ַ���������string
	std::string vertexCode;
	std::string fragmentCode;
	//�������ڶ�ȡvs��fs�ļ���inFileStream
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	//��֤ifstream��������ʱ�׳��쳣
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		//1 ���ļ�
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);

		//2 ���ļ����������� ���ַ������뵽stringstream����
		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		//3 �ر��ļ�
		vShaderFile.close();
		fShaderFile.close();

		//4 ���ַ�����stringStream���ж�ȡ������ת�뵽code String��
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR: Shader File Error: " << e.what() << std::endl;
	}
	
	const char* vertexShaderSource = vertexCode.c_str();
	const char* fragmentShaderSource = fragmentCode.c_str();

	//�����ͳ�ʼ������shader
	GLuint vertexShader{ 0 }, fragmentShader{ 0 };
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	

	//��shader���루glgf���룩ע��shader��
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

	
	//ִ��shader�������
	glCompileShader(vertexShader);
	checkShaderErrors(vertexShader, "COMPILE");
	//���vertex������
	glCompileShader(fragmentShader);
	checkShaderErrors(fragmentShader, "COMPILE");


	//��������
	//GLuint program{ 0 };
	mProgram = glCreateProgram();
	//��vs��fs������������
	glAttachShader(mProgram, vertexShader);
	glAttachShader(mProgram, fragmentShader);

	//ִ�����������ӣ�����ʹ��shader���������
	glLinkProgram(mProgram);
	//������ӽ��
	checkShaderErrors(mProgram, "LINK");

	//��ʱvertexShader��fragmentShader�Ѿ�ʧȥ���ã���Ҫ����
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//���û��ģʽ
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




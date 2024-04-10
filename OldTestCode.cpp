#include "OldTestCode.h"

#include "stb_image.h"

void OnResize(int width, int height)
{
	GL_CALL(glViewport(0, 0, width, height));
	std::cout << "OnResize" << std::endl;
}

void OnKeyboardCallback(int key, int action, int mods)
{
	GL_CALL();
	std::cout << "OnKeyboardCallback Pressed: " << key << " " << action << " " << mods << std::endl;
}

void OnMouseCallback(int button, int action, int mods)
{
	GL_CALL();
	std::cout << "OnMouseCallback : " << button << " " << action << " " << mods << std::endl;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
		std::cout << "press the bottom" << std::endl;
	else if (action == GLFW_RELEASE)
		std::cout << "release the bottom" << std::endl;
	if (mods == GLFW_MOD_CONTROL)
		std::cout << "press the ctrl and key" << std::endl;
	std::cout << "Pressed: " << key << std::endl;
	std::cout << "Action: " << action << std::endl;
	std::cout << "Mods: " << mods << std::endl;
}

void prepareVBO()
{
	float position[]
	{
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.0f, 0.5f,
	};
	float color[]
	{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};

	float verticesAll[]
	{
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f,
	};

	//����һ��VBO
	GLuint posVbo{ 0 }, colorVbo{ 0 }, vboAllInOne{ 0 };
	GLuint vao{ 0 };
	//�˴���������һ��vbo��posVbo��ֵΪ��vbo��ID
	GL_CALL(glGenBuffers(1, &posVbo));
	//�󶨵�ǰ��vbo��opengl״̬���ĵ�ǰvbo�����
	//GL_ARRAY_BUFFER��ʾ��ǰvbo�Ĳ��
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posVbo));
	//����ռ䲢�������
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW));

	//��ɫvbo
	GL_CALL(glGenBuffers(1, &colorVbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorVbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW));


	//�ಢһ
	GL_CALL(glGenBuffers(1, &vboAllInOne));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vboAllInOne));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(verticesAll), verticesAll, GL_STATIC_DRAW));

	//����VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorVbo));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));


	GL_CALL(glDeleteBuffers(1, &vboAllInOne));
	GL_CALL(glDeleteBuffers(1, &posVbo));
	GL_CALL(glDeleteBuffers(1, &colorVbo));
}

void prepareSingleBuffer()
{
	//1 ׼��position colors ����
	float position[]
	{
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.0f, 0.5f,
	};
	float color[]
	{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};

	//2 ʹ��������������vbo posVbo, colorVbo
	GLuint posVbo{ 0 }, colorVbo{ 0 };
	glGenBuffers(1, &posVbo);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

	glGenBuffers(1, &colorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);

	//3 ����vao����
	GLuint myVao{ 0 };
	glGenVertexArrays(1, &myVao);
	glBindVertexArray(myVao);

	//4 �ֱ�λ��/��ɫ���Ե�������Ϣ����vao
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	//������ҪVAOʱ�������󶨣��Է�֮������������VAO������
	glBindVertexArray(0);
}

void prepareInterleaveBuffer()
{
	float vertex[]{
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f,
	};

	GLuint vbo{ 0 };

	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW));

	//GLuint vao{ 0 };
	GL_CALL(glGenVertexArrays(1, &vao));
	GL_CALL(glBindVertexArray(vao));

	//�󶨶�����Ϣ
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(0)));

	//�󶨶�����ɫ��Ϣ
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))));

	GL_CALL(glBindVertexArray(0));
}

void prepareTest1()
{
	float vertex[]
	{
		0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.8f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.8f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.8f, 0.0f, 0.5f,
	};

	GLuint vbo{ 0 };
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW));

	GLuint vao{ 0 };
	GL_CALL(glGenVertexArrays(1, &vao));
	GL_CALL(glBindVertexArray(vao));
	//��λ����Ϣ
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), static_cast<void*>(0)));
	//����ɫ��Ϣ
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float))));
	//��uv��Ϣ
	GL_CALL(glEnableVertexAttribArray(2));
	GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float))));

	//���vao��
	GL_CALL(glBindVertexArray(0));
}

void prepareTest2()
{
	float vertex_pos_uv[]
	{
		0.5f, 0.0f, 0.0f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.5f,
	};
	float vertex_color[]
	{
		1.0f, 0.0f, 0.0f, 0.8f,
		0.0f, 1.0f, 0.0f, 0.8f,
		0.0f, 0.0f, 1.0f, 0.8f,
	};
	GLuint pos_uv_vbo{ 0 }, color_vbo{ 0 };

	//����pos_uv_vbo
	GL_CALL(glGenBuffers(1, &pos_uv_vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pos_uv_vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_pos_uv), vertex_pos_uv, GL_STATIC_DRAW));

	//����color_vbo
	GL_CALL(glGenBuffers(1, &color_vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, color_vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_color), vertex_color, GL_STATIC_DRAW));

	//����vao���󶨲��
	GLuint vao{ 0 };
	GL_CALL(glGenVertexArrays(1, &vao));
	GL_CALL(glBindVertexArray(vao));
	//�Ȱ�pos_uv��Ϣ
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pos_uv_vbo));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(0)));
	GL_CALL(glEnableVertexAttribArray(2));
	GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
	//��color��Ϣ
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, color_vbo));
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(0)));

	//���vao��
	GL_CALL(glBindVertexArray(0));
}


void prepareVAOForGLTrangles()
{
	float vertex[]{
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 0.0f, 1.0f, 0.5f, 0.25f,
		0.8f, 0.8f, 0.0f, 0.5f, 0.2f, 1.0f,
		0.8f, 0.0f, 0.0f, 0.5f, 0.2f, 1.0f,
	};

	//����VBO
	GLuint vbo{ 0 };
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	//����VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr));

	glBindVertexArray(0);
}

void prepareEBOForGLTrangles()
{
	float vertex[]
	{
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};
	int indexes[]
	{
		0, 1, 2,
		0, 1, 3,
		1, 2, 3,
	};

	GLuint vbo{ 0 };

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	GLuint ebo{ 0 };
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);
}
void prepareColorGLTrangles()
{
	float vertex[]{
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
	};

	int indexes[]
	{
		0, 1, 2,
	};
	GLuint ebo{ 0 }, vbo{ 0 };
	//�ȷ����vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//װ��vbo����
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);


	//��̬��ȡλ��
	GLuint posLocation = glGetAttribLocation(shader->getProgram(), "aPos");
	GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");

	//��λ���Լ���ɫ��Ϣ
	glEnableVertexAttribArray(posLocation);
	glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(colorLocation);
	glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void prepareUVGLTranglesTest1()
{
	float vertex[]
	{
		-1.0f,-1.0f,0.0f,0.0f,0.0f,
		-1.0f,1.0f,0.0f,0.0f,10.0f,
		1.0f,1.0f,0.0f,10.0f,10.0f,
		1.0f,-1.0f,0.0f,10.0f,0.0f,
	};
	int index[]
	{
		0,1,2,
		0,2,3,
	};

	GLuint vbo{ 0 }, uv{ 0 };
	//�Ƚ�vao��
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//��ʼ��vbo��ע�����ݵȲ���
	GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
	GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(positionLocation);
	glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(uvLocation);
	glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glGenBuffers(1, &uv);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uv);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void prepareTextureTest1()
{
	//����ͼƬ������ɫͨ��
	int width, height, channels;
	//��תy��
	stbi_set_flip_vertically_on_load(true);
	//��ȡͼƬ����
	unsigned char* data = stbi_load("Texture/panda.jpg", &width, &height, &channels, STBI_rgb_alpha);

	//��������
	glGenTextures(1, &Texture);
	//��������Ԫ
	glActiveTexture(GL_TEXTURE0);
	//���������
	glBindTexture(GL_TEXTURE_2D, Texture);
	//�����Դ沢������������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	//�ͷ�RAM�ϵ�data
	stbi_image_free(data);

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//�����Ⱦ����С��ԭͼ����ͽ�ԭ�򣬳�ͼ����������ݸ�ǿ
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 �������������ʽ������x, y����ĳ�ԭͼ��Χ����, S��Ӧx(u), T��Ӧy(v)
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));


}

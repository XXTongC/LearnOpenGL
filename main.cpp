#include "core.h"
#include <iostream>
#include "GL_ERROR_FIND.h"
#include "Application.h"
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void OnResize(int width, int height);
void OnKeyboardCallback(int key, int action, int mods);
void OnMouseCallback(int button, int action, int mods);
//以下皆为vao与vbo的创建与绑定
void prepareVBO();
void prepareSingleBuffer();
void prepareInterleaveBuffer();
void prepareTest1();
void prepareTest2();
void prepareVAOForGLTrangles();
void prepareEBOForGLTrangles();
void prepareColorGLTrangles();
//shader的创建
void prepareShader();
//绘制命令
void render();
//纹理创建
void prepareTexture();


//声明全局变量vao以及shaderProram
GLuint vao;
GLShaderwork::Shader* shader = nullptr;

int main()
{
	if (!GL_APP->init(640, 480)) return -1;
	GL_APP->setResizeCallback(OnResize);
	GL_APP->setKeyboardCallback(OnKeyboardCallback);
	GL_APP->setMouseCallback(OnMouseCallback);
	prepareShader();
	prepareColorGLTrangles();
	prepareTexture();
	GL_CALL(glViewport(0, 0, 640, 480));
	GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
	while (GL_APP->update())
	{
		render();
	}

	GL_APP->destory();
	return 0;
}

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

	//生成一个VBO
	GLuint posVbo{0}, colorVbo{0}, vboAllInOne{0};
	GLuint vao{0};
	//此处才是生成一个vbo，posVbo的值为其vbo的ID
	GL_CALL(glGenBuffers(1, &posVbo));
	//绑定当前的vbo到opengl状态机的当前vbo插槽上
	//GL_ARRAY_BUFFER表示当前vbo的插槽
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posVbo));
	//申请空间并填充数据
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(position), position,GL_STATIC_DRAW));

	//颜色vbo
	GL_CALL(glGenBuffers(1, &colorVbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorVbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW));


	//多并一
	GL_CALL(glGenBuffers(1, &vboAllInOne));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vboAllInOne));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(verticesAll), verticesAll, GL_STATIC_DRAW));

	//创建VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorVbo));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));


	GL_CALL(glDeleteBuffers(1, &vboAllInOne));
	GL_CALL(glDeleteBuffers(1, &posVbo));
	GL_CALL(glDeleteBuffers(1, &colorVbo));
}

void prepareSingleBuffer()
{
	//1 准备position colors 数据
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

	//2 使用数据生成两个vbo posVbo, colorVbo
	GLuint posVbo{0}, colorVbo{0};
	glGenBuffers(1, &posVbo);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

	glGenBuffers(1, &colorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);

	//3 生成vao并绑定
	GLuint myVao{0};
	glGenVertexArrays(1, &myVao);
	glBindVertexArray(myVao);

	//4 分别将位置/颜色属性的描述信息加入vao
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	//将不需要VAO时将其解除绑定，以防之后意外更改这个VAO的属性
	glBindVertexArray(0);
}

void prepareInterleaveBuffer()
{
	float vertex[]{
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f,
	};

	GLuint vbo{0};

	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW));

	//GLuint vao{ 0 };
	GL_CALL(glGenVertexArrays(1, &vao));
	GL_CALL(glBindVertexArray(vao));

	//绑定顶点信息
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(0)));

	//绑定顶点颜色信息
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

	GLuint vbo{0};
	GL_CALL(glGenBuffers(1,&vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW));

	GLuint vao{0};
	GL_CALL(glGenVertexArrays(1,&vao));
	GL_CALL(glBindVertexArray(vao));
	//绑定位置信息
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), static_cast<void*>(0)));
	//绑定颜色信息
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float))));
	//绑定uv信息
	GL_CALL(glEnableVertexAttribArray(2));
	GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float))));

	//解除vao绑定
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
	GLuint pos_uv_vbo{0}, color_vbo{0};

	//创建pos_uv_vbo
	GL_CALL(glGenBuffers(1,&pos_uv_vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pos_uv_vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_pos_uv), vertex_pos_uv, GL_STATIC_DRAW));

	//创建color_vbo
	GL_CALL(glGenBuffers(1, &color_vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, color_vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_color), vertex_color, GL_STATIC_DRAW));

	//创建vao并绑定插槽
	GLuint vao{0};
	GL_CALL(glGenVertexArrays(1, &vao));
	GL_CALL(glBindVertexArray(vao));
	//先绑定pos_uv信息
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pos_uv_vbo));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(0)));
	GL_CALL(glEnableVertexAttribArray(2));
	GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
	//绑定color信息
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER,color_vbo));
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(0)));

	//解除vao绑定
	GL_CALL(glBindVertexArray(0));
}

void prepareShader()
{
	shader = new GLShaderwork::Shader("vertexAuto.glsl", "fragmentAuto.glsl");
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

	//创建VBO
	GLuint vbo{0};
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	//创建VAO
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

	GLuint vbo{0};

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	GLuint ebo{0};
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);
}

void render()
{
	/*
	* glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	* 这里一定不能加上这句命令，会报错，以下解释：
	* 在第一帧的时候我们将绑定于GL状态机的ebo解绑，但是状态机仍然能通过vao获取ebo信息
	* 所以第一帧可以正常绘制图片
	* 但到了第二帧，我们将vao中的ebo也解绑了，使得第二帧无法获取到有效的ebo而报错
	*/
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	//1 绑定当前program
	shader->begin();
	//2 绑定当前vao
	glBindVertexArray(vao);
	//发出绘制指令
	//GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
	//GL_TRIANGLE_STRIP 如果末尾点序号n为偶数，则链接规则为[n-2,n-1,n]
	//奇数为[n-1,n-2,n]
	//保证了顺时针或逆时针的链接顺序相同
	//GL_TRIANGLE_FAN 绘制以V0为起点的扇形
	//GL_CALL(glDrawArrays(GL_SMOOTH_LINE_WIDTH_GRANULARITY, 0, 1));

	GL_CALL(glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT,static_cast<void*>(0)));
	float value[]{0.5, 0.1, 0.78};
	shader->setFloat("time", glfwGetTime());
	shader->setFloat("speed", 5);
	//shader->setVector3("uColor", 1.0f, 0.7f, 0.25f);
	shader->setVector3("uColor", value);
	//但如果加上这条指令，那么我们上面所说的错误就可以避免
	//这提示了一个在OpenGL中健康的代码习惯：有开有关
	GL_CALL(glBindVertexArray(0));
	shader->end();
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
	GLuint ebo{0}, vbo{0};
	//先分配好vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//装配vbo数据
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

	//动态获取位置
	GLuint posLocation = glGetAttribLocation(shader->getProgram(), "aPos");
	GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");

	//绑定位置以及颜色信息
	glEnableVertexAttribArray(posLocation);
	glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(colorLocation);
	glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void prepareTexture()
{
	//1 读取stbImage 
	int width, height, channels;

	//反转Y轴，原因是图片的存储是从左上角开始，而OpenGL的图片读取是从左下角开始
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("Texture\panda.jpg",)
	//2 生成纹理并且激活单元绑定

	//3 传输纹理数据
}
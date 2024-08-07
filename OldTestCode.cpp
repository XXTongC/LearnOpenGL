#include "OldTestCode.h"

#include "stb_image.h"

/*
void prepareUVGLTranglesTest2()
{
	
float vertex[]
{
	-1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,
	1.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,
	0.0f,1.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,
};
int index[]
{
	0,1,2,
};

GLuint vbo{ 0 }, ebo{ 0 };
//先将vao绑定
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);

//开始绑定vbo并注入数据等操作

GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");
GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");

glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

glEnableVertexAttribArray(positionLocation);
glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
glEnableVertexAttribArray(colorLocation);
glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(uvLocation);
glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

glGenBuffers(1, &ebo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

glBindVertexArray(0);
}
*/
/*
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
	GLuint posVbo{ 0 }, colorVbo{ 0 }, vboAllInOne{ 0 };
	GLuint vao{ 0 };
	//此处才是生成一个vbo，posVbo的值为其vbo的ID
	GL_CALL(glGenBuffers(1, &posVbo));
	//绑定当前的vbo到opengl状态机的当前vbo插槽上
	//GL_ARRAY_BUFFER表示当前vbo的插槽
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posVbo));
	//申请空间并填充数据
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW));

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
	GLuint posVbo{ 0 }, colorVbo{ 0 };
	glGenBuffers(1, &posVbo);
	glBindBuffer(GL_ARRAY_BUFFER, posVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

	glGenBuffers(1, &colorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);

	//3 生成vao并绑定
	GLuint myVao{ 0 };
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

	GLuint vbo{ 0 };

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

	GLuint vbo{ 0 };
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW));

	GLuint vao{ 0 };
	GL_CALL(glGenVertexArrays(1, &vao));
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
	GLuint pos_uv_vbo{ 0 }, color_vbo{ 0 };

	//创建pos_uv_vbo
	GL_CALL(glGenBuffers(1, &pos_uv_vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pos_uv_vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_pos_uv), vertex_pos_uv, GL_STATIC_DRAW));

	//创建color_vbo
	GL_CALL(glGenBuffers(1, &color_vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, color_vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_color), vertex_color, GL_STATIC_DRAW));

	//创建vao并绑定插槽
	GLuint vao{ 0 };
	GL_CALL(glGenVertexArrays(1, &vao));
	GL_CALL(glBindVertexArray(vao));
	//先绑定pos_uv信息
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pos_uv_vbo));
	GL_CALL(glEnableVertexAttribArray(0));
	GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(0)));
	GL_CALL(glEnableVertexAttribArray(2));
	GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
	//绑定color信息
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, color_vbo));
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), static_cast<void*>(0)));

	//解除vao绑定
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

	//创建VBO
	GLuint vbo{ 0 };
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
	//先将vao绑定
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//开始绑定vbo并注入数据等操作
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


void doRotateTransform()
{
	//第二个参数需要传入的应该是弧度制
	transform = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3{ 0.0f,0.0f,1.0f });
}

void doTranslationTransform()
{
	transform = glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, 0.3f, 0.0f));
}

void doScaleTransform()
{
	transform = glm::scale(glm::mat4(1.0f), glm::vec3{ 0.5f,0.5f,1.0f });
}

void doTransform()
{
	auto transformRotate = glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(glfwGetTime()) * 10), glm::vec3{ 0.0f,0.0f,1.0f });
	auto transformTranslate = glm::translate(glm::mat4(1.0f), glm::vec3{ 0.5f,0.5f,0.0f });
	auto transformScale = glm::scale(glm::mat4(1.0f), glm::vec3{ 0.5f,0.5f,1.0f });
	transform = transformRotate * transformScale * transformTranslate;
}

void doRotate()
{
	//每帧重新构建旋转矩阵
	angle += 0.01f;
	transform = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3{ 0.0f,0.0f,1.0f });
}

void preTransform()
{
	transform = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3{ 0.0f,0.0f,1.0f });

}

void doTestTransform()
{
	//目标一：旋转的三角形S
	//transform = glm::rotate(transform, glm::radians(0.1f),glm::vec3{0.0f,0.0f,1.0f});

	//目标二：先平移后旋转
	//transform = glm::translate(transform, glm::vec3{ 0.0001f,0.0f,0.0f });

	//目标三：先旋转后平移
}
*/
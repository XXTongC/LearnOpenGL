#include "core.h"
#include <iostream>
#include "GL_ERROR_FIND.h"
#include "Application.h"
#include "shader.h"
#include "OldTestCode.h"
#include "texture.h"

/*
 * to make main.cpp clear
 * if u need the code belong to past classes
 * check file "OldTestCode.h/.cpp"
 *
*/


//vao、vbo等设置

void prepareUVGLTranglesTest2();
//shader的创建
void prepareShader();
//绘制命令
void render();
//纹理创建
void prepareTexture();

//TextureTest 1 纹理平铺



//声明全局变量vao以及shaderProram
GLuint vao;
GLframework::Shader* shader = nullptr;
GLframework::Texture* textureGrass = nullptr;
GLframework::Texture* textureLand = nullptr;
GLframework::Texture* textureNoise = nullptr;

int main()
{
	if (!GL_APP->init(640, 480)) return -1;
	GL_APP->setResizeCallback(OnResize);
	GL_APP->setKeyboardCallback(OnKeyboardCallback);
	GL_APP->setMouseCallback(OnMouseCallback);
	prepareShader();
	prepareUVGLTranglesTest2();
	prepareTexture();
	GL_CALL(glViewport(0, 0, 640, 480));
	GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
	while (GL_APP->update())
	{
		render();
	}

	GL_APP->destory();
	delete textureGrass;
	delete textureLand;

	return 0;
}

void prepareShader()
{
	shader = new GLframework::Shader("vertexAuto.glsl", "fragmentAuto.glsl");
}

void prepareTexture()
{
	textureGrass = new GLframework::Texture("Texture/grass.jpg", 0);
	textureLand = new GLframework::Texture("Texture/land.jpg", 1);
	textureLand = new GLframework::Texture("Texture/noise.jpg", 2);

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
	//设置shader的采样器为0号采样器
	shader->setInt("samplerGrass", 0);
	shader->setInt("samplerLand", 1);
	shader->setInt("samplerNoise", 2);

	shader->setFloat("time", glfwGetTime());
	shader->setFloat("speed", 0.5);
	//2 绑定当前vao

	GL_CALL(glBindVertexArray(vao));
	//发出绘制指令
	//GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 3));
	//GL_TRIANGLE_STRIP 如果末尾点序号n为偶数，则链接规则为[n-2,n-1,n]
	//奇数为[n-1,n-2,n]
	//保证了顺时针或逆时针的链接顺序相同
	//GL_TRIANGLE_FAN 绘制以V0为起点的扇形
	//GL_CALL(glDrawArrays(GL_SMOOTH_LINE_WIDTH_GRANULARITY, 0, 1));

	GL_CALL(glDrawElements(GL_TRIANGLES,6 , GL_UNSIGNED_INT,static_cast<void*>(0)));
	
	
	//但如果加上下面这条指令，那么我们上面所说的错误就可以避免
	//这提示了一个在OpenGL中健康的代码习惯：有开有关
	GL_CALL(glBindVertexArray(0));
	shader->end();
}

void prepareUVGLTranglesTest2()
{
	float vertex[]
{
		-1.0f,-1.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,
		-1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,
		1.0f,1.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f,
		1.0f,-1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,
	};
	int index[]
	{
		0,1,2,
		0,2,3,
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

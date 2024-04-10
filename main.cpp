#include "core.h"
#include <iostream>
#include "GL_ERROR_FIND.h"
#include "Application.h"
#include "shader.h"
#include "OldTestCode.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


/*
 * to make main.cpp clear
 * if u need the code belong to past classes
 * check file "OldTestCode.h/.cpp"
 *
*/


//vao、vbo等设置

void prepareUVGLTranglesTest2();
void prepareTextureTest2();
//shader的创建
void prepareShader();
//绘制命令
void render();
//纹理创建
void prepareTexture();

//TextureTest 1 纹理平铺



//声明全局变量vao以及shaderProram
GLuint vao,Texture;
GLShaderwork::Shader* shader = nullptr;

int main()
{
	if (!GL_APP->init(640, 480)) return -1;
	GL_APP->setResizeCallback(OnResize);
	GL_APP->setKeyboardCallback(OnKeyboardCallback);
	GL_APP->setMouseCallback(OnMouseCallback);
	prepareShader();
	prepareUVGLTranglesTest2();
	prepareTextureTest2();
	GL_CALL(glViewport(0, 0, 640, 480));
	GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
	while (GL_APP->update())
	{
		render();
	}

	GL_APP->destory();
	return 0;
}

void prepareShader()
{
	shader = new GLShaderwork::Shader("vertexAuto.glsl", "fragmentAuto.glsl");
}

void prepareUVGLTrangles()
{
	float vertex[]{
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,0.0f,0.0f,
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,0.5f,0.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f,0.25f,1.0f,
	};
	int indexes[]
	{
		0, 1, 2,
	};
	//先绑定好vao
	GL_CALL(glGenVertexArrays(1, &vao));
	GL_CALL(glBindVertexArray(vao));
	//装配vbo
	GLuint vbo{ 0 };
	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW));

	//动态获取相关location
	GLuint positionLocation = glGetAttribLocation(shader->getProgram(), "aPos");
	GLuint colorLocation = glGetAttribLocation(shader->getProgram(), "aColor");
	GLuint uvLocation = glGetAttribLocation(shader->getProgram(), "aUV");

	//激活vao插槽，并绑定装载数据
	GL_CALL(glEnableVertexAttribArray(positionLocation));
	GL_CALL(glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0));
	GL_CALL((glEnableVertexAttribArray(colorLocation)));
	GL_CALL(glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))));
	GL_CALL(glEnableVertexAttribArray(uvLocation));
	GL_CALL(glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))));

	//设置ebo, 在当前以绑定vao的情况下绑定ebo时ebo会自动绑定至改vao
	GLuint ebo{ 0 };
	GL_CALL(glGenBuffers(1, &ebo));
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), indexes, GL_STATIC_DRAW));

	//解绑vao
	GL_CALL(glBindVertexArray(0));
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
	shader->setInt("sampler", 0);
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

void prepareTexture()
{
	//1 读取stbImage 
	int width, height, channels;

	//反转Y轴，原因是图片的存储是从左上角开始，而OpenGL的图片读取是从左下角开始
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("Texture/panda.jpg", &width, &height, &channels, STBI_rgb_alpha);
	
	if (data == nullptr) std::cout << "nullptr" << std::endl;
	//2 生成纹理并且激活单元绑定
	GL_CALL(glGenTextures(1, &Texture));
	//激活纹理单元
	GL_CALL(glActiveTexture(GL_TEXTURE0));
	//绑定纹理对象
	GL_CALL(glBindTexture(GL_TEXTURE_2D, Texture));
	//3 传输纹理数据，此时才会开辟显存
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
	
	//释放数据，此时data数据是存储在RAM上的，但是现在我们已经将其拷贝到了GPURAM上，后续操作都是针对存在GPURAM的数据，所以可以掉这段无用数据，或者使用智能指针避免这个操作
	stbi_image_free(data);

	//4 设置纹理过滤方式
	//如果渲染像素大于原图，则用插值算法，成图较模糊，锯齿感弱
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//如果渲染像素小于原图，则就近原则，成图较清晰，锯齿感强
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 设置纹理包裹方式，设置x, y方向的超原图范围属性, S对应x(u), T对应y(v)
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT));

	/*
	 *	小心这里不能解绑Texture，因为我们激活了一个纹理单元，如果我们解绑纹理单元就会失去链接使得shader后续无法采样
	 *	GL_CALL(glBindTexture(GL_TEXTURE_2D,0));
·····*/
}

void prepareUVGLTranglesTest2()
{
	float vertex[]
{
		-1.0f,-1.0f,0.0f,0.0f,0.0f,
		-1.0f,1.0f,0.0f,0.0f,1.0f,
		1.0f,1.0f,0.0f,1.0f,1.0f,
		1.0f,-1.0f,0.0f,1.0f,0.0f,
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

void prepareTextureTest2()
{
	//声明图片长宽，颜色通道
	int width, height, channels;
	//反转y轴
	stbi_set_flip_vertically_on_load(true);
	//读取图片数据
	unsigned char* data = stbi_load("Texture/panda.jpg", &width, &height, &channels, STBI_rgb_alpha);

	//生成纹理
	glGenTextures(1, &Texture);
	//激活纹理单元
	glActiveTexture(GL_TEXTURE0);
	//绑定纹理对象
	glBindTexture(GL_TEXTURE_2D, Texture);
	//开辟显存并传输纹理数据
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,data);

	//释放RAM上的data
	stbi_image_free(data);

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	//如果渲染像素小于原图，则就近原则，成图较清晰，锯齿感强
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	//5 设置纹理包裹方式，设置x, y方向的超原图范围属性, S对应x(u), T对应y(v)
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

}
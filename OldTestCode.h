#pragma once
/* u can find test code belong to past classes
 * enjoy it
 */
#include "core.h"
#include "shader.h"
extern GLuint vao;
extern GLframework::Shader* shader;


//部分回调函数
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

//纹理部分练习题，第二问主要是针对shader进行更改，很简单这里就不贴出来了
void prepareUVGLTranglesTest1();
void prepareTextureTest1();


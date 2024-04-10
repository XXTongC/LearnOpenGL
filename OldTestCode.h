#pragma once
/* u can find test code belong to past classes
 * enjoy it
 */
#include "core.h"
#include "shader.h"
extern GLuint vao;
extern GLframework::Shader* shader;


//���ֻص�����
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
void OnResize(int width, int height);
void OnKeyboardCallback(int key, int action, int mods);
void OnMouseCallback(int button, int action, int mods);
//���½�Ϊvao��vbo�Ĵ������
void prepareVBO();
void prepareSingleBuffer();
void prepareInterleaveBuffer();
void prepareTest1();
void prepareTest2();
void prepareVAOForGLTrangles();
void prepareEBOForGLTrangles();
void prepareColorGLTrangles();

//��������ϰ�⣬�ڶ�����Ҫ�����shader���и��ģ��ܼ�����Ͳ���������
void prepareUVGLTranglesTest1();
void prepareTextureTest1();


# Shader 类接口说明书

`Shader` 类属于 `GLframework` 命名空间，用于加载、编译和管理 OpenGL 着色器程序。

## 构造函数

```cpp
Shader(const char* vertexPath, const char* fragmentPath)
```

通过指定的顶点着色器和片段着色器文件路径创建一个新的 Shader 对象。

## 析构函数

```cpp
~Shader()
```

清理与着色器程序相关的资源。

## 公共方法

### 程序管理

```cpp
GLuint getProgram() const
```

返回此着色器的 OpenGL 程序 ID。

```cpp
void begin()
```

激活着色器程序以供使用。

```cpp
void end()
```

停用着色器程序。

### Uniform 设置方法

```cpp
void setFloat(const std::string& name, float value)
```

设置着色器中的 float 类型 uniform 变量。

```cpp
void setInt(const std::string& name, int value)
```

设置着色器中的 int 类型 uniform 变量。

```cpp
void setVector3(const std::string& name, float x, float y, float z)
```

使用单独的 float 分量设置着色器中的 vec3 类型 uniform 变量。

```cpp
void setVector3(const std::string& name, const float* values)
```

使用 float 数组设置着色器中的 vec3 类型 uniform 变量。

```cpp
void setVector3(const std::string& name, glm::vec3 vec)
```

使用 glm::vec3 设置着色器中的 vec3 类型 uniform 变量。

```cpp
void setMat4(const std::string& name, const glm::mat4 values)
```

设置着色器中的 mat4 类型 uniform 变量。

```cpp
void setMat4Array(const std::string& name, const glm::mat4 values[], int count)
```

设置着色器中的 mat4 数组类型 uniform 变量。

```cpp
void setMat3(const std::string& name, const glm::mat3 values)
```

设置着色器中的 mat3 类型 uniform 变量。

### 着色器加载

```cpp
std::string loadShader(const std::string& filePath)
```

从文件加载着色器源代码并返回为字符串。此方法支持 `#include` 指令，可以包含其他着色器文件。

## 私有方法

```cpp
void checkShaderErrors(GLuint target, const std::string& type)
```

检查着色器编译或链接错误，如果存在错误则打印。

## 私有成员

```cpp
GLuint mProgram
```

此着色器的 OpenGL 程序 ID。

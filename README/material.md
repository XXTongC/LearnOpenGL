

# Material 类接口说明书

`Material` 类属于 `GLframework` 命名空间，用于管理 OpenGL 材质属性和渲染状态。

## 枚举类型

### MaterialType

```cpp
enum class MaterialType
{
    PhongMaterial,
    WhiteMaterial,
    DepthMaterial,
    OpacityMaskMaterial,
    ScreenMaterial,
    CubeMaterial,
    CubeSphereMaterial,
    PhongEnvMaterial,
    PhongEnvSphereMaterial,
    PhongInstanceMaterial,
    PhongNormalMaterial,
    GrassInstanceMaterial,
    PhongParallaxMaterial,
};
```

定义了不同类型的材质。

### PreStencilType

```cpp
enum class PreStencilType
{
    Normal,
    Outlining,
    Custom,
};
```

定义了预设的模板测试类型。

## 构造函数

```cpp
Material()
```

创建一个新的 Material 对象。

## 析构函数

```cpp
~Material()
```

清理与材质相关的资源。

## 公共方法

### 材质类型管理

```cpp
MaterialType getMaterialType() const
```

获取当前材质类型。

```cpp
void setMaterialType(MaterialType type)
```

设置材质类型。

### 深度测试设置

```cpp
void setDepthTest(bool value)
```

设置是否启用深度测试。

```cpp
void setDepthWrite(bool value)
```

设置是否启用深度写入。

```cpp
void setDepthFunc(GLenum value)
```

设置深度比较函数。

```cpp
bool getDepthTest() const
```

获取深度测试状态。

```cpp
bool getDepthWrite() const
```

获取深度写入状态。

```cpp
GLenum getDepthFunc() const
```

获取当前深度比较函数。

### 多边形偏移设置

```cpp
bool getPolygonOffsetState() const
```

获取多边形偏移状态。

```cpp
void setPolygonOffsetState(bool value)
```

设置是否启用多边形偏移。

```cpp
unsigned int getPolygonOffsetType() const
```

获取多边形偏移类型。

```cpp
void setPolygonOffsetType(unsigned int PolygonOffSetType)
```

设置多边形偏移类型。

```cpp
float getFactor() const
```

获取多边形偏移因子。

```cpp
void setFactor(float value)
```

设置多边形偏移因子。

### 纹理单元设置

```cpp
float getUnit() const
```
获取当前设置的纹理单元。

```cpp
void setUnit(float value)
```
设置要使用的纹理单元。这个函数用于指定图片应该绑定到哪个纹理单元。

### 模板测试设置

```cpp
void setSFail(unsigned int value)
```

设置模板测试失败时的操作。

```cpp
void setZFail(unsigned int value)
```

设置深度测试失败时的模板操作。

```cpp
void setZPass(unsigned int value)
```

设置深度测试通过时的模板操作。

```cpp
void setStencilMask(unsigned int value)
```

设置模板掩码。

```cpp
void setStencilFunc(unsigned int value)
```

设置模板测试函数。

```cpp
void setStencilRef(unsigned int value)
```

设置模板参考值。

```cpp
void setStencilFuncMask(unsigned int value)
```

设置模板函数掩码。

```cpp
void setStencilState(bool value)
```

设置是否启用模板测试。

```cpp
void setPreStencilPreSettingType(PreStencilType preType)
```

设置预定义的模板测试配置。

```cpp
unsigned int getSFail() const
unsigned int getZFail() const
unsigned int getZPass() const
unsigned int getStencilMask() const
unsigned int getStencilFunc() const
unsigned int getStencilRef() const
unsigned int getStencilFuncMask() const
PreStencilType getStencilPreSettingType() const
bool getStencilState() const
```

获取各种模板测试相关的设置。

### 颜色混合设置

```cpp
void setColorBlendState(bool value)
```

设置是否启用颜色混合。

```cpp
void setSFactor(unsigned int value)
```

设置源混合因子。

```cpp
void setDFactor(unsigned int value)
```

设置目标混合因子。

```cpp
void setOpacity(float value)
```

设置不透明度。

```cpp
bool getColorBlendState() const
float getOpacity() const
unsigned int getSFactor() const
unsigned int getDFactor() const
```

获取颜色混合相关的设置。

### 面剔除设置

```cpp
void setFaceCullingState(bool value)
```

设置是否启用面剔除。

```cpp
void setFrontFace(unsigned int value)
```

设置前面的定义方式。

```cpp
void setCullFace(unsigned int value)
```

设置要剔除的面。

```cpp
bool getFaceCullingState() const
unsigned int getFrontFace() const
unsigned int getCullFace() const
```

获取面剔除相关的设置。

## 私有成员

```cpp
MaterialType mType;
bool mDepthTest;
GLenum mDepthFunc;
bool mDepthWrite;
bool mPolygonOffsetState;
unsigned int mPolygonOffsetType;
float mFactor;
float mUnit;
bool mStencilTestState;
unsigned int mSFail, mZFail, mZPass;
unsigned int mStencilMask, mStencilFunc, mStencilRef, mStencilFuncMask;
PreStencilType mStencilPreSettingType;
bool mColorBlendState;
unsigned int mSFactor, mDFactor;
float mOpacity;
bool mFaceCullingState;
unsigned int mFrontFace, mCullFace;
```

这些私有成员存储了材质的各种属性和状态。

---

这个 Material 类提供了一个全面的接口来管理 OpenGL 材质属性和渲染状态，包括深度测试、多边形偏移、模板测试、颜色混合和面剔除等功能。它封装了 OpenGL 状态的复杂性，为开发者提供了一个方便的 API 来控制渲染过程中的各种参数。

特别注意，`setPreStencilPreSettingType` 方法提供了预设的模板测试配置，可以快速设置常用的模板测试场景，如普通渲染和轮廓渲染。

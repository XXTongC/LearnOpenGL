# 已完成事项记录

## 2026-04-02

### 已完成

1. 创建了重构工作记录文件 `worked.md`，用于持续登记每次已完成的重构动作。
2. 盘点了当前项目的主要入口和核心模块，重点查看了：
   - `main.cpp`
   - `Application.h`
   - `Application.cpp`
   - `renderer.h`
   - `renderer.cpp`
3. 识别出当前第一批高优先级问题：
   - `main.cpp` 体量较大，承载了过多全局状态和启动流程细节。
   - `Renderer` 的构造逻辑直接写在头文件中，包含大量 shader 初始化细节，职责暴露过多。
   - `Renderer` 中多个接口对 `pointLights` 采用按值传递，存在不必要的复制。
4. 选定第一轮重构切口：
   - 先整理 `Renderer` 的初始化边界。
   - 将 shader 初始化从 `renderer.h` 中挪出，收敛到 `renderer.cpp`。
   - 将 `Renderer` 相关接口中的 `pointLights` 改为按 `const&` 传递。
5. 为了让后续补丁能够稳定应用，将 `renderer.h` 和 `renderer.cpp` 转换为了 UTF-8 编码。
6. 完成第一轮 `Renderer` 结构重构：
   - 将 `Renderer` 构造函数从头文件移出，改为在 `renderer.cpp` 中定义。
   - 新增 `initializeShaders()`，集中管理 shader 初始化逻辑。
   - 新增 `createShader()`，统一 shader 创建入口。
   - 将多个 `pointLights` 参数由按值传递改为按 `const&` 传递，减少不必要复制。
   - 保持 `Renderer` 的外部使用方式不变，属于低风险结构整理。
7. 完成第二轮 `main.cpp` 入口收敛：
   - 新增 `initializeApplication()`，统一处理窗口初始化、OpenGL 初始状态、相机准备、场景准备和 ImGui 初始化。
   - 新增 `runFrame()`，统一收纳每帧更新和两段渲染 pass。
   - 新增 `printOpenGLCapabilities()`，将显卡能力打印从 `main()` 中抽离。
   - 删除了未实现且具有误导性的 `render()` 前向声明。
   - `main()` 现在只保留“初始化 -> 主循环 -> 退出”三段主职责。
8. 进行了构建验证：
   - 使用 Visual Studio 2022 自带的 `MSBuild.exe` 构建 `text2.sln`。
   - 构建结果：成功，`0` error，存在若干历史遗留 warning。
9. 为了继续稳定补丁流程，将 `Application.h` 和 `Application.cpp` 转换为了 UTF-8 编码。
10. 完成第三轮 `Application` 生命周期与接口清理：
   - 将 `GLFWwindow` 前向声明从 `class` 改为 `struct`，与 GLFW 头文件保持一致。
   - 为 `Application` 显式禁用了拷贝与移动，强化 singleton 边界。
   - 将正确接口名统一为 `destroy()`，保留 `destory()` 作为兼容转发。
   - 在窗口尺寸回调中，无论是否注册 resize callback，都更新 `mWidth` / `mHeight`。
   - 将 `main.cpp` 中的退出调用同步改为 `GL_APP->destroy()`。
11. 完成第二次构建验证：
   - 构建结果：成功，`0` error。
   - 与本轮修改直接相关的 `GLFWwindow` 类型声明 warning 已消除。
   - 当前剩余警告已显著收敛，增量构建只剩 `3` 个 warning。
12. 完成第四轮 `main.cpp` 运行时上下文收拢：
   - 新增 `AppRuntimeContext`，集中承载核心运行时对象。
   - 首批纳入上下文的内容包括：
     - `renderer`
     - `sceneOffScreen`
     - `sceneInScreen`
     - `ambientLight`
     - `framebufferMultisample`
     - `framebufferResolve`
     - `grassMaterial`
     - `camera`
     - `cameracontrol`
     - `clearColor`
     - `dirLight`
     - `spotLight`
     - `pointLights`
     - `movePlane`
     - `textD`
     - `ScreenMat`
     - `skyBoxMesh`
     - `meshPointLight`
   - 采用“上下文 + 同名引用别名”的过渡式方案，在不大面积改写函数体的前提下先收拢所有权。
13. 补充了运行时退出清理：
   - 新增 `cleanupRuntime()`。
   - 在主循环结束后显式释放 `cameracontrol` 和 `camera` 两个裸指针。
   - 这一步修复了退出时明显的生命周期遗漏。
14. 完成第三次构建验证：
   - 构建结果：成功，`0` error，仍为 `3` 个 warning。
15. 完成第五轮 `prepare()` 分阶段拆分：
   - 新增 `prepareRenderResources()`，集中处理：
     - `renderer`
     - `sceneInScreen`
     - `sceneOffScreen`
     - `framebufferMultisample`
     - `framebufferResolve`
     - `PointLightShadow` 共享深度纹理初始化
   - 新增 `prepareScreenPass()`，集中处理屏幕输出 quad 和 `ScreenMaterial` 初始化。
   - 新增 `prepareLights()`，集中处理：
     - `spotLight`
     - `dirLight`
     - `ambientLight`
     - `pointLights`
   - `prepare()` 现在已经从“单块大函数”变成“资源初始化 -> 场景对象构建 -> 屏幕 pass -> 灯光初始化”的更清晰流程。
16. 完成第四次构建验证：
   - 构建结果：成功，`0` error，仍保持 `3` 个 warning。
17. 完成第六轮场景对象构建拆分：
   - 新增 `prepareRoomScene()`。
   - 将当前真正生效的房间/地面/盒子场景构建逻辑从 `prepare()` 中抽离出去。
   - 本轮迁移的主要对象包括：
     - 六面房间地面/墙面 plane
     - `textD`
     - 当前启用的 box 测试对象
   - 保留了原有实验性注释区块不动，避免误删历史参考代码。
18. 完成第五次构建验证：
   - 构建结果：成功，`0` error，仍为 `3` 个 warning。

19. 完成第七轮历史实验代码收口：
   - 新增 [LegacyExperimentRunner.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\legacyExperiments\LegacyExperimentRunner.h)，将历史实验逻辑整理为可复用的实验管理器。
   - 当前已收口的实验入口包括：
     - `enableSolarSystem`
     - `enableGrassField`
     - `enableEnvironmentSphere`
     - `enableCsmPlane`
     - `enableBackpackModel`
     - `enableShadowPreview`
     - `enableOrbitingPointLight`
   - 为避免重复挂载场景对象，实验管理器为多数组装型实验补充了启用态保护。
   - 同时补齐了实验管理头文件缺失的依赖引用，并修正了逐帧轨道点光源更新里的浮点窄化问题。
20. 完成第八轮 `main.cpp` 实验主流程接线：
   - 引入 `LegacyExperimentRunner` 到主程序入口。
   - 在 `AppRuntimeContext` 中新增 `csmShadowMaterial`，把实验运行所需对象继续收拢进统一上下文。
   - 新增：
     - `makeLegacyExperimentContext()`
     - `prepareLegacyExperiments()`
     - `updateLegacyExperiments()`
   - 将 `runFrame()` 中原本分散的 `rotatePlant()` / `rotateLight()` 替换为统一的 `updateLegacyExperiments()`。
   - 将 `prepare()` 中大段历史实验注释替换为一个集中实验配置点；现在只需要在 `prepareLegacyExperiments()` 中启用少量函数调用，就能重新打开对应实验。
   - 清理了已失效的历史全局状态与相关入口，包括：
     - 太阳系实验的 `roundFor*` / `speed`
     - 旧的 `mat2`
     - `rotatePlant()`
     - `rotateLight()`
     - 未再使用的 `parallaxMat` 全局
21. 完成第六次构建验证：
   - 构建结果：成功，`0` error，仍为 `3` 个 warning。
   - 说明新的实验管理器接线已通过真实工程构建验证，可继续作为后续重构基础。
22. 完成第九轮材质 Inspector 系统最小版落地：
   - 新增 [MaterialInspector.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\MaterialInspector.h)，建立了声明式可编辑属性系统。
   - 新系统当前支持的属性类型包括：
     - `float`
     - `bool`
     - `vec3`
     - `color3`
     - 只读文本信息
     - 分节标题
   - `UI` 端现在不再直接依赖具体材质类字段，而是通过 `PropertyBuilder` 消费材质声明的属性描述。
23. 完成第十轮材质类可编辑属性接线：
   - 为 [material.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\material.h) / [material.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\material.cpp) 新增统一入口 `visitEditableProperties(...)`。
   - `Material` 基类已统一暴露通用渲染状态：
     - `Depth Test`
     - `Depth Write`
     - `Blend`
     - `Face Culling`
     - `Opacity`
     - `Polygon Offset Factor`
     - `Polygon Offset Unit`
   - 第一批接入自动 Inspector 的具体材质包括：
     - [phongMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\phongMaterial.cpp)
     - [grassInstanceMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\grassInstanceMaterial.cpp)
     - [phongCSMShadowMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\phongCSMShadowMaterial\phongCSMShadowMaterial.cpp)
     - [phongPointShadowMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\phongPointShadowMaterial\phongPointShadowMaterial.cpp)
     - [screenMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\screenMaterial.cpp)
   - 这些材质现在都能声明自己的参数范围与纹理槽信息，而不需要在 `main.cpp` 手写对应控件。
24. 完成第十一轮场景材质自动收集与展示：
   - 在 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中新增场景材质递归收集逻辑。
   - 当前会自动遍历 `sceneOffScreen` 与 `sceneInScreen`，收集去重后的材质实例，并在新的 `materials` 面板中自动生成 inspector。
   - 这意味着后续只要某个实验场景把材质挂进 `Mesh` / `InstancedMesh`，就会自动出现在材质面板里，不再需要每次换实验都重写一套 `ImGui`。
25. 完成第七次构建验证：
   - 首次完整 `Build` 时发现 `GrassInstanceMaterial` 的实现文件实际由根目录 [grassInstanceMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\grassInstanceMaterial.cpp) 参与编译，而不是子目录副本；已修正并重新接通实现。
   - 修复后增量 `Build` 成功，结果为 `0` error、`0` warning。
   - 随后执行完整 `Rebuild`，结果为 `0` error；当前工程仍存在一批历史 warning，完整重建统计为 `45` 个 warning，本轮未新增构建错误。
26. 完成第十二轮选择状态基础设施接入：
   - 为 [object.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\object.h) / [object.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\object.cpp) 补充了对象编辑器所需的基础能力：
     - `getScale()`
     - `setName()`
     - `getName()`
   - 将 [scene.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\scene.h) 的 `Scene` 默认类型显式设为 `Scene`，让场景根节点能被 hierarchy 正确识别。
   - 为了后续补丁和重构稳定性，还将相关对象文件统一转换为了 UTF-8 编码。
27. 完成第十三轮 hierarchy + selection 编辑器流转落地：
   - 在 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中新增 `EditorSelectionContext`，正式引入“当前选中对象”这一中间状态。
   - 新增层级面板与选择面板相关逻辑：
     - `renderHierarchyPanel()`
     - `renderSelectionInspectorPanel()`
     - `renderObjectHierarchyNode()`
     - `ensureSelectionIsInitialized()`
     - `getSelectedObject()`
   - 原先基于“遍历全部材质”的面板已替换为真正的 `hierarchy -> selection -> inspector` 数据流。
   - Inspector 现在会针对当前选中对象显示：
     - 名称
     - 类型
     - 子节点数量
     - 本地 Position / Rotation / Scale
     - 若选中项是 `Mesh`，则自动显示其材质 inspector
28. 完成第十四轮场景命名补强：
   - 为当前主场景中的关键对象补充了可读名称，提升 hierarchy 可用性。
   - 当前已命名的核心节点包括：
     - `World Scene`
     - `Screen Scene`
     - `SkyBox`
     - `Floor`
     - `Ceiling`
     - `Front Wall`
     - `Back Wall`
     - `Left Wall`
     - `Right Wall`
     - `Center Box`
     - `Screen Quad`
29. 完成第八次构建验证：
   - 使用 VS2022 MSBuild 对当前工程再次执行真实 `Build`。
   - 构建结果：成功，`0` error。
   - 当前增量构建统计为 `25` 个 warning；本轮选择机制改造未引入新的构建错误。
30. 完成第十五轮多目标 selection 上下文扩展：
   - 将 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中的选择状态从“仅支持 `Object`”扩展为统一支持：
     - `Object`
     - `Camera`
     - `Shadow`
   - 新增了显式选择入口：
     - `selectObject(...)`
     - `selectCamera(...)`
     - `selectShadow(...)`
   - 这一步让 hierarchy 和 inspector 不再局限于场景网格对象，而是能覆盖更完整的运行时编辑目标。
31. 完成第十六轮 Light / Camera / Shadow hierarchy 接线：
   - 在 hierarchy 面板中新增了 `Lights` 和 `Cameras` 分组。
   - 灯光节点现在支持层级展开，当前每个 `Light` 下可继续选择：
     - 对应 `Shadow`
     - 若存在，则对应 `Shadow Camera`
   - 同时在 [light.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\light.h) 中将 `Light` 的默认对象类型显式设为 `Light`，避免在选择系统中被误识别为普通 `Object`。
   - 在场景初始化阶段为当前运行中的光照对象补充了可读名称，包括：
     - `Directional Light`
     - `Spot Light`
     - `Point Light 0`
     - `Point Light 1`
32. 完成第十七轮 Light / Camera / Shadow Inspector 落地：
   - 当前 `inspector` 会根据选中目标类型自动分流：
     - 选中 `Object`：显示基础 Transform
     - 选中 `Light`：在 Transform 基础上继续显示灯光参数
     - 选中 `Camera`：显示相机参数
     - 选中 `Shadow`：显示阴影参数
   - 当前已接入的编辑项包括：
     - `Light`
       - 颜色
       - 强度
       - 高光强度
       - `PointLight` 衰减系数
       - `SpotLight` 内外角
     - `Camera`
       - 位置
       - `Up`
       - `Right`
       - `Near` / `Far`
       - `PerspectiveCamera` 的 `Fovy` / `Aspect`
       - `OrthographicCamera` 的 `Left` / `Right` / `Top` / `Bottom`
     - `Shadow`
       - `Bias`
       - `PCF Radius`
       - `Disk Tightness`
       - `Light Size`
       - 阴影贴图尺寸
       - `DirectionalLightCSMShadow` 的 cascade layer 数
       - `PointLightShadow` 的 shadow map index
33. 完成第九次构建验证：
   - 针对 `Light / Camera / Shadow` selection inspector 接线后再次执行真实 `Build`。
   - 构建结果：成功，`0` error。
   - 当前增量构建结果维持为 `25` 个 warning，本轮未新增构建错误。
34. 完成第十八轮选中对象场景高亮基础设施：
   - 为 [object.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\object.h) / [object.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\object.cpp) 新增 `removeChild(...)`，允许编辑器运行时对象在父节点之间安全重挂。
   - 在 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中新增了选中高亮所需的运行时对象：
     - `selectionOutlineMesh`
     - `selectionOutlineMaterial`
   - 当前 outline 使用现有 `WhiteMaterial` 路线，不改写原始业务材质状态。
35. 完成第十九轮 hierarchy 选中高亮同步：
   - 新增 `updateSelectionOutline()` 与 `hideSelectionOutline()`。
   - 当前每帧会根据 `selectedObject` 自动同步 outline 外壳：
     - 几何体跟随当前选中 `Mesh`
     - 本地 `Position / Rotation / Scale` 同步
     - 外壳缩放为原对象的 `1.06x`
     - 若切换到其他父节点，会自动从旧父节点移除并挂到新父节点下
   - 为了避免影响层级结构浏览，编辑器辅助对象会被 `hierarchy` 自动过滤，不会显示为正常场景节点。
   - 当前高亮策略为：
     - 仅对当前选中的 `Mesh` / `InstancedMesh` 生效
     - 非网格目标（如 `Light` / `Camera` / `Shadow`）暂不绘制 outline
36. 完成第二十轮 outline 渲染策略接线：
   - outline 材质当前采用“白色外壳”方案：
     - `WhiteMaterial`
     - 关闭深度写入
     - 开启面剔除
     - 剔除正面 `GL_FRONT`
     - 以透明队列方式参与主渲染，避免进入阴影贴图渲染列表
   - 这条路线不依赖修改原 mesh 材质的 stencil 设定，落地更稳，也更适合作为第一版可视反馈。
37. 完成第十次构建验证：
   - 针对选中对象 outline 高亮接线后再次执行真实 `Build`。
   - 构建结果：成功，`0` error。
   - 当前增量构建结果仍为 `25` 个 warning，本轮未新增构建错误。
38. 完成第二十一轮 outline 反馈修正：
   - 根据实际观察，将 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中的 outline 放大量从 `1.06x` 收窄为 `1.02x`，降低了边框侵入感。
   - 同时调整了 outline 材质状态：
     - 关闭颜色混合
     - 关闭深度测试
     - 保留正面剔除
     - 显式设置 stencil `NOTEQUAL` 规则
   - 这一步的目标是让轮廓更细、更像真正的边缘描边，而不是明显的“第二层外壳”。
39. 完成第二十二轮 stencil outline 渲染通道接入：
   - 在 [renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer.h) / [renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer.cpp) 中新增了 selection outline 专用渲染接口：
     - `setSelectionOutlineMeshes(...)`
     - `renderSelectionOutline(...)`
   - 当前 outline 流程已从原先的“仅靠放大外壳直接参与主场景渲染”升级为：
     1. 用选中 mesh 本体写入 stencil
     2. 再绘制放大后的白色 outline mesh
     3. 通过 `GL_NOTEQUAL` 仅保留外轮廓
   - 这一步专门修复了“单独面片整面发白”的问题，因为现在面片内部会被 stencil 遮掉，只留下边缘区域。
40. 完成第二十三轮 outline 辅助对象解耦：
   - `selectionOutlineMesh` 不再作为普通场景子节点参与常规投影队列，而是作为 selection 专用对象交给 renderer 的 outline pass。
   - 这减少了 outline 对主场景层级与常规渲染排序的干扰，也避免了它以普通对象身份再次进入场景收集流程。
41. 完成第十一次构建验证：
   - 针对 outline 缩小与 stencil 化修正后再次执行真实 `Build`。
   - 构建结果：成功，`0` error。
   - 当前增量构建仍为 `25` 个 warning，本轮未新增构建错误。
42. 完成第二十四轮选中描边分支回退：
   - 按当前重构目标重新对齐方向，撤销了“选中后外轮廓描边”这一条支线实现，避免继续在偏实验性的高亮方案上投入。
   - 已从 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中移除：
     - `selectionOutlineMesh`
     - `selectionOutlineMaterial`
     - `updateSelectionOutline()`
     - `hideSelectionOutline()`
     - 与 outline 专用过滤相关的辅助逻辑
   - 已从 [renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer.h) / [renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer.cpp) 中移除 selection outline 专用渲染接口与通道。
   - 已同步撤回为 outline 专门加入的对象树辅助接口，恢复 [object.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\object.h) / [object.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\object.cpp) 的主线状态。
43. 完成第十二次构建验证：
   - 在回退选中描边分支后再次执行真实 `Build`。
   - 构建结果：成功，`0` error。
   - 当前增量构建结果仍为 `25` 个 warning，说明本次回退已回到稳定主线。
44. 完成第二十五轮编辑器模块拆分：
   - 新增 [tools/editor/EditorPanels.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\EditorPanels.h) 与 [tools/editor/EditorPanels.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\EditorPanels.cpp)，把 `hierarchy / selection / inspector` 的状态与面板渲染从 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中抽离出来。
   - 新模块当前统一承接了：
     - 选中状态 `SelectionContext`
     - hierarchy 树绘制
     - object / light / camera / shadow inspector
   - [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 现在只保留 `makeEditorPanelContext()` 与 `renderIMGUI()` 中的高层调度，不再直接承载大段编辑器实现细节。
   - 同步更新了 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj)，让 `tools/editor` 模块进入工程编译列表。
45. 完成第十三次构建验证：
   - 针对编辑器模块拆分后再次执行真实 `Build`。
   - 构建结果：成功，`0` error。
   - 当前增量构建结果为 `3` 个 warning，主要来自：
     - `APIENTRY` 宏重定义
     - `main.cpp` 中既有的 `size_t -> int`
     - `double -> float`
   - 说明这轮拆分已经在不新增构建错误的前提下稳定落地。
46. 完成第二十六轮物理目录重组：
   - 按 VS 中既有的模块分类，对 [text2-refactor](C:\Code\CodeOfC++\OpenGL_test\text2-refactor) 的物理文件夹进行了重新组织，新增并落地了这些目录：
     - [application](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application)
     - [camera](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\camera)
     - [framework](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framework)
     - [framebuffer](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framebuffer)
     - [legacy](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\legacy)
     - [wrapper](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\wrapper)
   - 同时将原本散落在根目录的实现/头文件迁入了更对应的模块目录，例如：
     - `Application / assimp*` 迁入 [application](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application)
     - `camera / cameracontrol / perspective / orthographic / trackball` 迁入 [camera](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\camera)
     - `object / scene / geometry / shader / texture` 迁入 [framework](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framework)
     - `framebuffer` 迁入 [framebuffer](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framebuffer)
     - 根目录灯光与材质实现分别收拢进 [light](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\light) 与 [materials](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials)
     - `GL_ERROR_FIND` 迁入 [wrapper](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\wrapper)
     - `OldTestCode` 迁入 [legacy](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\legacy)
   - 将 `grassInstanceMaterial` 的重复实现收敛为单一来源，最终保留 [materials/grassInstanceMaterial/grassInstanceMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\grassInstanceMaterial\grassInstanceMaterial.cpp) 作为唯一有效实现。
   - 将工程内原本指向用户下载目录的 `glad.c` 外部路径收回为仓库内的 [third_party/src/glad.c](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\third_party\src\glad.c)，消除了对 `Downloads` 目录的隐式依赖。
47. 完成第十四次构建验证：
   - 针对整轮物理目录重组、`vcxproj / filters` 路径更新、以及受影响 `#include` 修正后再次执行真实 `Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 说明本轮不仅完成了目录与工程结构对齐，也顺带把此前增量构建里残留的一批 warning 清到了当前这次构建结果的 `0/0` 状态。
48. 完成第二十七轮场景装配模块拆分：
   - 新增 [tools/sceneSetup/SceneSetup.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.h) 与 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，把默认场景的装配流程从 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中抽离成独立模块。
   - 新模块当前统一承接了这些职责：
     - 渲染资源初始化
     - SkyBox 准备
     - 房间场景对象构建
     - 屏幕 pass 准备
     - 默认灯光装配
   - [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 现在通过 `makeSceneSetupContext()` 组装上下文，并以 `GL_SCENE::prepareDefaultScene(...)` 触发场景准备，主流程职责进一步收敛。
   - 同步更新了 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `SceneSetup` 模块纳入工程与 VS 视图分类。
49. 完成第十五次构建验证：
   - 针对场景装配模块拆分后再次执行真实 `Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 说明默认场景准备逻辑已经在不引入新的耦合和构建噪音的前提下稳定迁出。
50. 完成第一轮 GitHub 分支发布：
   - 为 [text2-refactor](C:\Code\CodeOfC++\OpenGL_test\text2-refactor) 配置了指向 `https://github.com/XXTongC/LearnOpenGL.git` 的远程，并保留本地 `master` 不受影响。
   - 将当前重构成果提交到新分支 `codex/text2-refactor` 并成功推送到 GitHub。
   - 本次发布未覆盖 `master`，后续可以继续在该分支上独立推进重构。
51. 完成第一轮全项目重构分析：
   - 基于当前 `codex/text2-refactor` 分支重新盘点项目结构，明确本轮分析只针对重构版本，不以 `master` 的旧结构为准。
   - 在 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md) 中补充 `2026-05-20 全项目重构分析（refactor 分支）`，记录当前模块状态、主要耦合热点、后续风险和建议顺序。
   - 当前判断：下一步最适合继续拆 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中剩余的 debug controller UI，把它迁入 `tools/editor` 下的独立模块。
52. 完成第二轮全项目重构分析补强：
   - 继续基于当前 `codex/text2-refactor` 工作区盘点模块依赖，补充了 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md) 中的模块依赖地图。
   - 按高 / 中 / 低优先级整理了 `main.cpp`、`renderer`、EditorPanels、include 风格、资源生命周期和 legacy 实验入口的后续风险。
   - 补充了分阶段执行 backlog，并明确下一轮 Debug Controller Panel 拆分的验收标准。
53. 完成第二十八轮 Debug Controller Panel 拆分：
   - 新增 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h) 与 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，把 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中 `"controller"` 调试面板迁入独立 editor 模块。
   - [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 现在通过 `makeDebugControllerContext()` 显式传入调试面板依赖，`renderIMGUI()` 只保留 ImGui frame 生命周期和面板调度。
   - 同步更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，确保新模块进入 VS 工程。
54. 完成第十六次构建验证：
   - 针对 Debug Controller Panel 拆分后执行真实 `Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 同步清理了入口层两个 warning：移除未使用的 `windows.h`，并在滚轮回调处显式处理 `double -> float` 转换。
55. 完成第二十九轮 Renderer 状态拆分：
   - 新增 [renderer/RenderState.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RenderState.h) 与 [renderer/RenderState.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RenderState.cpp)，把 depth、polygon offset、stencil、blend、face culling 的 OpenGL 状态应用逻辑从 [renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 中迁出。
   - [renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 现在通过 `RenderState::applyMaterialState(*material)` 应用材质通用渲染状态，为后续 PBR 材质绑定和 shader 参数上传拆分留出边界。
   - 同步更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，确保 `RenderState` 模块进入 VS 工程。
56. 完成第十七次构建验证：
   - 针对 Renderer 状态拆分后执行真实 `Build`。
   - 构建结果：成功，`0` error。
   - 本次项目文件变更触发多文件重编译，暴露 `22` 个既有 warning，主要集中在 Assimp loader 的有符号/无符号比较与 Renderer 的窄化转换；本轮未继续混入 warning 清理。
57. 完成第三十轮 ShaderLibrary 拆分：
   - 新增 [renderer/ShaderLibrary.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.h) 与 [renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)，集中管理 `MaterialType -> Shader` 映射、普通材质 shader 初始化、shadow shader 初始化。
   - [renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 不再保存一长串 shader 成员，改为持有 `ShaderLibrary mShaderLibrary`。
   - [renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 不再负责 shader 创建和 switch 选择；普通渲染通过 `getShader(...)` 间接访问，shadow pass 通过 `ShaderLibrary` 获取专用 shader。
   - 这一步为后续接入 PBR 渲染路径降低了改动范围：新增 PBR shader 时优先扩展 `ShaderLibrary`，而不是继续扩大 Renderer 主文件。
58. 完成第十八次构建验证：
   - 针对 `ShaderLibrary` 拆分后执行真实 `Build`。
   - 构建结果：成功，`0` error。
   - warning 数量仍为 `22`，来源与上一轮一致，后续应作为单独清理任务处理。
59. 完成第十九次构建验证：
   - 在所有本轮代码与文档变更完成后再次执行增量 `Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 当前可提交状态下工程可以稳定生成 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe)。
60. 完成第三十一轮 PBR 材质数据骨架接入：
   - 新增 [materials/pbrMaterial/PBRMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.h) 与 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)，定义 `PBRMaterial` 的 albedo、metallic、roughness、ao、normal、emissive 等基础数据。
   - 在 [materials/material.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\material.h) 中新增 `MaterialType::PBRMaterial`。
   - 在 [tools/inspector/MaterialInspector.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\MaterialInspector.h) 中接入 PBR 材质类型名称与声明式属性展示，保持材质 UI 自动生成路径一致。
61. 完成第三十二轮最小 PBR shader 路径接入：
   - 新增 [shaders/pbr/pbr.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.vert) 与 [shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag)，提供直接光照版 GGX PBR 基础 shader。
   - PBR vertex/fragment shader 显式传递并使用 `aColor`，避免几何初始化通过 `glGetAttribLocation("aColor")` 查询时被 shader 优化导致 attribute location 失效。
   - 在 [renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp) 中注册 `PBRMaterial -> shaders/pbr/pbr.*` 映射。
   - 在 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 中增加 PBR 材质最小 uniform 上传和可选贴图绑定分支。
   - 同步更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，确保新材质与 shader 纳入 VS 工程和分类。
62. 完成第二十次构建验证：
   - 针对 PBR 材质骨架、PBR shader 路径、工程文件更新后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`22` warning。
   - 本次 warning 来源仍为既有问题：Assimp loader 的有符号/无符号比较，以及 Renderer 旧代码中的 `double -> float` / `size_t -> int` 窄化转换；PBR 新增 C++ 文件已参与编译并生成 `PBRMaterial.obj`。
   - 当前验证边界：MSBuild 不会编译 GLSL shader，`shaders/pbr/pbr.*` 的运行时 shader 编译仍需要后续启动程序或引入 GLSL 验证工具确认。
63. 完成第二十一次增量构建验证：
   - 在补充 PBR shader 的 `aColor` attribute 保活逻辑后再次执行 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 说明最新工作区在 C++ / VS 工程层面可以稳定增量生成。
64. 完成第三十三轮 `MaterialBinder` 初步拆分：
   - 新增 [renderer/MaterialBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.h) 与 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，建立材质 uniform 上传的独立边界。
   - 将 `PhongMaterial` 与 `PBRMaterial` 的通用矩阵、法线矩阵、灯光、材质参数和贴图绑定逻辑从 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 迁入 `MaterialBinder`。
   - [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 现在优先调用 `MaterialBinder::bind(...)` 处理已迁移材质，未迁移的旧材质仍走原有 switch，降低一次性重构风险。
   - 同步更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `MaterialBinder` 纳入 VS 工程与 Renderer 分类。
65. 完成第二十二次构建验证：
   - 针对 `MaterialBinder` 拆分后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`12` warning。
   - 本次 warning 均来自重编译后的 Renderer 旧代码，主要是历史 `double -> float`、`size_t -> int` 窄化转换与 include packing 警告；新增 `MaterialBinder.cpp` 成功编译并链接为 `MaterialBinder.obj`。
66. 完成第二十三次构建验证：
   - 在整理 `Renderer` 中 `MaterialBinder::bind(...)` 分支缩进后再次执行 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`12` warning。
   - warning 来源仍为 Renderer 旧代码中的 include packing 与窄化转换，未出现新的编译错误。
67. 完成第三十四轮 TBN 相关材质绑定迁移：
   - 将 `PhongNormalMaterial` 与 `PhongParallaxMaterial` 的 uniform 上传、贴图绑定、TBN 相关 sampler 绑定从 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 迁入 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)。
   - [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 不再直接包含 `phongNormalMaterial.h` 与 `phongParallaxMaterial.h`，Renderer 对具体 normal/parallax 材质的直接依赖减少。
   - `MaterialBinder` 新增统一的 `bindTexture(...)` helper，普通 Phong、PhongNormal、PhongParallax 统一按 `Texture::getUnit()` 设置 sampler 并绑定纹理，为后续 PBR normal map 贴图槽管理做准备。
68. 完成第二十四次构建验证：
   - 针对 TBN 相关材质绑定迁移后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`11` warning。
   - warning 仍来自 Renderer 旧代码中的 include packing 与窄化转换；由于 normal/parallax 分支迁出，Renderer 旧 warning 数量从上一轮 `12` 个降到 `11` 个。
69. 完成第三十五轮 PBR normal map 接入：
   - 更新 [shaders/pbr/pbr.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.vert)，接收 `aTangent` 并输出 `TBN`。
   - 更新 [shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag)，新增 `normalMap` / `useNormalMap`，启用贴图时将 tangent-space normal 转换到世界空间后参与 GGX 光照。
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，把 `PBRMaterial::mNormalMap` 纳入可选贴图绑定流程。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR normal map 当前状态和后续 tangent 数据验证风险。
70. 完成第二十五次构建验证：
   - 针对 PBR normal map 接入后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 当前验证边界不变：MSBuild 不会编译 GLSL，`pbr.vert/.frag` 仍需要运行时 shader 编译或外部 GLSL validator 进一步确认。
71. 完成第三十六轮 Geometry tangent fallback 补强：
   - 更新 [framework/geometry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framework\geometry.cpp)，为没有显式 tangent 输入的通用 `Geometry` 构造函数补充 tangent 自动生成逻辑。
   - 为显式 tangent 构造函数增加 `aTangent` attribute guard，避免 shader 不使用 tangent 时仍绑定无效 attribute location。
   - 新增 tangent 退化 fallback：当 UV 退化或切线向量不可用时，根据 normal 生成稳定正交 tangent，降低 PBR normal map 在普通几何上的失效风险。
72. 完成第二十六次构建验证：
   - 针对 Geometry tangent fallback 补强后执行真实 `Debug|x64 Build`。
   - 初次构建成功但暴露 `4` 个 `size_t -> GLsizei` warning，随后对 `mIndicesCount` 赋值增加显式 `static_cast<GLsizei>`。
   - 再次构建结果：成功，`0` error，`0` warning。
73. 完成第三十七轮 Geometry attribute guard 收敛：
   - 更新 [framework/geometry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framework\geometry.cpp)，新增统一的 `bindFloatAttribute(...)`，对 shader 未使用的 attribute location 进行 guard。
   - 将 position、uv、normal、color、tangent 等 VBO attribute 绑定逐步收敛到统一 helper，避免 shader 优化掉 attribute 后仍执行无效绑定。
   - 更新 [shaders/pbr/pbr.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.vert) 与 [shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag)，移除 PBR shader 对 `aColor` 的依赖，避免无 vertex color 的导入模型被乘黑。
74. 完成第二十七次构建验证：
   - 针对 Geometry attribute guard 与 PBR shader 去 vertex color 依赖后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
75. 完成第三十八轮默认场景 PBR preview 接入：
   - 更新 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，新增 `preparePBRPreview(...)`。
   - 默认 world scene 现在会创建一个 `PBR Preview Sphere`，使用 `PBRMaterial`、PBR shader、程序生成 sphere geometry 和 `Texture/normal/normal_map.png` normal map。
   - 这一步让默认场景实际覆盖 PBR material、PBR shader、TBN geometry attribute 与 `MaterialBinder` 的 PBR normal map 绑定路径，后续启动程序即可做运行时 shader/link/visual 验证。
76. 完成第二十八次构建验证：
   - 针对默认场景 PBR preview 接入后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 当前验证仍是 C++ / VS 工程层面；PBR shader 编译和 preview 视觉效果需要后续启动程序确认。
77. 完成第三十九轮 Shader 运行时诊断补强：
   - 更新 [framework/shader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framework\shader.cpp)，为 shader 文件加载失败增加明确路径输出，避免缺失文件静默进入空源码编译。
   - 为 shader `#include` 解析增加引号格式检查，格式错误时输出当前 shader 文件路径。
   - 将 vertex compile、fragment compile、program link 的错误输出改为带阶段和路径上下文，后续验证 PBR / shadow / IBL shader 时更容易定位失败来源。
   - 更新 [framework/shader.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framework\shader.h)，同步调整 `checkShaderErrors(...)` 内部接口。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 shader 诊断补强对后续 PBR 路径验证的意义。
78. 完成第二十九次构建验证：
   - 使用 Visual Studio 2022 Community 的 MSBuild 完整执行 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`22` warning。
   - warning 来源为既有代码：Assimp loader 的有符号/无符号比较、Bloom 的 float 到 int 转换、Renderer 旧代码中的 include packing 与窄化转换；本轮 `framework/shader.*` 改动未引入新的编译错误。
79. 完成第四十轮 `ShadowRenderer` 初步拆分：
   - 新增 [renderer/ShadowRenderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowRenderer.h) 与 [renderer/ShadowRenderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowRenderer.cpp)，承接 CSM 方向光 shadow map 与 point light shadow map 绘制。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，移除 `renderShadowMap(...)`、`renderDirShadowMap(...)`、`renderPointShadowMap(...)` 的具体实现，主渲染流程改为委派 `mShadowRenderer.render(...)`。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)，移除 shadow pass 私有函数声明并新增 `ShadowRenderer` 成员。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `ShadowRenderer` 纳入 VS 工程和 Renderer 分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 ShadowRenderer 拆分目的和后续 shadow material binder 方向。
80. 完成第三十次构建验证：
   - 首次构建发现 `pointLights.empty()` 早退误放入方向光 shadow 函数，已移动到 point light shadow 函数入口。
   - 再次执行 `Debug|x64 Build`，`ShadowRenderer.obj` 成功编译并参与链接。
   - 构建结果：成功，`0` error，`0` warning。
81. 完成第四十一轮 shadow 材质绑定迁移：
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，新增 `PhongShadowMaterial`、`PhongCSMShadowMaterial`、`PhongPointShadowMaterial` 的绑定分支。
   - `PhongCSMShadowMaterial` 的 CSM layer、shadow map array、light matrices、PCSS 参数绑定已迁入 `MaterialBinder`。
   - `PhongPointShadowMaterial` 的 point shadow texture array、point light far/near、directional fallback matrix 和 debug uniform 已迁入 `MaterialBinder`。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，删除 shadow 材质旧 case、旧 uniform helper 和不再需要的具体 shadow material include。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)，删除已迁移的 uniform helper 声明。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 shadow 材质绑定迁移对后续 PBR shadow 接入的意义。
82. 完成第三十一次构建验证：
   - 针对 shadow 材质绑定迁移后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`16` warning。
   - warning 来源仍为既有代码：Assimp loader 的有符号/无符号比较、Renderer 旧分支中的 include packing 和 `double -> float` 窄化转换；本轮迁移没有引入编译错误。
83. 完成第四十二轮 `ShadowResourceBinder` 抽象：
   - 新增 [renderer/ShadowResourceBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowResourceBinder.h) 与 [renderer/ShadowResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowResourceBinder.cpp)，集中封装 CSM shadow resource、point shadow resource 和 directional fallback shadow 参数绑定。
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，让 `PhongCSMShadowMaterial` 与 `PhongPointShadowMaterial` 调用 `ShadowResourceBinder`，不再直接维护 cascade layers、shadow map array、light matrices、point light far/near 等资源细节。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `ShadowResourceBinder` 纳入 VS 工程和 Renderer 分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录该抽象对后续 PBR shader 接入 shadow 的意义。
84. 完成第三十二次构建验证：
   - 针对 `ShadowResourceBinder` 抽象后执行真实 `Debug|x64 Build`。
   - `ShadowResourceBinder.obj` 成功编译并参与链接。
   - 构建结果：成功，`0` error，`0` warning。
85. 完成第四十三轮 PBR CSM shadow 接入：
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，在 `PBRMaterial` 绑定阶段调用 `ShadowResourceBinder::bindCSMShadowResources(...)`。
   - PBR shadow map 使用 texture unit `8`，避免和常见 PBR 贴图槽位冲突。
   - 更新 [shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag)，新增 CSM shadow uniforms、cascade layer 选择、shadow map array 采样和 3x3 PCF。
   - PBR 方向光 BRDF 现在会乘以 CSM shadow visibility，PBR 路径开始实际消费统一 shadow resource。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR CSM shadow 接入范围和验证边界。
86. 完成第三十三次构建与运行时 smoke 验证：
   - 针对 PBR CSM shadow 接入后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 本机未找到 `glslangValidator`，无法做离线 GLSL validator 检查。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 出现既有 `Failed to open logfile.`，未发现与本轮 PBR shader 相关的运行时编译/链接错误。
87. 完成第四十四轮简单材质绑定迁移：
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，新增 `WhiteMaterial`、`DepthMaterial`、`ScreenMaterial`、`CubeMaterial`、`CubeSphereMaterial` 的绑定分支。
   - `CubeMaterial` / `CubeSphereMaterial` 迁移时保留原有 skybox 行为：绑定前将 mesh position 同步到 camera position，并临时把 cube texture unit 设为 `0` 后恢复为 `2`。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，删除上述简单材质旧 case 和不再需要的 include。
   - 移除未使用的 `ScreenShot.h` include，并将剩余旧分支中的 `glfwGetTime()` 显式转换为 `float`，减少构建 warning 噪音。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录简单材质迁移和剩余旧分支边界。
88. 完成第三十四次构建验证：
   - 针对简单材质迁移和 warning 清理后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
89. 完成第四十五轮 Opacity / Env 材质绑定迁移：
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，新增 `OpacityMaskMaterial`、`PhongEnvMaterial`、`PhongEnvSphereMaterial` 的绑定分支。
   - `OpacityMaskMaterial` 的 diffuse、opacity mask、MVP、normal matrix、通用光照和 shininess 上传已迁入 `MaterialBinder`。
   - `PhongEnvMaterial` 与 `PhongEnvSphereMaterial` 的 diffuse、specular mask、environment sampler、矩阵和光照上传已迁入 `MaterialBinder`。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，删除上述三个旧 case 和不再需要的具体材质 include。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录本轮迁移和剩余 instanced 材质边界。
90. 完成第三十五次构建验证：
   - 针对 Opacity / Env 材质绑定迁移后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
91. 完成第四十六轮 instanced 材质绑定迁移：
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，新增 `PhongInstanceMaterial` 与 `GrassInstanceMaterial` 的绑定分支。
   - 新增 `setInstanceMatrixUniforms(...)`，统一维护 `matrices` uniform 上传和 `matricesUpdateState` shader 开关。
   - `PhongInstanceMaterial` 的 diffuse、specular mask、MVP、通用光照、shininess 和 instance matrix 状态上传已迁入 `MaterialBinder`。
   - `GrassInstanceMaterial` 的草地参数、风参数、云参数、透明 mask / cloud mask 绑定、`updateMatrices()` 和 instance matrix 状态上传已迁入 `MaterialBinder`。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，删除最后两个具体材质旧 case、相关 include、未使用的 `geometry` 临时变量和失效的历史绘制注释块。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录材质绑定整体剥离完成后的下一步 pass 拆分方向。
92. 完成第三十六次构建与运行时 smoke 验证：
   - 针对 instanced 材质迁移后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
93. 完成第四十七轮 `SceneRenderPass` 初步拆分：
   - 新增 [renderer/SceneRenderPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.h) 与 [renderer/SceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.cpp)，承接主 scene pass 的 mesh 绘制。
   - `SceneRenderPass` 现在负责 opaque / transparent 队列绘制、`RenderState::applyMaterialState(...)`、shader begin/end、`MaterialBinder::bind(...)` 和普通 / instanced draw call。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，删除 `drawMesh(...)` 与 `renderObject(...)`，主 `render(...)` 改为调度 shadow pass 后调用 `mSceneRenderPass.render(...)`。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)，移除对象绘制私有函数声明并新增 `SceneRenderPass` 成员。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `SceneRenderPass` 纳入 VS 工程和 Renderer 分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 scene pass 边界和后续 render queue / postprocess 拆分方向。
94. 完成第三十七次构建与运行时 smoke 验证：
   - 针对 `SceneRenderPass` 拆分后执行真实 `Debug|x64 Build`。
   - 首次构建结果：成功，`0` error，`10` warning。
   - warning 均来自既有 `application\assimpLoader.cpp` 与 `application\assimpInstanceLoader.cpp` 的有符号/无符号比较；新增 `renderer\SceneRenderPass.*` 与修改后的 `renderer\renderer.*` 未产生 warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
95. 完成第四十八轮 Assimp loader warning 清理：
   - 更新 [application/assimpLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\assimpLoader.cpp)，将与 Assimp unsigned 计数字段比较的循环索引从 `int` 改为 `unsigned int`。
   - 更新 [application/assimpInstanceLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\assimpInstanceLoader.cpp)，同步修正 mesh、child、vertex、face、index 循环索引类型。
   - 移除 [application/assimpInstanceLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\assimpInstanceLoader.cpp) 顶部重复的 `#include "assimpInstanceLoader.h"`。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 warning 清理原因和验证意义。
96. 完成第三十八次构建与运行时 smoke 验证：
   - 针对 Assimp loader warning 清理后再次执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 再次短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
97. 完成第四十九轮 `RenderQueue` 拆分：
   - 新增 [renderer/RenderQueue.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RenderQueue.h) 与 [renderer/RenderQueue.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RenderQueue.cpp)，承接 opaque / transparent 队列构建。
   - `RenderQueue` 现在负责清空队列、递归收集 scene 中的 `Mesh` / `InstancedMesh`、按材质透明状态分组，以及按相机深度排序 transparent 队列。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，主 `render(...)` 改为调用 `mRenderQueue.build(scene, camera)`，再把队列结果交给 `ShadowRenderer` 和 `SceneRenderPass`。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)，删除队列成员和 `projectObject(...)` 声明，新增 `RenderQueue` 成员。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `RenderQueue` 纳入 VS 工程和 Renderer 分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 `RenderQueue` 对后续 PBR / IBL 队列扩展的意义。
98. 完成第三十九次构建与运行时 smoke 验证：
   - 针对 `RenderQueue` 拆分后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
99. 完成第五十轮 `FrameRenderState` 拆分：
   - 新增 [renderer/FrameRenderState.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderState.h) 与 [renderer/FrameRenderState.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderState.cpp)，承接每帧开始时的 framebuffer 绑定、depth / stencil / blend / polygon offset 状态准备和 clear。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，主 `render(...)` 开头改为调用 `mFrameRenderState.begin(fbo)`。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)，新增 `FrameRenderState` 成员。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `FrameRenderState` 纳入 VS 工程和 Renderer 分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 frame state 边界对后续 PBR / IBL render target 和 pass 管理的意义。
100. 完成第四十次构建与运行时 smoke 验证：
   - 针对 `FrameRenderState` 拆分后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
101. 完成第五十一轮 `PostProcessPass` 初步拆分：
   - 新增 [renderer/PostProcessPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.h) 与 [renderer/PostProcessPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.cpp)，承接 MSAA resolve。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 与 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)，移除 `Renderer::msaaResolve(...)`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，通过 `PostProcessPass::resolveMultisample(...)` 执行 multisample framebuffer 到 resolve framebuffer 的 blit。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PostProcessPass` 纳入 VS 工程和 Renderer 分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录后处理边界对后续 Bloom / tone mapping / PBR render target 管理的意义。
102. 完成第四十一次构建与运行时 smoke 验证：
   - 针对 `PostProcessPass` 拆分后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
103. 完成第五十二轮 `FrameRenderTargets` 初步拆分：
   - 新增 [renderer/FrameRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderTargets.h) 与 [renderer/FrameRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderTargets.cpp)，统一持有 multisample scene framebuffer 与 resolved HDR framebuffer。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，移除 `framebufferMultisample` / `framebufferResolve` 运行时字段，改为通过 `frameRenderTargets.getSceneFbo()` 和 `frameRenderTargets.getMultisample()` / `getResolved()` 驱动渲染与 resolve。
   - 更新 [tools/sceneSetup/SceneSetup.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.h) 与 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，由 `FrameRenderTargets` 负责初始化主帧目标，并把 resolved color attachment 接到 `ScreenMaterial`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `FrameRenderTargets` 纳入 VS 工程和 Renderer 分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 render target orchestration 对 PBR / IBL 输出路径的意义。
104. 完成第四十二次构建与运行时 smoke 验证：
   - 针对 `FrameRenderTargets` 拆分后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
105. 完成第五十三轮 `PostProcessPass` screen composite 拆分：
   - 更新 [renderer/PostProcessPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.h) 与 [renderer/PostProcessPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.cpp)，新增 `renderScreenComposite(...)`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，移除 `renderer->render(sceneInScreen, ...)` 的屏幕输出调用，改由 `PostProcessPass` 直接绘制 screen quad 到默认 framebuffer。
   - 更新 [tools/sceneSetup/SceneSetup.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.h) 与 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，显式保留 `screenQuad` 引用，并继续挂到 `sceneInScreen` 供 hierarchy / inspector 使用。
   - `PostProcessPass` 现在负责 screen composite 的 framebuffer 绑定、viewport、基础后处理 GL state、exposure uniform、texture binding 和 fullscreen quad draw。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 screen composite 迁出普通 scene render path 的意义。
106. 完成第四十三次构建与运行时 smoke 验证：
   - 针对 `PostProcessPass` screen composite 拆分后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
107. 完成第五十四轮 `ScreenMaterial` 绑定清理：
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，删除 `ScreenMaterial` include、`bindScreenMaterial(...)` helper 和 `MaterialType::ScreenMaterial` 分支。
   - `ScreenMaterial` 的 screen texture、depth texture、exposure 和 fullscreen draw 相关绑定现在只保留在 `PostProcessPass::renderScreenComposite(...)`。
   - 普通 scene pass 不再需要知道 screen composite 的 shader uniform 细节。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 screen composite 从 scene material binding 中脱钩。
108. 完成第四十四次构建与运行时 smoke 验证：
   - 针对 `ScreenMaterial` 绑定清理后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
109. 完成第五十五轮 Bloom 与 framebuffer 解耦：
   - 更新 [framebuffer/framebuffer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framebuffer\framebuffer.cpp)，删除 `renderer/Bloom/Bloom.h` include 和 `Bloom::extractBright(...)` 实现。
   - 更新 [renderer/Bloom/Bloom.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\Bloom\Bloom.cpp)，将 `Bloom::extractBright(...)` 迁回 Bloom 模块，并补充空指针保护与 VAO 解绑。
   - 更新 [renderer/Bloom/Bloom.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\Bloom\Bloom.h)，将 `extractBright(...)` 暴露为后续 pass 可调用接口，并改用 `const std::shared_ptr<Framebuffer>&` 参数。
   - `Bloom` 的 mip level 计算增加显式 `static_cast<int>`，避免重新编译时的浮点到整数转换噪音。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 framebuffer 层不再反向依赖 renderer/Bloom。
110. 完成第四十五次构建与运行时 smoke 验证：
   - 针对 Bloom 与 framebuffer 解耦后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
111. 完成第五十六轮 Bloom bright extraction 接入：
   - 更新 [renderer/FrameRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderTargets.h) 与 [renderer/FrameRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderTargets.cpp)，新增 `bloomBright` HDR framebuffer 与对应 getter。
   - 更新 [tools/sceneSetup/SceneSetup.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.h) 与 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，在 render resource 初始化阶段创建 `Bloom` 实例。
   - 更新 [renderer/PostProcessPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.h) 与 [renderer/PostProcessPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.cpp)，新增 `extractBloomBright(...)` 调度入口。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，在 MSAA resolve 后执行 resolved HDR color 到 bloom bright target 的 bright extraction。
   - 当前只生成 bloom bright 中间结果，暂不做 blur/composite，避免在完整 Bloom 链路完成前改变最终画面输出。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 Bloom bright extraction 接入方式和后续边界。
112. 完成第四十六次构建与运行时 smoke 验证：
   - 针对 Bloom bright extraction 接入后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
113. 完成第五十七轮 Bloom blur ping-pong 接入：
   - 新增 [shaders/bloom/blur.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\bloom\blur.vert) 与 [shaders/bloom/blur.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\bloom\blur.frag)，实现 separable Gaussian blur。
   - 更新 [renderer/FrameRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderTargets.h) 与 [renderer/FrameRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderTargets.cpp)，新增 `bloomPing` / `bloomPong` HDR framebuffer 与 getter。
   - 更新 [renderer/Bloom/Bloom.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\Bloom\Bloom.h) 与 [renderer/Bloom/Bloom.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\Bloom\Bloom.cpp)，新增 `blurPingPong(...)` 和内部 `drawTextureToTarget(...)`。
   - 更新 [renderer/PostProcessPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.h) 与 [renderer/PostProcessPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.cpp)，新增 `blurBloom(...)` 调度入口。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，在 bright extraction 后执行 blur ping-pong；最终 screen composite 暂时仍只使用 resolved HDR color。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 Bloom blur 中间链路和后续 composite 边界。
114. 完成第四十七次构建与运行时 smoke 验证：
   - 针对 Bloom blur ping-pong 接入后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
115. 完成第五十八轮 Bloom screen composite 接入：
   - 更新 [materials/screenMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\screenMaterial.h) 与 [materials/screenMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\screenMaterial.cpp)，新增 `mBloomTexture`、`mBloomIntensity`、`mBloomEnabled`，并暴露到 inspector。
   - 更新 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，将 `FrameRenderTargets::getBloomPong()` 的 color attachment 接入 `ScreenMaterial`。
   - 更新 [renderer/PostProcessPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.cpp)，在 screen composite 中绑定 bloom texture，并上传 `enableBloom` 与 `bloomIntensity`。
   - 更新 [shaders/screen/screen.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\screen\screen.frag)，在 tone mapping 前合成 resolved HDR color 与 blurred bloom color。
   - 更新 [renderer/Bloom/Bloom.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\Bloom\Bloom.h)，将 bright threshold 默认值从 `0.0` 调整为 `1.0`。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 Bloom 链路闭环和后续参数系统化方向。
116. 完成第四十八次构建与运行时 smoke 验证：
   - 针对 Bloom screen composite 接入后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
117. 完成第五十九轮 `PostProcessSettings` 参数收敛：
   - 新增 [renderer/PostProcessSettings.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.h)，统一维护 exposure、tone mapping mode、Bloom 开关、Bloom threshold、Bloom intensity 和 Bloom iterations。
   - 更新 [materials/screenMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\screenMaterial.h) 与 [materials/screenMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\screenMaterial.cpp)，移除零散后处理字段并改用 `mSettings`，同时暴露新的 inspector 参数。
   - 更新 [tools/inspector/MaterialInspector.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\MaterialInspector.h)，新增 `Int` 属性类型，用于 tone mapping mode 与 Bloom iterations。
   - 更新 [renderer/Bloom/Bloom.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\Bloom\Bloom.h) 与 [renderer/Bloom/Bloom.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\Bloom\Bloom.cpp)，让 `extractBright(...)` 从外部接收 threshold。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，根据 `ScreenMaterial::mSettings` 控制 Bloom extract / blur，并使用配置的 threshold 与 iterations。
   - 更新 [renderer/PostProcessPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.h) 与 [renderer/PostProcessPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.cpp)，从统一 settings 上传 screen composite uniform。
   - 更新 [shaders/screen/screen.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\screen\screen.frag)，支持 exposure / Reinhard 两种 tone mapping mode。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PostProcessSettings.h` 纳入 VS 工程。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录后处理参数收敛结果和后续 profile / runtime 提升方向。
118. 完成第四十九次构建与运行时 smoke 验证：
   - 针对 `PostProcessSettings` 参数收敛后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout 未出现 `Shader Compile Error` 或 `Shader Link Error`；stderr 仍只有既有 `Failed to open logfile.`。
119. 完成第六十轮 `EnvironmentRenderTargets` 初步接入：
   - 新增 [renderer/EnvironmentRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentRenderTargets.h) 与 [renderer/EnvironmentRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentRenderTargets.cpp)，集中管理 IBL 所需 environment cubemap、irradiance cubemap、prefilter cubemap、BRDF LUT 和 capture FBO / RBO。
   - 新增 `EnvironmentRenderTargetSettings`，统一记录 IBL 资源尺寸、texture unit 和 prefilter mip 数量。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 与 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，让 `Renderer` 持有并初始化 `EnvironmentRenderTargets`，同时暴露 getter 给后续 IBL pass 使用。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将新 IBL 资源模块纳入 VS 工程和 Renderer 分类。
   - 本轮不修改 PBR shader 行为，只建立资源边界，避免把 environment capture / BRDF LUT 生成逻辑继续塞进 `Renderer` 或 framebuffer 层。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 EnvironmentRenderTargets 的职责边界和后续 IBL pass 顺序。
120. 完成第五十次构建与运行时 smoke 验证：
   - 针对 `EnvironmentRenderTargets` 初步接入后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning；`EnvironmentRenderTargets.obj` 已参与链接。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
121. 完成第六十一轮 `IBLPrecomputePass` 边界建立：
   - 新增 [renderer/IBLPrecomputePass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\IBLPrecomputePass.h) 与 [renderer/IBLPrecomputePass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\IBLPrecomputePass.cpp)，把 IBL 预计算流程从 `EnvironmentRenderTargets` 的资源所有权中拆出。
   - `IBLPrecomputePass` 目前提供 equirectangular HDR texture -> environment cubemap、environment cubemap -> irradiance cubemap、environment cubemap -> prefilter cubemap、BRDF LUT 四个调度入口。
   - 更新 [renderer/ShaderLibrary.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.h) 与 [renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)，新增 IBL utility shader 注册与 getter。
   - 新增 [shaders/ibl/capture.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\ibl\capture.vert)、[shaders/ibl/equirectangular_to_cubemap.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\ibl\equirectangular_to_cubemap.frag)、[shaders/ibl/irradiance_convolution.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\ibl\irradiance_convolution.frag)、[shaders/ibl/prefilter.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\ibl\prefilter.frag)、[shaders/ibl/brdf_lut.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\ibl\brdf_lut.vert)、[shaders/ibl/brdf_lut.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\ibl\brdf_lut.frag)，为后续 IBL 烘焙提供 shader 路径。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 与 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，让 `Renderer` 持有并暴露 `IBLPrecomputePass`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将新 pass 和 IBL shader 纳入 VS 工程分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 IBL 预计算 pass 与资源层的职责分离。
122. 完成第五十一次构建与运行时 smoke 验证：
   - 针对 `IBLPrecomputePass` 和 IBL shader 注册后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning；`IBLPrecomputePass.obj` 已参与链接。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
123. 完成第六十二轮 `EnvironmentProfile` 可选预计算入口：
   - 新增 [renderer/EnvironmentProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.h) 与 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp)，记录 HDR equirectangular path、HDR texture unit、是否 prepare 阶段预计算，并提供 `stbi_loadf` HDR texture loader。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 与 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，新增 IBL capture / BRDF shader getter 和 `precomputeEnvironment(...)` 调度入口。
   - 更新 [tools/sceneSetup/SceneSetup.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.h) 与 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，接入 `EnvironmentProfile`，在 profile 启用且有 HDR path 时创建 capture cube / BRDF quad 并触发 IBL 预计算。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，让 `AppRuntimeContext` 持有 `EnvironmentProfile` 并传入 scene setup。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `EnvironmentProfile` 纳入 VS 工程和 Renderer 分类。
   - 当前默认 `precomputeOnPrepare=false`，没有配置 HDR 文件时不会改变当前画面或启动路径。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 environment profile 入口和后续 PBR IBL sampler 绑定方向。
124. 完成第五十二次构建与运行时 smoke 验证：
   - 针对 `EnvironmentProfile` 可选预计算入口后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning；`EnvironmentProfile.obj` 已参与链接。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
125. 完成第六十三轮 PBR 可选 IBL 绑定路径：
   - 更新 [materials/pbrMaterial/PBRMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.h) 与 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)，新增 `Use IBL`、`IBL Diffuse Strength`、`IBL Specular Strength` 材质参数并暴露到 inspector。
   - 更新 [renderer/EnvironmentRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentRenderTargets.h) 与 [renderer/EnvironmentRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentRenderTargets.cpp)，增加 `hasPrecomputedEnvironment` 状态，防止 PBR 采样未烘焙完成的 IBL 贴图。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，在 `precomputeEnvironment(...)` 完整成功后标记 environment ready，并将 `EnvironmentRenderTargets` 传入 scene pass。
   - 更新 [renderer/SceneRenderPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.h) 与 [renderer/SceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.cpp)，把 environment resources 继续传给 `MaterialBinder`。
   - 更新 [renderer/MaterialBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.h) 与 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，在 PBR 材质启用 IBL 且 environment ready 时绑定 irradiance cubemap、prefilter cubemap 和 BRDF LUT。
   - 更新 [shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag)，新增 `useIBL`、IBL strength、irradiance / prefilter / BRDF LUT sampler 和可选 IBL ambient 计算。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR IBL sampler 绑定链路。
126. 完成第五十三次构建与运行时 smoke 验证：
   - 针对 PBR 可选 IBL 绑定路径后执行真实 `Debug|x64 Build`。
   - 构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
127. 完成第六十四轮 `EnvironmentProfile` UI 入口：
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 与 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，新增 `Renderer::precomputeEnvironment(const EnvironmentProfile&)` 高层入口，统一封装 HDR 加载、IBL capture cube / BRDF quad 创建和预计算调度。
   - 更新 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，让 prepare 阶段调用 `Renderer` 的 profile 入口，不再重复持有 IBL 资源准备细节。
   - 更新 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h) 与 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，在 controller 面板新增 `Environment / IBL` 区域，支持编辑 HDR path、HDR texture unit、prepare 预计算开关、查看 IBL ready 状态并手动触发预计算。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，将 `Renderer` 和 `EnvironmentProfile` 传入 DebugController context。
   - 默认没有 HDR path 且 `precomputeOnPrepare=false`，所以 UI 入口不会改变当前启动画面。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 EnvironmentProfile UI 入口和下一步验证方向。
128. 完成第五十四次构建与运行时 smoke 验证：
   - 针对 `EnvironmentProfile` UI 入口后执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
129. 完成第六十五轮 `EnvironmentProfile` 持久化入口：
   - 更新 [renderer/EnvironmentProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.h) 与 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp)，新增 `EnvironmentProfileStorage`，支持读取和写入 key-value 格式的 environment profile。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，在 `prepare()` 前加载 `config/environment_profile.local.ini`，让 HDR path 和 `precomputeOnPrepare` 能驱动 scene setup。
   - 更新 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h) 与 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，在 `Environment / IBL` 区域新增 profile 文件路径显示、Save 和 Reload 操作。
   - 新增 [config/environment_profile.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\environment_profile.example.ini)，记录可提交的 profile 字段示例。
   - 更新 [.gitignore](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\.gitignore)，忽略 `config/*.local.ini`，避免本地 HDR 路径和 UI 保存结果污染提交。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 example profile 纳入 VS 工程资源分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 EnvironmentProfile 持久化策略和下一步完整 IBL 验证方向。
130. 完成第五十五次构建与运行时 smoke 验证：
   - 针对 `EnvironmentProfile` 持久化入口后执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning；`EnvironmentProfile.obj` 重新编译并参与链接。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，在缺省没有 `config/environment_profile.local.ini` 时仍正常使用默认 profile；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
131. 完成第六十六轮 `PostProcessSettings` 提升到 runtime：
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，在 `AppRuntimeContext` 中新增 `PostProcessSettings postProcessSettings`，并让 `runFrame()` 使用它控制 Bloom extract / blur 和 screen composite。
   - 更新 [renderer/PostProcessPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.h) 与 [renderer/PostProcessPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessPass.cpp)，让 `renderScreenComposite(...)` 显式接收 `PostProcessSettings`，不再从 `ScreenMaterial` 读取后处理参数。
   - 更新 [materials/screenMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\screenMaterial.h) 与 [materials/screenMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\screenMaterial.cpp)，移除 `ScreenMaterial::mSettings`，保留 screen/depth/bloom texture 输入展示。
   - 更新 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h) 与 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，新增 `Post Process` 控制区，直接编辑 runtime-level settings。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录后处理参数从材质属性迁移到运行时配置的边界调整。
132. 完成第五十六次构建与运行时 smoke 验证：
   - 针对 `PostProcessSettings` runtime 提升后执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
133. 完成第六十七轮 `FrameRenderTargets` resize 生命周期：
   - 更新 [renderer/FrameRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderTargets.h) 与 [renderer/FrameRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\FrameRenderTargets.cpp)，新增 `resize(width, height)`，在窗口尺寸变化时重建 multisample scene target、resolved HDR target、Bloom bright / ping / pong targets。
   - `FrameRenderTargets::initialize(...)` 增加 0 尺寸保护，避免窗口最小化时创建非法 framebuffer。
   - `FrameRenderTargets` 新增 resolved depth-stencil attachment 和 Bloom pong color attachment getter，减少外部代码穿透到底层 framebuffer。
   - 更新 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，初始 screen pass 通过统一 getter 绑定 resolved color、resolved depth-stencil 和 Bloom pong texture。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，新增 `refreshPostProcessInputTextures()`，并让 `OnResize(...)` 更新全局尺寸、viewport、perspective camera aspect、frame render targets 和 screen material texture 输入。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 resize 生命周期接入和后续 camera/runtime 模块收敛方向。
134. 完成第五十七次构建、启动 smoke 与 resize smoke 验证：
   - 针对 `FrameRenderTargets` resize 生命周期后执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 通过 Win32 按进程枚举找到 `MyFirstWindow` 并执行两次 `SetWindowPos`，stdout 出现两次 `OnResize` 和旧 texture 删除输出；错误关键字扫描为空，说明 resize 回调与 framebuffer 重建路径已被实际触发。
135. 完成第六十八轮 `PostProcessSettings` 持久化入口：
   - 更新 [renderer/PostProcessSettings.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.h) 并新增 [renderer/PostProcessSettings.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.cpp)，加入 `PostProcessSettingsStorage`，支持保存和加载 exposure、tone mapping mode、Bloom 开关、threshold、intensity 和 iterations。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，在 `AppRuntimeContext` 中新增 `postProcessSettingsPath`，启动时在 `prepare()` 前尝试加载 `config/postprocess_settings.local.ini`。
   - 更新 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h) 与 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，在 `Post Process` 控制区新增 profile 文件路径显示、Save 和 Reload 操作。
   - 新增 [config/postprocess_settings.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\postprocess_settings.example.ini)，记录可提交的后处理配置字段示例。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PostProcessSettings.cpp` 和 example profile 纳入 VS 工程分类。
   - `config/postprocess_settings.local.ini` 被既有 `config/*.local.ini` ignore 规则覆盖，不会提交本地调参结果。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录后处理 profile 持久化策略和后续 HDR / IBL 验证方向。
136. 完成第五十八次构建与运行时 smoke 验证：
   - 针对 `PostProcessSettings` 持久化入口后执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning；`PostProcessSettings.obj` 已参与链接。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，在缺省没有 `config/postprocess_settings.local.ini` 时仍正常使用默认 settings；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
137. 完成第六十九轮 PBR 线性 HDR 输出修正：
   - 更新 [shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag)，删除 PBR shader 末尾的 Reinhard tone mapping 和 gamma correction。
   - PBR scene pass 现在输出线性 HDR color，交给统一 screen composite 根据 `PostProcessSettings` 执行 tone mapping、exposure 和 gamma。
   - 这避免 PBR 颜色在材质 shader 和 screen shader 中被重复 tone mapping / gamma，对后续 IBL、Bloom 和 HDR 调参更稳定。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR 输出空间与后处理链路的边界修正。
138. 完成第五十九次构建与运行时 shader smoke 验证：
   - 针对 PBR 线性 HDR 输出修正后执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning。
   - 短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。

### 当前状态

- 重构文档：`work.md` 已存在。
- 工作记录：`worked.md` 已建立。
- 第一轮实际代码重构：已完成。
- 第二轮入口收敛：已完成。
- 第三轮 `Application` 清理：已完成。
- 第四轮运行时上下文收拢：已完成。
- 第五轮 `prepare()` 分阶段拆分：已完成。
- 第六轮场景对象构建拆分：已完成。
- 当前工程可成功构建。
- 当前 `Renderer::renderObject()` 已不再保留具体材质旧分支，材质绑定整体收敛到 `MaterialBinder`。
- 当前 `Renderer` 已具备 shadow pass 与 scene pass 两个明确调度边界。
- 当前 render queue 构建已从 `Renderer` 拆出，后续可扩展 PBR / IBL 所需的队列类别。
- 当前 frame GL state 已从 `Renderer` 拆出，`Renderer` 进一步收敛为 frame state / render queue / shadow pass / scene pass 编排器。
- 当前 postprocess 边界已开始建立，MSAA resolve 已从 `Renderer` 移入 `PostProcessPass`。
- 当前主颜色 render target orchestration 已开始建立，multisample scene target 与 resolved HDR target 已收拢到 `FrameRenderTargets`。
- 当前 screen composite 已从普通 `Renderer::render(sceneInScreen, ...)` 路径移入 `PostProcessPass`，screen quad 只保留为 hierarchy / inspector 可见对象。
- 当前 `ScreenMaterial` 已从 `MaterialBinder` 中移除，screen composite shader 绑定只归 `PostProcessPass` 管理。
- 当前 Bloom 亮度提取实现已从 framebuffer 层移回 renderer/Bloom，framebuffer 不再反向依赖 Bloom。
- 当前 Bloom bright extraction 已接入运行时，resolved HDR color 会提取到 `FrameRenderTargets` 管理的 bloom bright target。
- 当前 Bloom blur ping-pong 已接入运行时，bright target 会经过 `bloomPing` / `bloomPong` 迭代模糊。
- 当前 Bloom composite 已接入 screen shader，resolved HDR color 会与 blurred bloom texture 合成后再 tone mapping / gamma。
- 当前后处理参数已收敛到 runtime-level `PostProcessSettings`，exposure、tone mapping mode、Bloom 开关、threshold、intensity、iterations 都能通过 DebugControllerPanel 修改，`ScreenMaterial` 只保留 postprocess 输入贴图。
- 当前 IBL / environment 资源边界已建立，`EnvironmentRenderTargets` 管理 environment / irradiance / prefilter cubemap、BRDF LUT 和 capture FBO。
- 当前 IBL 预计算流程已拆到 `IBLPrecomputePass`，并可通过 `EnvironmentProfile` 在 scene setup 阶段加载 HDR environment、创建 capture cube / BRDF quad 并触发预计算。
- 当前 PBR shader 已支持直接光 + 可选 IBL 组合，`MaterialBinder` 会在 PBR 材质启用 IBL 且 environment ready 时绑定 irradiance / prefilter / BRDF LUT。
- 当前 PBR shader 已输出线性 HDR color，不再在材质 shader 内部执行 tone mapping / gamma，最终显示转换统一交给 screen postprocess。
- 当前 `EnvironmentProfile` 已接入 DebugControllerPanel UI，可在运行时编辑 HDR path / texture unit、切换 prepare 预计算，并手动触发 IBL precompute。
- 当前 `EnvironmentProfile` 已支持 `config/environment_profile.local.ini` 本地保存 / 加载，UI 可保存和重载 profile；仓库保留 `config/environment_profile.example.ini` 作为字段示例。
- 当前 `FrameRenderTargets` 已支持窗口 resize 后重建 MSAA scene target、resolved HDR target 和 Bloom targets，并刷新 screen material 的 postprocess 输入贴图。
- 当前 `PostProcessSettings` 已支持 `config/postprocess_settings.local.ini` 本地保存 / 加载，UI 可保存和重载 profile；仓库保留 `config/postprocess_settings.example.ini` 作为字段示例。
- 当前剩余明显问题：工程内还没有默认 HDR environment 资源；PBR IBL 效果尚未用真实 HDR 资源验证；camera/runtime resize 行为仍在 `main.cpp` 中，尚未拆成独立模块。
- 下一步建议目标：引入或指定一份真实 HDR environment，验证 local profile 加载、启动预计算、PBR 材质启用 IBL 和最终画面效果的完整链路。

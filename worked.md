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
139. 完成第七十轮 Procedural HDR Environment 入口：
   - 更新 [renderer/EnvironmentProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.h) 与 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp)，为 `EnvironmentProfile` 新增 procedural environment 开关、分辨率、sky / ground / sun intensity 参数，并支持保存 / 加载这些字段。
   - 新增 `EnvironmentProfile::hasEnvironmentSource()` 和 `EnvironmentTextureLoader::loadEquirectangular(...)`，统一 HDR 文件加载与 procedural RGB16F equirectangular texture 生成入口。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 与 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，让 IBL precompute 支持 HDR path 或 procedural source 两种来源。
   - 更新 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，在 `Environment / IBL` UI 中新增 `Use Procedural Environment`、procedural width / height 和 sky / ground / sun intensity 控制。
   - 更新 [config/environment_profile.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\environment_profile.example.ini)，补充可提交的 procedural profile 字段示例。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 procedural source 作为无外部资源依赖的 IBL 验证入口。
140. 完成第六十次构建、默认启动 smoke 与 procedural IBL smoke 验证：
   - 针对 Procedural HDR Environment 入口执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning。
   - 默认短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 临时创建被 `.gitignore` 覆盖的 [config/environment_profile.local.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\environment_profile.local.ini)，设置 `useProceduralEnvironment=1`、`precomputeOnPrepare=1`、`proceduralWidth=128`、`proceduralHeight=64` 后再次短启动约 `10` 秒；错误关键字扫描为空。
   - 验证结束后已移除临时 local profile，避免改变用户后续手动运行的默认环境。
141. 完成第七十一轮 PBR Preview IBL 实验入口：
   - 更新 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，让默认 `PBR Preview Sphere` 的 `PBRMaterial` 设置 `mUseIBL=true`。
   - 该设置只表达 preview 材质希望消费 IBL；如果 environment 尚未预计算完成，`MaterialBinder` 仍会把 shader uniform `useIBL` 设为 `0`，所以缺省无 environment 的启动路径保持安全。
   - 配合上一轮 procedural environment，用户只需要启用 procedural source 并触发 precompute，即可让默认 PBR 测试球进入 IBL 采样路径。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR preview IBL 实验入口和后续 test scene / material preset 方向。
142. 完成第六十一次构建、默认启动 smoke 与 PBR IBL 绑定 smoke 验证：
   - 针对 PBR Preview IBL 实验入口执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning。
   - 默认短启动 `x64\Debug\text2.exe` 约 `6` 秒，在无 environment precompute 时仍未出现 shader / IBL 错误关键字。
   - 临时创建被 `.gitignore` 覆盖的 [config/environment_profile.local.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\environment_profile.local.ini)，设置 procedural source 与 `precomputeOnPrepare=1` 后短启动约 `10` 秒；错误关键字扫描为空，覆盖了 PBR preview 请求 IBL 且 environment ready 后的绑定路径。
   - 验证结束后已移除临时 local profile。
143. 完成第七十二轮 PBR Preview Profile 配置化：
   - 新增 [tools/sceneSetup/PBRPreviewProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.h) 与 [tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)，集中描述 PBR preview sphere 的启用开关、位置、半径、细分数、PBR surface 参数、IBL 强度和 normal map，并支持从 key-value local profile 保存 / 加载。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，在 `AppRuntimeContext` 中新增 `pbrPreviewProfile` 和 `pbrPreviewProfilePath`，启动时在 `prepare()` 前尝试加载 `config/pbr_preview.local.ini`。
   - 更新 [tools/sceneSetup/SceneSetup.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.h) 与 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，让 `preparePBRPreview(...)` 从 profile 读取材质和 mesh 参数，不再直接写死 albedo / metallic / roughness / position / sphere subdivision。
   - 新增 [config/pbr_preview.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_preview.example.ini)，记录可提交的 PBR preview preset 字段示例；本地 `config/pbr_preview.local.ini` 被既有 `config/*.local.ini` ignore 规则覆盖。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将新 profile 源文件、头文件和 example config 纳入 VS 工程分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR preview profile 化和后续多 material preset / 多球阵列方向。
144. 完成第六十二次构建、默认启动 smoke 与 PBR preview local profile smoke 验证：
   - 针对 PBR Preview Profile 配置化执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning；`PBRPreviewProfile.obj` 已参与链接。
   - 默认短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 临时创建被 `.gitignore` 覆盖的 [config/pbr_preview.local.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_preview.local.ini)，覆盖 preview sphere 位置、半径、细分数、albedo、metallic、roughness 和 IBL strength 后再次短启动约 `6` 秒；错误关键字扫描为空。
   - 验证结束后已移除临时 local profile，避免改变用户后续手动运行的默认 preview preset。
145. 完成第七十三轮 PBR Preview Material Grid：
   - 更新 [tools/sceneSetup/PBRPreviewProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.h) 与 [tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)，新增 `useMaterialGrid`、grid 行列数、spacing、grid radius、metallic range 和 roughness range，并支持保存 / 加载。
   - 更新 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，保留单球模式，同时在 grid 模式下复用同一个 sphere geometry 和 normal map，根据列插值 metallic、根据行插值 roughness，生成 `PBR Preview Mx Ry` 测试球阵列。
   - grid 行列数在 scene setup 中限制到 `1..10`，避免 local profile 写错导致一次创建过多 mesh。
   - 更新 [config/pbr_preview.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_preview.example.ini)，增加 5x5 PBR material grid 示例字段。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR preview material grid 和后续 experiment preset 方向。
146. 完成第六十三次构建、默认启动 smoke 与 grid + procedural IBL smoke 验证：
   - 针对 PBR Preview Material Grid 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error，`0` warning。
   - 默认短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 临时创建被 `.gitignore` 覆盖的 [config/pbr_preview.local.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_preview.local.ini) 和 [config/environment_profile.local.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\environment_profile.local.ini)，启用 `4x4` material grid 与 procedural IBL precompute 后短启动约 `10` 秒；错误关键字扫描为空。
   - 验证结束后已移除两个临时 local profile，避免改变用户后续手动运行的默认环境。
147. 完成第七十四轮 PBR Experiment Profile：
   - 新增 [tools/sceneSetup/PBRExperimentProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.h) 与 [tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)，提供高层 `config/pbr_experiment.local.ini` 覆盖入口。
   - experiment preset 支持 `environment.*`、`postprocess.*`、`pbrPreview.*` 前缀，可在一个文件中覆盖 procedural IBL、postprocess 参数和 PBR preview grid。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，启动时仍先加载原有三个 local profile，再加载 `PBRExperimentProfileStorage::defaultPath()` 作为最终覆盖层。
   - 新增 [config/pbr_experiment.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_experiment.example.ini)，提供 procedural IBL + postprocess + 5x5 PBR material grid 的组合示例。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将新 profile 源文件、头文件和 example config 纳入 VS 工程分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录高层 experiment preset 的加载顺序和后续 Debug UI 接入方向。
148. 完成第六十四次构建、默认启动 smoke 与 PBR experiment preset smoke 验证：
   - 针对 PBR Experiment Profile 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行全量 `Debug|x64 Build`，构建结果：成功，`0` error；因为此前清理过构建目录，本次重新编译暴露既有 camera double-to-float `C4244` warning，新增 `PBRExperimentProfile.obj` 已参与链接。
   - 默认短启动 `x64\Debug\text2.exe` 约 `6` 秒，stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 临时创建被 `.gitignore` 覆盖的 [config/pbr_experiment.local.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_experiment.local.ini)，用单个文件启用 procedural IBL、postprocess 和 `4x4` PBR material grid 后短启动约 `10` 秒；错误关键字扫描为空。
   - 验证结束后已移除临时 experiment local profile。
149. 完成第七十五轮 ProfileConfigParser 共享解析工具：
   - 新增 [tools/config/ProfileConfigParser.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigParser.h) 与 [tools/config/ProfileConfigParser.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigParser.cpp)，统一提供 `trim`、`startsWith`、`readKeyValueFile`、`parseBool`、`parseFloat`、`parseInt`、`parseUnsigned`。
   - 更新 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp)、[renderer/PostProcessSettings.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.cpp)、[tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)、[tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)，移除重复的 key-value 文件遍历和基础类型解析函数。
   - 各 profile 仍保留字段映射逻辑，只把通用文件解析和类型解析下沉到共享工具，避免过度抽象配置语义。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将共享 parser 纳入 VS 工程分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 profile parser 收敛和后续 schema 化方向。
150. 完成第六十五次轻量语法验证：
   - 针对 ProfileConfigParser 收敛执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - C 盘剩余空间约 `200MB`，不足以可靠执行完整 MSBuild 并生成 `text2/`、`x64/` 输出；本轮改用 MSVC `cl /Zs` 对 [tools/config/ProfileConfigParser.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigParser.cpp)、[renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp)、[renderer/PostProcessSettings.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.cpp)、[tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)、[tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp) 做无输出语法检查。
   - `cl /Zs` 结果：通过，未生成 obj/link 产物；本轮未执行完整运行时 smoke，原因是磁盘空间不足。
151. 完成第七十六轮冗余第三方归档清理：
   - 检查 [third_party.zip](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\third_party.zip) 的 Git 跟踪状态，确认它是已提交的大体积二进制归档。
   - 清理前使用 `rg -n "third_party\\.zip|third_party.zip" . -g '!third_party/**'` 搜索仓库引用，结果为空；当前工程直接使用解压后的 [third_party](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\third_party) 目录。
   - 从 Git 跟踪中删除未引用的 `third_party.zip`，释放本地空间并减少后续分支体积；保留 `third_party/` 目录和现有 include / lib 引用。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录这次清理对后续 PBR build / smoke 验证的意义。
152. 完成第六十六次清理验证：
   - 清理前确认工作树只有既有 [imgui.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\imgui.ini) 本地运行噪声。
   - 清理后 C 盘剩余空间从约 `1.95GB` 提升到约 `2.00GB`。
   - 清理后再次搜索 `third_party.zip`，除 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md) 与 [worked.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\worked.md) 的记录外，没有代码、工程文件或配置引用。
   - 本轮未修改运行时代码，因此不执行完整 MSBuild；后续进入 profile schema / 自动 UI 重构前再执行针对性构建和 smoke。
153. 完成第七十七轮 PropertyInspector 与 PBR Preview 自动 UI：
   - 新增 [tools/inspector/PropertyInspector.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\PropertyInspector.h)，从 [tools/inspector/MaterialInspector.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\MaterialInspector.h) 拆出通用 `PropertyBuilder`、`PropertyDescriptor` 和 `drawProperties(...)`。
   - `PropertyInspector` 新增可编辑 string 属性，路径类字段可以通过 descriptor 自动绘制 `InputText`，后续不需要每个面板手写固定 char buffer。
   - 更新 [tools/inspector/MaterialInspector.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\MaterialInspector.h)，保留材质类型名、贴图描述和材质 inspector 入口，通用属性绘制逻辑改为复用 `PropertyInspector`。
   - 更新 [tools/sceneSetup/PBRPreviewProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.h) 与 [tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)，新增 `visitEditableProperties(...)`，集中描述 enabled、position、geometry、material grid、PBR surface、IBL 和 normal map 配置。
   - 更新 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，把 `PBRPreviewProfile` 接入 Debug 面板并自动绘制 profile UI，支持保存 / 重载 `config/pbr_preview.local.ini`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PropertyInspector.h` 与既有 `MaterialInspector.h` 纳入 VS 工程分类。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录自动 UI 第一层落地和后续 schema 合并方向。
154. 完成第六十七次轻量语法验证：
   - 第一次 `cl /Zs` 因手动 include path 不完整失败，缺少 `Application.h` / `framebuffer.h` 等工程 include 路径；这不是代码错误。
   - 按 VS 工程 include 目录补齐 `application`、`camera`、`framework`、`framebuffer`、`light`、`materials`、`mesh`、`renderer`、`wrapper`、`legacy`、`third_party` 和 `third_party/stb_image` 后，使用 MSVC `cl /Zs` 检查 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)、[tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)、[materials/material.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\material.cpp)、[materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)。
   - `cl /Zs` 结果：通过，未生成 obj/link 产物。
155. 完成第六十八次完整构建与短启动 smoke 验证：
   - 针对 PropertyInspector 与 PBR Preview 自动 UI 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera / shadow camera double-to-float `C4244` warning，本轮新增代码未引入构建错误。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 构建输出目录 [text2](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2) 约 `158MB`、[x64](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64) 约 `113MB`，均为 ignored generated output；验证结束后已校验路径位于当前 workspace 内并清理，C 盘剩余空间恢复到约 `1.99GB`。
156. 完成第七十八轮 PropertySchema 与 Profile UI 继续收敛：
   - 新增 [tools/inspector/PropertySchema.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\PropertySchema.h)，将 `PropertyKind`、`PropertyDescriptor` 和 `PropertyBuilder` 从 ImGui 绘制层拆出，形成不依赖 ImGui 的字段描述层。
   - 更新 [tools/inspector/PropertyInspector.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\PropertyInspector.h)，保留 `drawProperties(...)` 作为 ImGui renderer，改为消费 `PropertySchema`。
   - 更新 [renderer/EnvironmentProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.h) 与 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp)，新增 `visitEditableProperties(...)` 描述 HDR source、texture unit、procedural source、precompute 和 procedural intensity 字段。
   - 更新 [renderer/PostProcessSettings.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.h) 与 [renderer/PostProcessSettings.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.cpp)，新增 `visitEditableProperties(...)` 描述 tone mapping 与 Bloom 字段。
   - 更新 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，Environment / Post Process 折叠区改为通过 `PropertyBuilder` 自动绘制字段 UI，面板只保留保存、重载、IBL precompute 和状态显示逻辑。
   - 更新 [tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)，profile 数据层改为只包含 `PropertySchema`，避免引入 ImGui 绘制层依赖。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj)、[text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters) 与 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，将新 schema 头文件纳入 VS 工程并记录后续方向。
157. 完成第六十九次 schema UI 构建与短启动 smoke 验证：
   - 针对 PropertySchema 与 Environment / Post Process profile UI 收敛执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp)、[renderer/PostProcessSettings.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)、[tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)，结果通过。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera / shadow camera double-to-float `C4244` warning，本轮新增代码未引入构建错误。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 构建输出目录 [text2](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2) 约 `160MB`、[x64](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64) 约 `114MB`，均为 ignored generated output；验证结束后已校验路径位于当前 workspace 内并清理，C 盘剩余空间恢复到约 `1.98GB`。
158. 完成第七十九轮 ProfileConfigIO 与 PostProcess schema 存取：
   - 更新 [tools/inspector/PropertySchema.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\PropertySchema.h)，为 `PropertyDescriptor` 增加 `configKey`，并为 `PropertyBuilder` 增加 `addConfigFloat`、`addConfigInt`、`addConfigBool`、`addConfigString`。
   - 新增 [tools/config/ProfileConfigIO.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.h) 与 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp)，根据 descriptor 的 `configKey`、字段类型和 getter / setter 执行通用 ini load/save。
   - 更新 [renderer/PostProcessSettings.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.cpp)，让 `visitEditableProperties(...)` 同时提供 UI 描述和 config key；`PostProcessSettingsStorage::loadFromFile(...)` / `saveToFile(...)` 改为通过 `ProfileConfigIO` 读写。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `ProfileConfigIO.cpp/.h` 纳入 VS 工程。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PostProcess schema 存取作为后续迁移 Environment / PBRPreview profile 的低风险验证点。
159. 完成第七十次 PostProcess schema 存取验证：
   - 针对 ProfileConfigIO 与 PostProcess schema 存取执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp) 与 [renderer/PostProcessSettings.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.cpp)，结果通过。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；新增 `ProfileConfigIO.obj` 已参与链接，仍存在既有 camera / shadow camera double-to-float `C4244` warning。
   - 临时编译 `%TEMP%/text2_schema_test/postprocess_schema_test.cpp`，只链接 [renderer/PostProcessSettings.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.cpp)、[tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp)、[tools/config/ProfileConfigParser.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigParser.cpp)，直接验证 `PostProcessSettingsStorage::loadFromFile(...)` 能读取 exposure / toneMapping / Bloom 字段，`saveToFile(...)` 能按 schema 写回相同 key；临时测试程序返回 `postprocess schema load/save ok`。
   - 构建输出目录 [text2](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2) 约 `161MB`、[x64](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64) 约 `114MB`，以及最小测试遗留的 ignored `.obj` 产物均已在验证结束后清理。
160. 完成第八十轮 EnvironmentProfile schema 存取：
   - 更新 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp)，让 `visitEditableProperties(...)` 为 HDR path、texture unit、procedural source、precompute 和 procedural intensity 字段提供 config key。
   - `EnvironmentProfileStorage::loadFromFile(...)` / `saveToFile(...)` 改为通过 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp) 消费同一份 property schema，不再维护手写 key 分支和手写输出逻辑。
   - 删除 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp) 对 `<filesystem>`、`<fstream>`、`<sstream>` 和 `ProfileConfigParser` 的直接依赖。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 environment 与 postprocess 都已迁移到 schema-driven 配置读写，并把下一步目标收敛到 `PBRPreviewProfile` 的 vec3 / 多 key 映射。
161. 完成第七十一次 EnvironmentProfile schema 存取验证：
   - 针对 EnvironmentProfile schema 存取执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp) 与 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp)，结果通过。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera / shadow camera double-to-float `C4244` warning。
   - 临时编译 `%TEMP%/text2_environment_schema_test/environment_schema_test.cpp`，链接 [renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp)、[framework/texture.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\framework\texture.cpp)、[tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp)、[tools/config/ProfileConfigParser.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigParser.cpp) 和 `glad.c`，直接验证 `EnvironmentProfileStorage::loadFromFile(...)` / `saveToFile(...)` 可按 schema 读写 HDR、procedural 和 precompute 字段；临时测试程序返回 `environment schema load/save ok`。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 构建输出目录 [text2](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2) 约 `160MB`、[x64](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64) 约 `114MB`，均为 ignored generated output；验证结束后清理。
162. 完成第八十一轮 PBRPreviewProfile schema 存取：
   - 更新 [tools/inspector/PropertySchema.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\inspector\PropertySchema.h)，为 `PropertyDescriptor` 增加 `configKeys`，并新增 `addConfigVec3(...)` / `addConfigColor3(...)`，支持一个 UI 属性对应多个 ini key。
   - 更新 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp)，支持按 component 读写 `Vec3` / `Color3` descriptor。
   - 更新 [tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)，让 enabled、position、geometry、grid、albedo、PBR surface、IBL 和 normal map 字段都在 `visitEditableProperties(...)` 中提供 config key。
   - `PBRPreviewProfileStorage::loadFromFile(...)` / `saveToFile(...)` 改为通过 `ProfileConfigIO` 消费同一份 property schema，不再维护手写 key 分支和手写输出逻辑。
   - 保留既有 ini 字段名：`positionX/Y/Z`、`albedoR/G/B` 等字段格式不变，避免破坏已有 `config/pbr_preview.local.ini` 和 experiment preset。
163. 完成第七十二次 PBRPreviewProfile schema 存取验证：
   - 针对 PBRPreviewProfile schema 存取执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp) 与 [tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)，结果通过。
   - 临时编译并运行 `__codex_tmp_pbr_preview_schema_test.cpp`，直接验证 `PBRPreviewProfileStorage::loadFromFile(...)` 可读取旧格式 `positionX/Y/Z`、`albedoR/G/B` 等拆分字段，`saveToFile(...)` 可通过 schema 写回并再次加载；测试输出 `pbr preview schema load/save ok`，测试源文件已删除。
   - 执行真实 `Debug|x64 Build`，首次构建成功但暴露新增 `ProfileConfigIO.cpp` 的 `C4267` warning；已通过显式转换到 `glm::vec3::length_type` 修复，随后 `cl /Zs` 和增量 `Debug|x64 Build` 均通过，增量构建结果 `0` warning、`0` error。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 验证结束后已校验路径并清理 generated output 目录 [text2](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2)、[x64](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64) 和临时测试目录 `%TEMP%/text2_pbr_preview_schema_test`。
164. 完成第八十二轮 PBRExperimentProfile schema 覆盖：
   - 更新 [tools/config/ProfileConfigIO.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.h) 与 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp)，新增 `applyPropertyConfigValue(...)`，让调用方可以把单个 key-value 应用到已有 property schema。
   - 更新 [tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)，为 environment、postprocess 和 PBR preview 的临时副本分别构建 `PropertyBuilder`，再把 `environment.*`、`postprocess.*`、`pbrPreview.*` 去掉前缀后交给对应 schema。
   - 删除 [tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp) 中手写维护的 environment / postprocess / PBR preview 字段分支。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 single-profile local ini 与 high-level experiment ini 已开始共享字段来源。
165. 完成第七十三次 PBRExperimentProfile schema 覆盖验证：
   - 针对 PBRExperimentProfile schema 覆盖执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp)、[tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)、[tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)、[renderer/EnvironmentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\EnvironmentProfile.cpp) 和 [renderer/PostProcessSettings.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PostProcessSettings.cpp)，结果通过。
   - 临时编译并运行 `__codex_tmp_pbr_experiment_schema_test.cpp`，直接验证 `PBRExperimentProfileStorage::loadFromFile(...)` 可通过 schema 应用 `environment.*`、`postprocess.*`、`pbrPreview.*` 覆盖，并验证 `enabled=0` 时不应用覆盖；测试输出 `pbr experiment schema load ok`，测试源文件已删除。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera / shadow camera double-to-float `C4244` warning，本轮未引入新的 schema / experiment profile warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 验证结束后已校验路径并清理 generated output 目录 [text2](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2)、[x64](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64) 和临时测试目录 `%TEMP%/text2_pbr_experiment_schema_test`。
166. 完成第八十三轮 PBR texture slot schema：
   - 更新 [materials/pbrMaterial/PBRMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.h)，新增 `PBRTextureSlot` / `PBRConstTextureSlot`，集中描述 PBR 贴图槽的 Inspector label、sampler uniform、use flag uniform 和 texture 指针。
   - 更新 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)，通过一份内部 metadata 表生成 texture slot 列表，并让 `visitEditableProperties(...)` 遍历 slot 列表生成贴图状态文本。
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，让 `bindPBRMaterial(...)` 遍历同一份 texture slot 列表绑定 sampler 和 use flag，不再手写 6 组 PBR 可选贴图绑定。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR texture slot schema 边界和后续扩展方向。
167. 完成第七十四次 PBR texture slot schema 验证：
   - 针对 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp) 与 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp) 执行 MSVC `cl /Zs`，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera / shadow camera double-to-float `C4244` warning，本轮未引入新的 PBR texture slot warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
168. 完成第八十四轮 PBR uniform slot schema：
   - 更新 [materials/pbrMaterial/PBRMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.h)，新增 `PBRVec3UniformSlot` / `PBRFloatUniformSlot` 及 const 版本，用于描述 PBR 参数的 Inspector label、shader uniform 名、字段指针和 float UI 范围。
   - 更新 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)，通过内部 metadata 表生成 vec3、surface float 和 IBL float uniform slot，并让 `visitEditableProperties(...)` 复用这些 slot 生成 PBR surface / IBL UI。
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，让 `bindPBRMaterial(...)` 遍历 uniform slot 写入 `pbrAlbedo`、`pbrMetallic`、`pbrRoughness`、`pbrAo`、`pbrEmissive*` 和 IBL strength uniform，不再手写这组 shader uniform。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR uniform slot schema 边界；`useIBL` 因依赖 environment readiness 暂时仍保留在 Binder 计算。
169. 完成第七十五次 PBR uniform slot schema 验证：
   - 针对 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp) 与 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp) 执行 MSVC `cl /Zs`，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera / shadow camera double-to-float `C4244` warning，本轮未引入新的 PBR uniform slot warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
170. 完成第八十五轮 PBR material profile：
   - 更新 [materials/pbrMaterial/PBRMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.h) 与 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)，新增 `PBRMaterialProfile`，集中保存 albedo、metallic、roughness、AO、emissive 和 IBL strength 等材质 preset 参数。
   - `PBRMaterialProfile` 新增 `visitEditableProperties(...)`，提供可复用的 PBR surface / IBL property schema，并保持既有 `albedoR/G/B`、`metallic`、`roughness`、`ao`、`useIBL` 等 config key 兼容。
   - `PBRMaterialProfile` 新增 `applyTo(...)` / `copyFrom(...)`，提供 profile 与真实 `PBRMaterial` 之间的转换入口。
   - 更新 [tools/sceneSetup/PBRPreviewProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.h) 与 [tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)，让 preview profile 持有 `PBRMaterialProfile material`，不再直接维护一套 PBR surface / IBL 字段。
   - 更新 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)，创建 PBR preview material 时先应用 `profile.material`，material grid 再覆盖 metallic / roughness。
   - 更新 [config/pbr_preview.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_preview.example.ini) 与 [config/pbr_experiment.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_experiment.example.ini)，补齐 emissive profile 字段示例。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR material profile 与运行时 `PBRMaterial` 的转换边界。
171. 完成第七十六次 PBR material profile 验证：
   - 使用 MSVC `cl /Zs` 检查 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)、[tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)、[tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)、[tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp) 和 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，结果通过。
   - 临时编译并运行 `__codex_tmp_pbr_material_profile_test.cpp`，直接验证 `PBRPreviewProfileStorage::loadFromFile(...)` 可加载旧字段和新增 emissive 字段，`saveToFile(...)` 可写回并再次加载，`PBRMaterialProfile::applyTo(...)` 可正确落到真实 `PBRMaterial`；测试输出 `pbr material profile config ok`，测试源文件已删除。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera / shadow camera double-to-float `C4244` warning，本轮未引入新的 PBR material profile warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
172. 完成第八十六轮 runtime viewport boundary：
   - 新增 [application/RuntimeViewport.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeViewport.h) 与 [application/RuntimeViewport.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeViewport.cpp)，集中处理 resize 尺寸校验、OpenGL viewport、PerspectiveCamera aspect、FrameRenderTargets resize 和 ScreenMaterial postprocess 输入贴图同步。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，让初始化 viewport、ImGui default framebuffer viewport 和 `OnResize(...)` 都走 `RuntimeViewport`；`main.cpp` 不再保留 `refreshPostProcessInputTextures()` 手写逻辑。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime viewport 新源码加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 resize-sensitive runtime 资源的后续挂载边界。
173. 完成第七十七次 runtime viewport boundary 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeViewport.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeViewport.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera control / shadow camera double-to-float `C4244` warning，本轮未引入 runtime viewport warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
174. 完成第八十七轮 runtime input boundary：
   - 新增 [application/RuntimeInputController.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeInputController.h) 与 [application/RuntimeInputController.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeInputController.cpp)，集中处理 scroll、keyboard、mouse、cursor 对 `CameraControl` 的分发。
   - 将原本写在 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 鼠标回调里的中键临时 FOV 缩放逻辑迁移到 `RuntimeInputController::applyTemporaryFovZoom(...)`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，GLFW 回调只保留 cursor position 获取和 debug log，再把输入事件转交给 `RuntimeInputController`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime input controller 新源码加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录后续 editor camera / preview camera 输入路由应挂到 runtime input 边界。
175. 完成第七十八次 runtime input boundary 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeInputController.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeInputController.cpp)、[application/RuntimeViewport.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeViewport.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera control / shadow camera double-to-float `C4244` warning，本轮未引入 runtime input warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
176. 完成第八十八轮 PBR material preset files：
   - 更新 [materials/pbrMaterial/PBRMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.h) 与 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)，新增 `PBRMaterialProfileStorage`，通过 `ProfileConfigIO` 复用 `PBRMaterialProfile` schema 保存 / 加载独立材质 preset。
   - 新增 [config/pbr_material.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_material.example.ini)，作为 `config/pbr_material.local.ini` 的字段示例。
   - 更新 [tools/sceneSetup/PBRPreviewProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.h) 与 [tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)，新增 `materialProfilePath`，并在加载 preview profile 后应用引用的 material preset。
   - 更新 [tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)，让 `pbrPreview.materialProfilePath` 在高层 experiment preset 中同样生效。
   - 更新 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，在 PBR Preview Profile 面板中新增保存 / 重载 PBR material profile 的按钮。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 material preset 示例加入 `资源文件\config`。
   - 更新 [config/pbr_preview.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_preview.example.ini) 与 [config/pbr_experiment.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_experiment.example.ini)，补充 `materialProfilePath` 示例字段。
177. 完成第七十九次 PBR material preset files 验证：
   - 使用 MSVC `cl /Zs` 检查 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)、[tools/sceneSetup/PBRPreviewProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRPreviewProfile.cpp)、[tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp) 与 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，结果通过。
   - 临时编译并运行 `__codex_tmp_pbr_material_preset_test.cpp`，验证 `PBRMaterialProfileStorage::saveToFile(...)` / `loadFromFile(...)` roundtrip，以及 `PBRPreviewProfileStorage::loadFromFile(...)` 可通过 `materialProfilePath` 应用独立 material preset；测试输出 `pbr material preset config ok`，测试源文件和编译产物已删除。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera control / shadow camera double-to-float `C4244` warning，本轮未引入 PBR material preset warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
178. 完成第八十九轮 PBR experiment Debug UI：
   - 更新 [tools/config/ProfileConfigIO.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.h) 与 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp)，新增公开 `writePropertyConfig(...)`，支持用指定 prefix 写出 property schema。
   - 更新 [tools/sceneSetup/PBRExperimentProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.h) 与 [tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)，新增 `saveToFile(...)`，将 environment、postprocess、PBR preview 保存成 prefixed experiment preset。
   - 更新 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h) 与 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，新增 `PBR Experiment Preset` 面板，可保存 / 重载组合 preset。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，把 `pbrExperimentProfilePath` 传入 `DebugControllerContext`。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR experiment preset 作为组合实验层的边界。
179. 完成第八十次 PBR experiment Debug UI 验证：
   - 使用 MSVC `cl /Zs` 检查 [tools/config/ProfileConfigIO.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\config\ProfileConfigIO.cpp)、[tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 临时编译并运行 `__codex_tmp_pbr_experiment_preset_test.cpp`，验证 `PBRExperimentProfileStorage::saveToFile(...)` 可保存 prefixed experiment preset，`loadFromFile(...)` 可还原 environment / postprocess / PBR preview，并能继续应用 `pbrPreview.materialProfilePath` 引用的独立 material preset；测试输出 `pbr experiment preset config ok`，测试源文件和编译产物已删除。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera control / shadow camera double-to-float `C4244` warning，本轮未引入 PBR experiment Debug UI warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
180. 完成第九十轮 PBR light rig profile：
   - 新增 [tools/sceneSetup/PBRLightRigProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRLightRigProfile.h) 与 [tools/sceneSetup/PBRLightRigProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRLightRigProfile.cpp)，集中描述 ambient、directional、spot 和 point light 的默认 PBR 实验灯光参数。
   - 更新 [tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp) 与 [tools/sceneSetup/SceneSetup.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.h)，让默认 light 创建改由 `PBRLightRigProfile::applyTo(...)` 驱动，不再在 scene setup 中硬编码灯光字段。
   - 更新 [tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp) 与 [tools/sceneSetup/PBRExperimentProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.h)，让 experiment preset 读写新增 `lightRig.*` prefixed key。
   - 更新 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 与 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h)，保存 PBR experiment preset 前先从运行时 light 回写 profile，重载 preset 后再应用到运行时 light。
   - `DebugControllerContext` 中 directional / ambient / spot light 改为指向 `shared_ptr` owner 的指针，避免 preset reload 只修改 context 副本。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，在 runtime context 中持有 `PBRLightRigProfile`，并传入 scene setup、experiment load 和 Debug UI。
   - 更新 [config/pbr_experiment.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_experiment.example.ini)，补齐 `lightRig.*` 示例字段。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PBRLightRigProfile` 加入 VS 工程。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR light rig profile 边界和下一步 camera rig 方向。
181. 完成第八十一次 PBR light rig profile 验证：
   - 使用 MSVC `cl /Zs` 检查 [tools/sceneSetup/PBRLightRigProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRLightRigProfile.cpp)、[tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)、[tools/sceneSetup/SceneSetup.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\SceneSetup.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 临时编译并运行 `__codex_tmp_pbr_light_rig_experiment_test.cpp`，验证 `PBRExperimentProfileStorage::saveToFile(...)` / `loadFromFile(...)` 可 roundtrip `lightRig.*`、environment、postprocess 和 PBR preview 字段；测试输出 `pbr light rig experiment config ok`，测试源文件和编译产物已删除。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera control / shadow camera double-to-float `C4244` warning，本轮未引入 PBR light rig warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
182. 完成第九十一轮 PBR camera rig profile：
   - 新增 [tools/sceneSetup/PBRCameraRigProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRCameraRigProfile.h) 与 [tools/sceneSetup/PBRCameraRigProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRCameraRigProfile.cpp)，集中描述主相机 position、up、right、fovy、nearPlane 和 farPlane。
   - 更新 [tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp) 与 [tools/sceneSetup/PBRExperimentProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.h)，让 experiment preset 读写新增 `cameraRig.*` prefixed key。
   - 更新 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 与 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h)，保存 PBR experiment preset 前先从当前主相机回写 camera rig，重载 preset 后再应用到当前主相机。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，在 runtime context 中持有 `PBRCameraRigProfile`，启动加载 experiment preset 后立即应用到主相机，并传入 Debug UI。
   - 更新 [config/pbr_experiment.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_experiment.example.ini)，补齐 `cameraRig.*` 示例字段。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PBRCameraRigProfile` 加入 VS 工程。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 camera rig profile 不接管 aspect，aspect 继续归 `RuntimeViewport` 管理。
183. 完成第八十二次 PBR camera rig profile 验证：
   - 使用 MSVC `cl /Zs` 检查 [tools/sceneSetup/PBRCameraRigProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRCameraRigProfile.cpp)、[tools/sceneSetup/PBRExperimentProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\sceneSetup\PBRExperimentProfile.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 临时编译并运行 `__codex_tmp_pbr_camera_rig_experiment_test.cpp`，验证 `PBRExperimentProfileStorage::saveToFile(...)` / `loadFromFile(...)` 可 roundtrip `cameraRig.*`，并验证 `PBRCameraRigProfile::applyTo(...)` / `copyFrom(...)` 对 `PerspectiveCamera` 的行为；测试输出 `pbr camera rig experiment config ok`，测试源文件和编译产物已删除。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera control / shadow camera double-to-float `C4244` warning，本轮未引入 PBR camera rig warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
184. 完成第九十二轮 runtime bootstrapper boundary：
   - 新增 [application/AppRuntimeContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\AppRuntimeContext.h)，将 renderer、scene、postprocess、environment、PBR preview、light rig、camera rig、camera、light 和 editor 相关运行时状态从 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 的本地 struct 移到 application 层。
   - 新增 [application/RuntimeBootstrapper.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeBootstrapper.h) 与 [application/RuntimeBootstrapper.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeBootstrapper.cpp)，集中执行 initialize、update loop、per-frame render、cleanup 和 destroy 生命周期骨架。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，让 `main()` 只设置日志等级并把 initialize / shouldContinue / runFrame / cleanup / destroy 回调交给 `RuntimeBootstrapper`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime context 与 bootstrapper 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 application lifecycle 第一层边界；具体 profile load、scene prepare 和 frame orchestration 暂时保留在 `main.cpp` 作为后续拆分目标。
185. 完成第八十三次 runtime bootstrapper boundary 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeBootstrapper.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeBootstrapper.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 临时编译并运行 `__codex_tmp_runtime_bootstrapper_test.cpp`，验证 bootstrapper 的 initialize -> shouldContinue / frame loop -> cleanup -> destroy 顺序，以及 initialize 失败时保持旧行为不执行 cleanup / destroy；测试输出 `runtime bootstrapper lifecycle ok`，测试源文件和编译产物已删除。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera control / shadow camera double-to-float `C4244` warning，本轮未引入 runtime bootstrapper warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
186. 完成第九十三轮 runtime profile loader：
   - 新增 [application/RuntimeProfileLoader.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeProfileLoader.h) 与 [application/RuntimeProfileLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeProfileLoader.cpp)，集中加载 environment、postprocess、PBR preview 和 PBR experiment profile。
   - `RuntimeProfileLoader::loadAll(...)` 保持原有分层顺序：先加载独立 profile，再加载高层 experiment preset 作为最终覆盖层。
   - Experiment preset 成功加载后，`RuntimeProfileLoader` 继续把 camera rig 应用到当前主相机，保持启动时恢复 PBR 观察视角的行为。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，删除 `loadEnvironmentProfile()`、`loadPostProcessSettings()`、`loadPBRPreviewProfile()` 和 `loadPBRExperimentProfile()` 四个本地函数，`initializeApplication()` 改为调用 `RuntimeProfileLoader::loadAll(gAppRuntime)`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 profile loader 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 profile loading 阶段已从主入口移入 application 层。
187. 完成第八十四次 runtime profile loader 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeProfileLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeProfileLoader.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 临时编译并运行 `__codex_tmp_runtime_profile_loader_test.cpp`，验证独立 profile 先加载、experiment preset 后覆盖，并验证 experiment 中的 camera rig 会应用到主相机；测试输出 `runtime profile loader layering ok`，测试源文件和编译产物已删除。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera control / shadow camera double-to-float `C4244` warning，本轮未引入 runtime profile loader warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
188. 完成第九十四轮 runtime scene preparer：
   - 新增 [application/RuntimeScenePreparer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeScenePreparer.h) 与 [application/RuntimeScenePreparer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeScenePreparer.cpp)，集中构建 `GL_SCENE::SetupContext` 并调用 `GL_SCENE::prepareDefaultScene(...)`。
   - 将 legacy experiment 的 `RuntimeContext` 构建、默认注释启用点和每帧 update 入口移入 `RuntimeScenePreparer`，保留默认不启用 legacy experiments 的旧行为。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，删除本地 `makeSceneSetupContext()`、`makeLegacyExperimentContext()`、`prepareLegacyExperiments()`、`updateLegacyExperiments()` 和 `prepare()`，`initializeApplication()` 改为调用 `RuntimeScenePreparer::prepare(...)`。
   - 新增 `RuntimeScenePrepareConfig`，把窗口尺寸、skybox texture path 和 legacy grass grid 参数作为 scene preparation 的过渡输入。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime scene preparer 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 scene preparation 阶段已从主入口收敛到 application 层。
189. 完成第八十五次 runtime scene preparer 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeScenePreparer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeScenePreparer.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；仍存在既有 camera / shadow camera double-to-float `C4244` warning，本轮未引入 runtime scene preparer warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
190. 完成第九十五轮 runtime frame runner：
   - 新增 [application/RuntimeFrameRunner.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFrameRunner.h) 与 [application/RuntimeFrameRunner.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFrameRunner.cpp)，集中执行 camera update、legacy experiment update、offscreen render、MSAA resolve、Bloom bright extraction / blur、screen composite 和 UI callback。
   - 新增 `RuntimeFrameConfig`，由 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 提供 default framebuffer width / height，避免 frame runner 直接依赖 `GL_APP`。
   - 新增 `RuntimeFrameCallbacks`，当前用于把 `renderIMGUI()` 作为 frame tail callback 传入，保持 UI host 与 frame pipeline 先解耦。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，让 `runFrame()` 收敛为一次 `RuntimeFrameRunner::run(...)` 调用。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime frame runner 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 frame orchestration 已从主入口移动到 application 层。
191. 完成第八十六次 runtime frame runner 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeFrameRunner.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFrameRunner.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；本轮增量构建未引入新的 runtime frame runner warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
192. 完成第九十六轮 runtime gui host：
   - 新增 [application/RuntimeGuiHost.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeGuiHost.h) 与 [application/RuntimeGuiHost.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeGuiHost.cpp)，集中处理 ImGui context 创建、backend 初始化、每帧 NewFrame / Render / RenderDrawData。
   - `RuntimeGuiHost::renderFrame(...)` 在提交 ImGui draw data 前调用 `RuntimeViewport::applyViewport(...)` 恢复 default framebuffer viewport，保留原本 UI 绘制前的 viewport 同步行为。
   - 新增 `RuntimeGuiFrameContext::drawPanels` callback，当前由 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 的 `drawEditorPanels()` 提供 editor 面板内容。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，删除直接 ImGui backend include、本地 `initIMGUI()` 和本地 `renderIMGUI()` host 逻辑，启动阶段改为 `RuntimeGuiHost::initialize(...)`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime gui host 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 ImGui host 生命周期已从主入口移动到 application 层。
193. 完成第八十七次 runtime gui host 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeGuiHost.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeGuiHost.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；本轮增量构建未引入新的 runtime gui host warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
194. 完成第九十七轮 runtime editor panel coordinator：
   - 新增 [application/RuntimeEditorPanelCoordinator.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeEditorPanelCoordinator.h) 与 [application/RuntimeEditorPanelCoordinator.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeEditorPanelCoordinator.cpp)，集中构建 `DebugControllerContext` 和 `EditorPanelContext`。
   - 将 DebugControllerPanel、HierarchyPanel、SelectionInspectorPanel 的绘制顺序和默认 selection 初始化移入 `RuntimeEditorPanelCoordinator::drawPanels(...)`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，删除本地 `makeDebugControllerContext()`、`makeEditorPanelContext()`，`drawEditorPanels()` 仅保留为 `RuntimeGuiHost` 的 callback wrapper。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime editor panel coordinator 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 editor panel 数据装配已从主入口移动到 application 层。
195. 完成第八十八次 runtime editor panel coordinator 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeEditorPanelCoordinator.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeEditorPanelCoordinator.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；本轮增量构建未引入新的 runtime editor panel coordinator warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
196. 完成第九十八轮 runtime camera lifecycle：
   - 新增 [application/RuntimeCameraLifecycle.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeCameraLifecycle.h) 与 [application/RuntimeCameraLifecycle.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeCameraLifecycle.cpp)，集中创建默认 `PerspectiveCamera` 和 `GameCameraControl`。
   - 新增 `RuntimeCameraConfig`，从窗口 width / height 计算初始 aspect，并保留 fovy / near / far 默认参数入口。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，删除本地 `prepareCamera()` 和直接 camera / camera control delete 逻辑，改为调用 `RuntimeCameraLifecycle::initializeDefaultCamera(...)` 与 `RuntimeCameraLifecycle::cleanup(...)`。
   - [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 不再直接 include `perspectivecamera.h`、`orthographiccamera.h` 或 `gamecameracontrol.h`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime camera lifecycle 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 camera lifecycle 已从主入口移动到 application 层。
197. 完成第八十九次 runtime camera lifecycle 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeCameraLifecycle.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeCameraLifecycle.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；本轮增量构建未引入新的 runtime camera lifecycle warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
198. 完成第九十九轮 runtime frame pipeline：
   - 新增 [application/RuntimeFramePipeline.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.h) 与 [application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp)，集中执行 scene-to-MSAA target、scene color resolve、Bloom 和 screen composite。
   - 更新 [application/RuntimeFrameRunner.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFrameRunner.cpp)，只保留 camera update、clear color、legacy experiment update、frame pipeline render 和 UI callback 这几个高层步骤。
   - 新增 `RuntimeFramePipelineConfig`，继续由 frame runner 传入 default framebuffer width / height，避免 pipeline 直接依赖 `GL_APP`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime frame pipeline 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 frame render pipeline 已从 frame runner 中独立出来。
199. 完成第九十次 runtime frame pipeline 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp) 与 [application/RuntimeFrameRunner.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFrameRunner.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；本轮增量构建未引入新的 runtime frame pipeline warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
200. 完成第一百轮 runtime window lifecycle：
   - 新增 [application/RuntimeWindowLifecycle.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeWindowLifecycle.h) 与 [application/RuntimeWindowLifecycle.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeWindowLifecycle.cpp)，集中执行 `GL_APP->init(...)`、callback context 绑定和 Application callback 注册。
   - 由于 `Application` 仍使用 C 风格函数指针，`RuntimeWindowLifecycle.cpp` 内部保存 `RuntimeWindowCallbackContext`，再由内部静态回调转发 resize / keyboard / mouse / cursor / scroll。
   - Resize callback 继续走 `RuntimeViewport::applyResize(...)`，同步 width / height、viewport、camera aspect、FrameRenderTargets 和 ScreenMaterial postprocess 输入贴图。
   - 输入 callback 继续走 `RuntimeInputController`，保留原有 scroll、keyboard、mouse 和 cursor 行为。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，删除 `setAndInitWindow()`、`OnScroll()`、`OnResize()`、`OnKeyboardCallback()`、`OnMouseCallback()`、`OnCursor()`、旧 `processInput()` 和旧 `keyCallBack()`；初始化改为调用 `RuntimeWindowLifecycle::initialize(...)`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime window lifecycle 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 window setup / callback glue 已从主入口移动到 application 层。
201. 完成第九十一次 runtime window lifecycle 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeWindowLifecycle.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeWindowLifecycle.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；本轮增量构建未引入新的 runtime window lifecycle warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
202. 完成第一百零一轮 main runtime alias cleanup：
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，移除对 `gAppRuntime` 各字段的大量 `auto&` alias，主入口不再直接展开 renderer、scene、material、light 和 profile 字段。
   - 移除 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 中未使用的旧实验变量和函数，包括 `scale`、`brigtnesee`、`angle`、`specularIntensity` 和旧 `moveit()`。
   - 新增本地 `MainStartupConfig`，集中保存 window size、skybox texture path、legacy grass grid 和 editor orbit angle。
   - 将 startup helper 和全局 runtime state 收入 anonymous namespace，减少主入口符号外泄。
   - 精简 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) include 列表，只保留当前入口实际依赖。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 main runtime alias cleanup。
203. 完成第九十二次 main runtime alias cleanup 验证：
   - 使用 MSVC `cl /Zs` 检查 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；本轮增量构建未引入新的 main cleanup warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
204. 完成第一百零二轮 runtime application shell：
   - 新增 [application/RuntimeApplicationShell.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.h) 与 [application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp)，集中持有 `AppRuntimeContext`、editor selection、legacy experiment runner 和 startup config。
   - `RuntimeApplicationShell::makeCallbacks()` 生成 `RuntimeBootstrapperCallbacks`，统一绑定 initialize / shouldContinue / runFrame / cleanup / destroy 生命周期。
   - 将原本在 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 的 window init、camera init、profile load、scene prepare、GUI init、frame run、camera cleanup 和 destroy 顺序移动到 `RuntimeApplicationShell`。
   - [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 现在只设置日志等级、创建 shell 并把 callbacks 交给 `RuntimeBootstrapper`。
   - `RuntimeFrameCallbacks::renderUi` 与 `RuntimeGuiFrameContext::drawPanels` 从函数指针升级为 `std::function`，支持 shell 使用成员函数 callback。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime application shell 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 runtime startup sequence 已聚合到 application shell。
205. 完成第九十三次 runtime application shell 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp)、[application/RuntimeFrameRunner.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFrameRunner.cpp)、[application/RuntimeGuiHost.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeGuiHost.cpp) 与 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；本轮增量构建未引入新的 runtime application shell warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
206. 完成第一百零三轮 runtime frame pass types：
   - 新增 [application/RuntimeFramePasses.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePasses.h) 与 [application/RuntimeFramePasses.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePasses.cpp)，定义 `RuntimeSceneColorPass`、`RuntimeSceneResolvePass`、`RuntimeBloomPass` 和 `RuntimeScreenCompositePass`。
   - 更新 [application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp)，只负责排列 pass 顺序，不再直接包含各 pass 的具体执行代码。
   - 更新 [application/RuntimeFramePipeline.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.h)，移除私有静态 pass 方法声明。
   - 当前 frame pass 行为保持不变：scene color 写入 MSAA target，resolve 到 HDR target，按 postprocess settings 运行 Bloom，再进行 screen composite。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime frame passes 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 frame pipeline 当前步骤已拆成显式 pass 类型。
207. 完成第九十四次 runtime frame pass types 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeFramePasses.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePasses.cpp) 与 [application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；本轮增量构建未引入新的 runtime frame pass warning。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
208. 完成第一百零四轮 runtime frame pipeline profile：
   - 新增 [application/RuntimeFramePipelineProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipelineProfile.h) 与 [application/RuntimeFramePipelineProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipelineProfile.cpp)，定义当前 frame pass 的启用开关和 schema 驱动的本地 ini 读写。
   - 更新 [application/AppRuntimeContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\AppRuntimeContext.h)，在 runtime context 中保存 frame pipeline profile 与默认 local 配置路径。
   - 更新 [application/RuntimeProfileLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeProfileLoader.cpp)，启动时加载 `config/runtime_frame_pipeline.local.ini`；缺失时保持全部 pass 默认开启。
   - 更新 [application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp)，每帧按 profile 决定 scene color、resolve、Bloom、screen composite 是否执行。
   - 更新 [tools/editor/DebugControllerPanel.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.h)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 与 [application/RuntimeEditorPanelCoordinator.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeEditorPanelCoordinator.cpp)，Debug UI 支持运行时切换、保存和重载 frame pipeline profile。
   - 新增 [config/runtime_frame_pipeline.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\runtime_frame_pipeline.example.ini)，记录可复制到 local 配置的 pass toggle 字段。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将新增 source/header/example config 加入 VS 工程和对应 filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 runtime frame pipeline profile 的职责边界和下一步 pass list 建议。
209. 完成第九十五次 runtime frame pipeline profile 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeFramePipelineProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipelineProfile.cpp)、[application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp)、[application/RuntimeProfileLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeProfileLoader.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)、[application/RuntimeEditorPanelCoordinator.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeEditorPanelCoordinator.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 编译并运行临时 roundtrip 测试，验证 `RuntimeFramePipelineProfileStorage::saveToFile()` / `loadFromFile()` 能正确保存和恢复四个 bool toggle；测试后已清理临时源文件和产物。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；新增 `RuntimeFramePipelineProfile.cpp` 已正确进入 VS 工程。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
210. 完成第一百零五轮 runtime frame pass registry：
   - 新增 [application/RuntimeFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePassRegistry.h) 与 [application/RuntimeFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePassRegistry.cpp)，用 `RuntimeFramePassDefinition` 描述 pass id、调试名、启用判断和统一执行函数。
   - 更新 [application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp)，主流程只遍历 `RuntimeFramePassRegistry::defaultPasses()` 并执行已启用 pass，不再直接读取 profile toggle 或调用具体 pass 类型。
   - 当前默认 pass list 保持 Scene Color、Scene Resolve、Bloom、Screen Composite 顺序，渲染行为预期不变。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime frame pass registry 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 pass registry 的职责边界和下一步 pass plan 建议。
211. 完成第九十六次 runtime frame pass registry 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePassRegistry.cpp)、[application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp)、[application/RuntimeFramePasses.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePasses.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；新增 `RuntimeFramePassRegistry.cpp` 已正确进入 VS 工程。
   - 短启动 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) 约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
212. 完成第一百零六轮 runtime PBR verification mode：
   - 新增 [application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h) 与 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，提供 `--verify-pbr` 使用的 deterministic PBR runtime profile、scene stats 输出和 default framebuffer PPM capture。
   - 更新 [application/RuntimeApplicationShell.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.h) 与 [application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp)，支持关闭 GUI、限制验证帧数、在指定帧导出 capture，并在 scene prepare 后报告 PBR scene stats。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，新增 `--verify-pbr` 参数；该模式使用 1280x720 窗口、禁用 ImGui，并启用 `out/pbr_verification.ppm` capture。
   - 验证 profile 会强制启用 procedural environment / IBL precompute、5x5 PBR material grid、normal map、PBR light rig、camera rig、postprocess 和全部 runtime frame pipeline pass，避免只验证旧 Phong 房间场景。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 runtime PBR verification source/header 加入 VS 工程和 Application filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR 验证模式、命令和验证边界。
213. 完成第九十七次 PBR verification mode 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)、[application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；新增 `RuntimePBRVerification.cpp` 已正确进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` 导出 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm)，文件头为 `P6 1280 720 255`，大小 `2764816` bytes。
   - 对导出的 PPM 做像素统计：`921600` 个像素，非黑像素 `921600`，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`，可排除空黑 framebuffer 或只启动旧场景的情况。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
214. 完成第一百零七轮 runtime frame pass plan：
   - 更新 [application/RuntimeFramePipelineProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipelineProfile.h) 与 [application/RuntimeFramePipelineProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipelineProfile.cpp)，新增 `passOrder` 字段并纳入 `PropertySchema` / `ProfileConfigIO` 保存加载。
   - 更新 [application/RuntimeFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePassRegistry.h) 与 [application/RuntimeFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePassRegistry.cpp)，为每个 pass 增加稳定 key，并新增 `findPassByKey()` / `buildPassPlan()`。
   - 更新 [application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp)，每帧从 `RuntimeFramePipelineProfile::passOrder` 构建 pass plan 并执行，而不是直接遍历固定 `defaultPasses()`。
   - `buildPassPlan()` 会解析逗号分隔 pass key，去重并忽略未知 token；如果没有任何有效 pass，则回退默认 Scene Color -> Scene Resolve -> Bloom -> Screen Composite 顺序。
   - 更新 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，`--verify-pbr` 显式设置完整 pass order，避免本地 passOrder 实验污染 PBR 验证。
   - 更新 [config/runtime_frame_pipeline.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\runtime_frame_pipeline.example.ini)，新增 `passOrder=SceneColor,SceneResolve,Bloom,ScreenComposite` 示例。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 profile-driven pass plan 的职责边界和后续 PBR pass 接入方式。
215. 完成第九十八次 runtime frame pass plan 验证：
   - 使用 MSVC `cl /Zs` 检查 [application/RuntimeFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePassRegistry.cpp)、[application/RuntimeFramePipeline.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipeline.cpp)、[application/RuntimeFramePipelineProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeFramePipelineProfile.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 编译并运行临时 roundtrip 测试，验证 `RuntimeFramePipelineProfileStorage::saveToFile()` / `loadFromFile()` 能正确保存和恢复 `passOrder` 与四个 bool toggle；测试后已清理临时源文件和产物。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；profile-driven pass plan 相关改动已进入 VS 工程构建。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 仍为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
216. 完成第一百零八轮 renderer PBR scene pass：
   - 更新 [renderer/RenderQueue.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RenderQueue.h) 与 [renderer/RenderQueue.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RenderQueue.cpp)，在保留全量 opaque / transparent 队列的同时新增 legacy / PBR 子队列。
   - 新增 [renderer/PBRSceneRenderPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.h) 与 [renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp)，只渲染 `PBRMaterial` mesh，并复用 `MaterialBinder` 绑定 PBR shader、IBL、light 和 shadow 资源。
   - 更新 [renderer/SceneRenderPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.h) 与 [renderer/SceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.cpp)，`render(...)` 返回实际 draw call 数，便于验证 legacy pass 绘制量。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 与 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，默认路径按 legacy opaque、PBR opaque、legacy transparent、PBR transparent 分段渲染；shadow pass 仍使用全量 opaque caster；`mGlobalMaterial` 不为空时继续回退原通用 scene pass。
   - 新增 `RendererFrameStats`，记录 `shadowCasterCount`、`legacySceneDrawCalls` 和 `pbrSceneDrawCalls`。
   - 更新 [application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 与 [application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp)，`--verify-pbr` capture 帧会输出 renderer stats。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 PBR scene render pass 加入 VS 工程和 renderer filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 renderer 侧 PBR scene pass 边界和验证方式。
217. 完成第九十九次 renderer PBR scene pass 验证：
   - 使用 MSVC `cl /Zs` 检查 [renderer/RenderQueue.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RenderQueue.cpp)、[renderer/SceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)、[application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；新增 `PBRSceneRenderPass.cpp` 已正确进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` 新增 renderer stats 输出：`shadowCasters=32, legacyDrawCalls=7, pbrDrawCalls=25`，证明 25 个 PBR preview mesh 已进入 PBR 专用 scene pass。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 仍为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
218. 完成第一百零九轮 renderer PBR depth prepass：
   - 新增 [renderer/PBRDepthPrepass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDepthPrepass.h) 与 [renderer/PBRDepthPrepass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDepthPrepass.cpp)，只渲染 PBR opaque mesh。
   - `PBRDepthPrepass` 使用 `DepthMaterial` shader 写入 depth，执行时关闭 color write，结束后恢复 color write。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 与 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，在 shadow pass 之后、legacy / PBR scene pass 之前执行 PBR depth prepass。
   - `RendererFrameStats` 新增 `pbrDepthPrepassDrawCalls`，用于区分 PBR depth prepass 和 PBR scene pass 的实际绘制数量。
   - 更新 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，`--verify-pbr` renderer stats 输出包含 `pbrDepthPrepassDrawCalls`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 PBR depth prepass 加入 VS 工程和 renderer filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR depth prepass 的执行位置和验证方式。
219. 完成第一百次 renderer PBR depth prepass 验证：
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRDepthPrepass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDepthPrepass.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)、[application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；新增 `PBRDepthPrepass.cpp` 已正确进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出：`shadowCasters=32, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`，证明 25 个 PBR preview mesh 已进入 PBR depth prepass 和 PBR scene pass。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 仍为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
220. 完成第一百一十轮 renderer frame stats Debug UI：
   - 更新 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，新增 `Renderer Frame Stats` UI 区块。
   - Debug UI 现在直接读取 `Renderer::getLastFrameStats()`，显示 `PBR Path Active`、shadow caster 数、legacy scene draw call 数、PBR depth prepass draw call 数和 PBR scene draw call 数。
   - 这让普通运行时也能观察当前场景是否真实进入 PBR 专用 depth / scene pass，不再只依赖旧 Phong 场景短启动或 `--verify-pbr` stdout。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 renderer frame stats Debug UI 的定位和验证边界。
221. 完成第一百零一次 renderer frame stats Debug UI 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)、[application/RuntimeEditorPanelCoordinator.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeEditorPanelCoordinator.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 执行真实 `Debug|x64 Build`，构建结果：成功，`0` error；`DebugControllerPanel.cpp` 已进入 VS/MSBuild 构建路径。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出：`shadowCasters=32, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`，证明当前验证场景确实经过 PBR depth prepass 和 PBR scene pass。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
222. 完成第一百一十一轮 renderer frame pass registry：
   - 新增 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)，将 `RendererFrameStats` 从 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 拆出，并新增 `rendererPassCount`。
   - 新增 [renderer/RendererFrameContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameContext.h)，集中传递 renderer 单帧执行所需的 scene、camera、light、queue、shader library、environment targets、pass 实例和 stats。
   - 新增 [renderer/RendererFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.h) 与 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，用 `RendererFramePassKey` 定义 renderer 内部 pass，并提供默认 pass plan 与 global material override pass plan。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，`Renderer::render()` 现在只创建 `RendererFrameContext`、选择 pass plan 并通过 registry 执行，不再直接硬编码 shadow、PBR depth、legacy scene、PBR scene 和 transparent pass 顺序。
   - 默认 pass plan 保持当前行为：`BeginFrame -> ShadowMaps -> PBRDepthPrepass -> LegacyOpaqueScene -> PBROpaqueScene -> LegacyTransparentScene -> PBRTransparentScene`。
   - global material override 行为保持为单独 pass plan：`BeginFrame -> ShadowMaps -> GlobalMaterialScene`。
   - 更新 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 与 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，输出 `rendererPassCount` 便于确认 registry pass plan 被执行。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 renderer frame pass registry/context/stats 文件加入 VS 工程和 renderer filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 renderer pass registry 的职责边界和后续 PBR pass 扩展方式。
223. 完成第一百零二次 renderer frame pass registry 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 初次 `Debug|x64` 并行构建遇到 `vc143.pdb` 并发写入问题；单进程构建又暴露 C 盘空间不足。清理本工程可再生成的中间产物后，使用 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `RendererFramePassRegistry.cpp` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出：`rendererPasses=7, shadowCasters=32, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`，证明默认 renderer pass plan 已执行且 PBR depth / scene pass 仍覆盖 25 个 PBR preview mesh。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
224. 完成第一百一十二轮 shadow render pass split：
   - 新增 [renderer/DirectionalShadowRenderPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\DirectionalShadowRenderPass.h) 与 [renderer/DirectionalShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\DirectionalShadowRenderPass.cpp)，负责 directional CSM shadow layer 绘制。
   - 新增 [renderer/PointShadowRenderPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PointShadowRenderPass.h) 与 [renderer/PointShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PointShadowRenderPass.cpp)，负责 point light cubemap array shadow face 绘制。
   - 新增 [renderer/ShadowMeshDraw.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowMeshDraw.h) 与 [renderer/ShadowMeshDraw.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowMeshDraw.cpp)，集中处理 shadow pass 共享的 postprocess-pass 判断和 mesh / instanced mesh draw。
   - 新增 [renderer/ShadowRenderStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowRenderStats.h)，记录 directional layer/draw call、point light/face/draw call。
   - 更新 [renderer/ShadowRenderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowRenderer.h) 与 [renderer/ShadowRenderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowRenderer.cpp)，`ShadowRenderer` 现在只作为 facade 调度 directional / point pass，并返回合并后的 shadow stats。
   - 更新 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp) 与 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)，将 shadow pass stats 汇总进 renderer frame stats。
   - 更新 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 与 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，Debug UI 和 `--verify-pbr` 会输出 shadow layer / face / draw call stats。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将新增 shadow pass 文件加入 VS 工程和 renderer filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 shadow render pass split 对 PBR shadow atlas 的准备意义。
225. 完成第一百零三次 shadow render pass split 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/DirectionalShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\DirectionalShadowRenderPass.cpp)、[renderer/PointShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PointShadowRenderPass.cpp)、[renderer/ShadowMeshDraw.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowMeshDraw.cpp)、[renderer/ShadowRenderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowRenderer.cpp)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过；因 C 盘空间仍低，继续关闭增量链接以避免 `.ilk` 膨胀。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出：`rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`，证明 shadow pass split 后 directional / point shadow 和 PBR depth / scene pass 均仍执行。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
226. 完成第一百一十三轮 light resource binder：
   - 新增 [renderer/LightResourceBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\LightResourceBinder.h) 与 [renderer/LightResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\LightResourceBinder.cpp)，集中绑定 forward lighting uniforms。
   - `LightResourceBinder::bindForwardLights(...)` 负责写入 spot light、directional light、point light array、`POINT_LIGHT_NUM` 和 ambient color。
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，移除内部 `setLightingUniforms(...)`，所有 Phong / PBR / shadow material 统一调用 `LightResourceBinder`。
   - `MaterialBinder` 不再直接依赖 `PointLightShadow` 获取点光数量常量，降低材质绑定与 shadow light 实现之间的耦合。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 LightResourceBinder 加入 VS 工程和 renderer filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 light resource binder 对 PBR lighting path 后续 UBO / clustered lighting 的准备意义。
227. 完成第一百零四次 light resource binder 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/LightResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\LightResourceBinder.cpp)、[renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp)、[renderer/SceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `LightResourceBinder.cpp` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出：`rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`，证明 lighting uniform 迁移后 PBR path、shadow pass 和 legacy draw calls 均保持正常。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
228. 完成第一百一十四轮 PBR material binder：
   - 新增 [renderer/PBRMaterialBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.h) 与 [renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)，集中绑定 PBR surface uniforms、texture slots、IBL uniforms、IBL textures、forward lights 和 CSM shadow resources。
   - 更新 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)，`PBRMaterial` 分支现在只转发到 `PBRMaterialBinder::bind(...)`，通用 material switch 不再直接维护 PBR shader uniform 细节。
   - `PBRMaterialBinder` 复用 `LightResourceBinder` 和 `ShadowResourceBinder`，保持 PBR 材质、灯光、shadow、IBL 资源边界清晰。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 PBRMaterialBinder 加入 VS 工程和 renderer filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR material binder 对后续 PBR light buffer、IBL debug、shadow atlas 和 BRDF 参数布局演进的准备意义。
229. 完成第一百零五次 PBR material binder 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)、[renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `PBRMaterialBinder.cpp` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出：`rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`，证明 PBR material binding 拆分后 PBR depth / scene pass、IBL 和 shadow 相关路径仍正常。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。

230. 完成第一百一十五轮 renderer mesh draw helper：
   - 新增 [renderer/MeshDraw.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MeshDraw.h) 与 [renderer/MeshDraw.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MeshDraw.cpp)，集中处理 mesh 空值 / geometry 空值检查、VAO 绑定、普通 indexed draw 和 instanced indexed draw。
   - 更新 [renderer/SceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp) 和 [renderer/PBRDepthPrepass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDepthPrepass.cpp)，删除各自重复的 `drawMesh(...)` 实现，统一调用 `MeshDraw::drawIndexed(...)`。
   - 更新 [renderer/ShadowMeshDraw.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowMeshDraw.cpp)，shadow 专用 helper 继续保留 postprocess-pass 判断，但实际 draw 动作转发到 `MeshDraw`。
   - 更新 [renderer/DirectionalShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\DirectionalShadowRenderPass.cpp) 与 [renderer/PointShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PointShadowRenderPass.cpp)，只在 mesh draw 成功后增加 shadow draw call 统计。
   - 更新 [renderer/IBLPrecomputePass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\IBLPrecomputePass.cpp)，cubemap face capture 与 BRDF LUT draw 复用同一个 `MeshDraw` 入口。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `MeshDraw.cpp/.h` 加入 VS 工程和 renderer filter。
231. 完成第一百零六次 renderer mesh draw helper 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/MeshDraw.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MeshDraw.cpp)、[renderer/SceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp)、[renderer/PBRDepthPrepass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDepthPrepass.cpp)、[renderer/ShadowMeshDraw.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowMeshDraw.cpp)、[renderer/DirectionalShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\DirectionalShadowRenderPass.cpp)、[renderer/PointShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PointShadowRenderPass.cpp) 和 [renderer/IBLPrecomputePass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\IBLPrecomputePass.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `MeshDraw.cpp` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
232. 完成第一百一十六轮 material binding context：
   - 新增 [renderer/MaterialBindingContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBindingContext.h)，集中承载 camera、directional light、spot light、point light list、ambient light 和 environment targets。
   - 更新 [renderer/SceneRenderPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.h) / [renderer/SceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.cpp)，legacy scene pass 不再逐项转发 camera / light / environment 参数。
   - 更新 [renderer/PBRSceneRenderPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.h) / [renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp)，PBR scene pass 通过同一个 binding context 调用材质绑定。
   - 更新 [renderer/MaterialBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.h)、[renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)、[renderer/PBRMaterialBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.h) 和 [renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)，外部 binding 接口改为接收 `MaterialBindingContext`。
   - 更新 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，renderer frame context 到 scene pass 的材质绑定参数组收敛为一次 `MaterialBindingContext` 构造。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `MaterialBindingContext.h` 加入 VS 工程和 renderer filter。
233. 完成第一百零七次 material binding context 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)、[renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)、[renderer/SceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\SceneRenderPass.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `MaterialBindingContext.h` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
234. 完成第一百一十七轮 depth prepass binder：
   - 新增 [renderer/DepthPrepassBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\DepthPrepassBinder.h) 与 [renderer/DepthPrepassBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\DepthPrepassBinder.cpp)，集中绑定 depth prepass 的 frame uniforms 和 per-object model matrix。
   - 更新 [renderer/PBRDepthPrepass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDepthPrepass.h) 与 [renderer/PBRDepthPrepass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDepthPrepass.cpp)，PBR depth pass 改为接收 `MaterialBindingContext`，不再直接维护 camera 参数和 depth shader uniform 写入细节。
   - 更新 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，PBR depth prepass 与 scene pass 复用同一套 material binding context 构造入口。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `DepthPrepassBinder.cpp/.h` 加入 VS 工程和 renderer filter。
235. 完成第一百零八次 depth prepass binder 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/DepthPrepassBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\DepthPrepassBinder.cpp)、[renderer/PBRDepthPrepass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDepthPrepass.cpp)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `DepthPrepassBinder.cpp` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
236. 完成第一百一十八轮 PBR IBL resource binder：
   - 新增 [renderer/PBRIBLResourceBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRIBLResourceBinder.h) 与 [renderer/PBRIBLResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRIBLResourceBinder.cpp)，集中处理 PBR IBL 可用性判断、`useIBL` uniform、IBL strength uniforms、irradiance / prefilter / BRDF LUT 贴图绑定和 max reflection LOD。
   - 更新 [renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)，移除内联 IBL resource binding 逻辑，改为调用 `PBRIBLResourceBinder::bind(...)`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PBRIBLResourceBinder.cpp/.h` 加入 VS 工程和 renderer filter。
237. 完成第一百零九次 PBR IBL resource binder 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRIBLResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRIBLResourceBinder.cpp)、[renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)、[renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `PBRIBLResourceBinder.cpp` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
238. 完成第一百一十九轮 PBR surface resource binder：
   - 新增 [renderer/PBRSurfaceResourceBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSurfaceResourceBinder.h) 与 [renderer/PBRSurfaceResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSurfaceResourceBinder.cpp)，集中处理 PBR surface vec3 uniforms、surface float uniforms、6 组可选 texture slots、对应 use flag 和 sampler binding。
   - 更新 [renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)，移除内联 surface uniform / texture binding 逻辑，改为调用 `PBRSurfaceResourceBinder::bind(...)`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PBRSurfaceResourceBinder.cpp/.h` 加入 VS 工程和 renderer filter。
239. 完成第一百一十次 PBR surface resource binder 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRSurfaceResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSurfaceResourceBinder.cpp)、[renderer/PBRIBLResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRIBLResourceBinder.cpp)、[renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)、[renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `PBRSurfaceResourceBinder.cpp` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
240. 完成第一百二十轮 PBR shadow resource binder：
   - 新增 [renderer/PBRShadowResourceBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowResourceBinder.h) 与 [renderer/PBRShadowResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowResourceBinder.cpp)，建立 PBR 专用 shadow resource binding 入口。
   - `PBRShadowResourceBinder` 当前委托 [renderer/ShadowResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowResourceBinder.cpp) 的 CSM shadow 绑定，并集中维护 PBR shadow texture unit `8`。
   - 当缺少 camera、directional light 或 directional shadow 时，`PBRShadowResourceBinder` 会显式写入 `csmLayerCount = 0`，让 PBR shader 跳过 shadow sampling。
   - 更新 [renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)，PBR material binding 不再直接调用通用 `ShadowResourceBinder::bindCSMShadowResources(...)`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PBRShadowResourceBinder.cpp/.h` 加入 VS 工程和 renderer filter。
241. 完成第一百一十一次 PBR shadow resource binder 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRShadowResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowResourceBinder.cpp)、[renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)、[renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `PBRShadowResourceBinder.cpp` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
242. 完成第一百二十一轮 PBR object uniform binder：
   - 新增 [renderer/PBRObjectUniformBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRObjectUniformBinder.h) 与 [renderer/PBRObjectUniformBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRObjectUniformBinder.cpp)，集中写入 PBR object-level uniforms。
   - `PBRObjectUniformBinder` 负责 `opacity`、`time`、`speed`、`cameraPosition`、`modelMatrix`、`viewMatrix`、`projectionMatrix` 和 `normalMatrix`。
   - 更新 [renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)，移除内联 object uniform 写入逻辑，改为调用 `PBRObjectUniformBinder::bind(...)`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，将 `PBRObjectUniformBinder.cpp/.h` 加入 VS 工程和 renderer filter。
243. 完成第一百一十二次 PBR object uniform binder 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRObjectUniformBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRObjectUniformBinder.cpp)、[renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)、[renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp)、[renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 `PBRObjectUniformBinder.cpp` 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
244. 完成第一百二十二轮 PBR scene pass direct binder：
   - 更新 [renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp)，PBR scene pass 不再通过通用 `MaterialBinder::bind(...)` switch 间接进入 PBR binding。
   - `PBRSceneRenderPass` 在确认 mesh 材质类型为 `PBRMaterial` 后，直接调用 `PBRMaterialBinder::bind(...)`。
   - 通用 [renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp) 仍保留 PBR 分支，用于 global material override 或 legacy scene pass 的兼容路径。
245. 完成第一百一十三次 PBR scene pass direct binder 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRSceneRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSceneRenderPass.cpp)、[renderer/PBRMaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRMaterialBinder.cpp)、[renderer/MaterialBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBinder.cpp) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式自动退出并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
246. 完成第一百二十三轮 renderer frame pass plan builder：
   - 更新 [renderer/RendererFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.h)，为 `RendererFramePassDefinition` 增加稳定 `keyName`，并新增 renderer pass order / find / build 接口。
   - 更新 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，新增 `defaultPassOrder()`、`globalMaterialOverridePassOrder()`、`findPassByKey(...)` 和 `buildPassPlan(...)`。
   - `buildPassPlan(...)` 会解析逗号分隔 pass order、忽略未知 token、去重重复 pass，并在没有有效 pass 时回退默认 renderer pass plan。
   - 更新 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，`Renderer::render(...)` 现在通过 pass order string 构建 renderer 内部 pass plan，再逐项执行。
   - 默认 renderer pass 顺序与 global material override 顺序保持不变；这一步只把后续 `PBRShadowAtlas`、`GBuffer`、`IBLDebug` 等 pass 的插入点稳定下来。
247. 完成第一百一十四次 renderer frame pass plan builder 验证：
   - 针对本轮改动执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 使用 MSVC `cl /Zs` 检查 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，结果通过。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过；只有预期的 `/EDITANDCONTINUE` 被 `/INCREMENTAL:NO` 覆盖提示。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式完成 capture 并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
248. 完成第一百二十四轮 renderer frame pass profile：
   - 新增 [renderer/RendererFramePassProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.h) 和 [renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp)，集中保存 renderer 默认 pass order 与 global material override pass order。
   - 新增 [config/renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini)，给出当前 renderer pass key 顺序示例。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，`Renderer` 现在持有 `RendererFramePassProfile`，并从 profile 读取 pass order 构建内部 pass plan。
   - 更新 [application/RuntimeProfileLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeProfileLoader.cpp)，启动时加载 `config/renderer_frame_pass.local.ini`；不存在本地配置时保留内建默认顺序。
   - 更新 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，新增 Renderer Frame Pass Plan 面板，可编辑、保存、重载和恢复内建默认 renderer pass order。
   - 更新 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，`--verify-pbr` 会强制重置 renderer pass profile，避免本地实验 pass order 干扰验证。
   - VS 工程已加入新增 profile 源文件、头文件和示例配置文件。
249. 完成第一百一十五次 renderer frame pass profile 验证：
   - 针对本轮新增 profile / UI / runtime wiring 执行 MSVC `cl /Zs` 检查，覆盖 [renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)、[application/RuntimeProfileLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeProfileLoader.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)、[application/RuntimeEditorPanelCoordinator.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeEditorPanelCoordinator.cpp) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，结果通过。
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，验证模式完成 capture 并输出 `PBR verification scene stats: objects=33, meshes=32, pbrMeshes=25, pbrPreviewMeshes=25, iblReady=yes`。
   - `--verify-pbr` renderer stats 输出保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
250. 完成第一百二十五轮 IBL debug renderer pass：
   - 新增 [renderer/IBLDebugPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\IBLDebugPass.h) 和 [renderer/IBLDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\IBLDebugPass.cpp)，可在当前 scene color target 绘制 IBL debug fullscreen view。
   - 新增 [shaders/diagnostics/ibl_debug.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\diagnostics\ibl_debug.vert) 和 [shaders/diagnostics/ibl_debug.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\diagnostics\ibl_debug.frag)，支持 environment、irradiance、prefilter 和 BRDF LUT debug view。
   - 更新 [renderer/ShaderLibrary.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.h) 和 [renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)，集中管理 IBL debug shader。
   - 更新 [renderer/RendererFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.h) 和 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，新增可选 pass key `IBLDebug`；默认 pass order 不包含它，本地 profile 可按需插入。
   - 更新 [renderer/RendererFramePassProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.h)、[renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp) 和 [config/renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini)，新增 `iblDebugMode`、`iblDebugMipLevel`、`iblDebugIntensity` 配置。
   - 更新 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，Debug UI 可观察 `iblDebugDrawCalls`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp)，新增 `--verify-pbr-ibl-debug` 验证入口。
   - VS 工程已加入新增 pass 源文件、头文件和 diagnostics shader 文件。
251. 完成第一百一十六次 IBL debug renderer pass 验证：
   - 使用 MSVC `cl /Zs` 检查 [renderer/IBLDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\IBLDebugPass.cpp)、[renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)、[renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)、[application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [renderer/IBLDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\IBLDebugPass.cpp) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，默认 PBR 验证保持 `rendererPasses=7`，说明默认 pass order 没有被 `IBLDebug` 改动污染。
   - 默认 `--verify-pbr` renderer stats 保持为 `rendererPasses=7, shadowCasters=32, directionalShadowLayers=5, directionalShadowDrawCalls=160, pointShadowLights=2, pointShadowFaces=12, pointShadowDrawCalls=384, pbrDepthPrepassDrawCalls=25, legacyDrawCalls=7, pbrDrawCalls=25`。
   - 默认 `--verify-pbr` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，验证模式临时插入 `IBLDebug` pass，并输出 `rendererPasses=8` 与 `iblDebugDrawCalls=1`。
   - `--verify-pbr-ibl-debug` 导出的 [out/pbr_ibl_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_ibl_debug_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计为 `921600` 个非黑像素，非黑比例 `100%`，RGB 均值约 `145.33 / 154.29 / 165.66`。
   - 补充执行普通短启动回归，约 `6` 秒后主动停止；stdout / stderr 未出现 `Shader Compile Error`、`Shader Link Error`、`Shader Load Error`、`Environment HDR Load Error`、`IBL precompute failed` 或 `Error:`；stderr 仍只有既有 `Failed to open logfile.`。
252. 完成第一百二十六轮 PBR G-buffer renderer pass：
   - 新增 [renderer/PBRGBufferRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferRenderTargets.h) 和 [renderer/PBRGBufferRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferRenderTargets.cpp)，集中管理 G-buffer FBO、三张 `RGBA16F` color attachment 和 depth attachment。
   - 新增 [renderer/PBRGBufferPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferPass.h) 和 [renderer/PBRGBufferPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferPass.cpp)，复用 `PBRObjectUniformBinder`、`PBRSurfaceResourceBinder` 与 `MeshDraw` 写入 PBR opaque mesh 的 G-buffer。
   - 新增 [shaders/pbr/pbr_gbuffer.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_gbuffer.vert) 和 [shaders/pbr/pbr_gbuffer.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_gbuffer.frag)，输出 position/roughness、normal/metallic、albedo/AO。
   - 更新 [renderer/ShaderLibrary.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.h) 和 [renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)，集中管理 PBR G-buffer shader。
   - 更新 [renderer/RendererFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.h) 和 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，新增可选 pass key `PBRGBuffer`；默认 pass order 不包含它，按 profile 或 verification 临时插入。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 和 [renderer/RendererFrameContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameContext.h)，把 G-buffer pass 与 render targets 接入 renderer frame context。
   - 更新 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，Debug UI 可观察 `pbrGBufferReady`、`pbrGBufferSize` 和 `pbrGBufferDrawCalls`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 `--verify-pbr-gbuffer` 验证入口。
   - 更新 [config/renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini) 与 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 `PBRGBuffer` 可选 pass 的插入方式和后续 deferred / debug 消费方向。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把新增 C++ 文件和 shader 文件加入 VS 工程。
253. 完成第一百一十七次 PBR G-buffer renderer pass 验证：
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRGBufferRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferRenderTargets.cpp)、[renderer/PBRGBufferPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferPass.cpp)、[renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)、[application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp)、[main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，结果通过。
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [renderer/PBRGBufferPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferPass.cpp) 与 [renderer/PBRGBufferRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferRenderTargets.cpp) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，默认 PBR 验证保持 `rendererPasses=7`，说明默认 pass order 没有被 `PBRGBuffer` 改动污染。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer`，验证模式临时插入 `PBRGBuffer` pass，并输出 `rendererPasses=8`、`pbrGBufferDrawCalls=25`、`pbrGBufferReady=yes`、`pbrGBufferSize=1280x720`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，已有 IBL debug 验证仍保持 `rendererPasses=8` 与 `iblDebugDrawCalls=1`。
   - `--verify-pbr` 与 `--verify-pbr-gbuffer` 导出的 [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) / [out/pbr_gbuffer_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_gbuffer_verification.ppm) 均为 `P6 1280 720 255`，大小 `2764816` bytes；像素统计均为非黑比例 `100%`，RGB 均值约 `166.93 / 126.55 / 81.78`。
   - `--verify-pbr-ibl-debug` 导出的 [out/pbr_ibl_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_ibl_debug_verification.ppm) 非黑比例 `100%`，RGB 均值约 `145.33 / 154.29 / 165.66`。
254. 完成第一百二十七轮 PBR G-buffer debug consumer：
   - 新增 [renderer/PBRGBufferDebugPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferDebugPass.h) 和 [renderer/PBRGBufferDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferDebugPass.cpp)，从 `PBRGBufferRenderTargets` 读取 G-buffer attachments 并绘制 fullscreen debug view。
   - 新增 [shaders/diagnostics/pbr_gbuffer_debug.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\diagnostics\pbr_gbuffer_debug.vert) 和 [shaders/diagnostics/pbr_gbuffer_debug.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\diagnostics\pbr_gbuffer_debug.frag)，支持 albedo、normal、roughness、metallic、AO、depth 和 world position debug mode。
   - 更新 [renderer/ShaderLibrary.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.h) 和 [renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)，集中管理 PBR G-buffer debug shader。
   - 更新 [renderer/RendererFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.h) 和 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，新增可选 pass key `PBRGBufferDebug`；该 pass 需要在 `PBRGBuffer` 后执行。
   - 更新 [renderer/RendererFramePassProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.h)、[renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp) 和 [config/renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini)，新增 `pbrGBufferDebugMode` 与 `pbrGBufferDebugIntensity` 配置。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 和 [renderer/RendererFrameContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameContext.h)，把 G-buffer debug pass 接入 renderer frame context。
   - 更新 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，Debug UI 可观察 `pbrGBufferDebugDrawCalls`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 `--verify-pbr-gbuffer-debug` 验证入口。
   - 修正 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 中 PBR preview grid 的位置，把验证材质球移到当前相机视野中心，避免只验证 draw call 而没有可见 PBR 内容。
   - 更新 [shaders/pbr/pbr.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.vert) 和 [shaders/pbr/pbr_gbuffer.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_gbuffer.vert)，固定 PBR attribute layout，使 forward PBR 与 G-buffer pass 共享稳定 VAO convention。
   - 更新 [renderer/PBRGBufferPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferPass.cpp)，每次绑定 G-buffer FBO 后显式设置 MRT draw buffers，并用 `glClearBufferfv` 清理 attachments，避免污染 renderer 全局 clear color。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把新增 pass 文件和 diagnostics shader 加入 VS 工程。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 G-buffer consumer 对后续 deferred lighting / G-buffer debug / clustered lighting 的准备意义。
255. 完成第一百一十八次 PBR G-buffer debug consumer 验证：
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRGBufferDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferDebugPass.cpp)、[renderer/PBRGBufferPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferPass.cpp)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，结果通过。
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [renderer/PBRGBufferDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferDebugPass.cpp) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，默认 PBR 验证保持 `rendererPasses=7`、`pbrDepthPrepassDrawCalls=25`、`pbrDrawCalls=25`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer`，G-buffer 验证保持 `rendererPasses=8`、`pbrGBufferDrawCalls=25`、`pbrGBufferReady=yes`、`pbrGBufferSize=1280x720`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，验证模式临时插入 `PBRGBuffer` 与 `PBRGBufferDebug`，输出 `rendererPasses=9`、`pbrGBufferDrawCalls=25`、`pbrGBufferDebugDrawCalls=1`。
   - `--verify-pbr-gbuffer-debug` 导出的 [out/pbr_gbuffer_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_gbuffer_debug_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；非黑比例约 `22.0009%`，RGB 均值约 `44.22 / 34.54 / 24.64`，证明 G-buffer debug pass 读到了可见 PBR grid 内容。
   - 补充执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，已有 IBL debug 验证仍保持 `rendererPasses=8` 与 `iblDebugDrawCalls=1`，导出图非黑比例 `100%`。
256. 完成第一百二十八轮 PBR deferred lighting consumer：
   - 新增 [renderer/PBRDeferredLightingPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.h) 和 [renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp)，从 `PBRGBufferRenderTargets` 读取 G-buffer attachments，并结合 camera、ambient、directional light、point lights 和 IBL resources 做 fullscreen PBR lighting。
   - 新增 [shaders/pbr/pbr_deferred_lighting.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_deferred_lighting.vert) 和 [shaders/pbr/pbr_deferred_lighting.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_deferred_lighting.frag)，复用 forward PBR 的 GGX / Smith / Schlick BRDF 计算方式。
   - 更新 [renderer/ShaderLibrary.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.h) 和 [renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)，集中管理 PBR deferred lighting shader。
   - 更新 [renderer/RendererFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.h) 和 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，新增可选 pass key `PBRDeferredLighting`；该 pass 需要在 `PBRGBuffer` 后执行。
   - 更新 [renderer/RendererFramePassProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.h)、[renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp) 和 [config/renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini)，新增 `pbrDeferredLightingIntensity`、`pbrDeferredIblDiffuseStrength`、`pbrDeferredIblSpecularStrength` 配置。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 和 [renderer/RendererFrameContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameContext.h)，把 deferred lighting pass 接入 renderer frame context。
   - 更新 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，Debug UI 可观察 `pbrDeferredLightingDrawCalls`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 `--verify-pbr-deferred` 验证入口。
   - `--verify-pbr-deferred` 使用 renderer pass order `BeginFrame,ShadowMaps,PBRDepthPrepass,PBRGBuffer,PBRDeferredLighting`，验证 deferred path 不依赖 `PBROpaqueScene` forward PBR pass。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把新增 pass 文件和 PBR shader 加入 VS 工程。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 deferred lighting consumer 的边界和后续 shadow atlas / clustered light list 演进方向。
257. 完成第一百一十九次 PBR deferred lighting consumer 验证：
   - 使用 MSVC `cl /Zs` 检查 [renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp)、[renderer/PBRGBufferPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferPass.cpp)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)、[main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，结果通过。
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，输出 `rendererPasses=5`、`pbrDrawCalls=0`、`pbrGBufferDrawCalls=25`、`pbrDeferredLightingDrawCalls=1`，证明本验证路径没有依赖 forward PBR scene pass。
   - `--verify-pbr-deferred` 导出的 [out/pbr_deferred_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_verification.ppm) 为 `P6 1280 720 255`，大小 `2764816` bytes；非黑比例约 `22.0043%`，RGB 均值约 `26.76 / 21.96 / 17.89`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，默认 PBR 验证保持 `rendererPasses=7`、`pbrDepthPrepassDrawCalls=25`、`pbrDrawCalls=25`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer`，G-buffer 验证保持 `rendererPasses=8`、`pbrGBufferDrawCalls=25`、`pbrGBufferReady=yes`、`pbrGBufferSize=1280x720`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，G-buffer debug 验证保持 `rendererPasses=9` 与 `pbrGBufferDebugDrawCalls=1`，导出图非黑比例约 `22.0009%`。
   - 补充执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，已有 IBL debug 验证仍保持 `rendererPasses=8` 与 `iblDebugDrawCalls=1`，导出图非黑比例 `100%`。
258. 完成第一百二十九轮共享 PBR lighting shader include：
   - 新增 [shaders/pbr/pbr_lighting.glsl](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_lighting.glsl)，集中保存 GGX distribution、Smith geometry、Schlick Fresnel、direct PBR lighting 和 IBL ambient 计算。
   - 更新 [shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag)，删除 forward PBR shader 中重复的 BRDF / IBL 函数，改为在 PBR / IBL uniforms 后 include `pbr_lighting.glsl`。
   - 更新 [shaders/pbr/pbr_deferred_lighting.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_deferred_lighting.frag)，删除 deferred shader 中同一套重复函数，改为 include 共享实现。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把 `pbr_lighting.glsl` 加入 VS 工程和 PBR shader filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 forward / deferred PBR shader 共享 lighting math 的后续维护边界。
259. 完成第一百二十次共享 PBR lighting shader include 验证：
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [shaders/pbr/pbr_lighting.glsl](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_lighting.glsl) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，forward PBR shader include 路径正常，renderer stats 保持 `rendererPasses=7`、`pbrDepthPrepassDrawCalls=25`、`pbrDrawCalls=25`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，deferred PBR shader include 路径正常，renderer stats 保持 `rendererPasses=5`、`pbrDrawCalls=0`、`pbrGBufferDrawCalls=25`、`pbrDeferredLightingDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，G-buffer debug 验证保持 `rendererPasses=9`、`pbrGBufferDrawCalls=25`、`pbrGBufferDebugDrawCalls=1`。
   - [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 非黑比例 `100%`，RGB 均值约 `160.99 / 123.69 / 83.49`。
   - [out/pbr_deferred_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_verification.ppm) 非黑比例约 `22.0043%`，RGB 均值约 `26.76 / 21.96 / 17.89`。
   - [out/pbr_gbuffer_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_gbuffer_debug_verification.ppm) 非黑比例约 `22.0009%`，RGB 均值约 `44.22 / 34.54 / 24.64`。
260. 完成第一百三十轮 PBR deferred CSM shadow sampling：
   - 新增 [shaders/pbr/pbr_csm_shadow.glsl](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_csm_shadow.glsl)，集中保存 CSM layer selection、shadow bias 和 3x3 PCF sampling。
   - 更新 [shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag)，删除 forward PBR shader 中原本内嵌的 CSM helper，改为 include `pbr_csm_shadow.glsl`。
   - 更新 [shaders/pbr/pbr_deferred_lighting.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_deferred_lighting.frag)，include `pbr_csm_shadow.glsl`，并用 `calculateCsmShadow(...)` 调制 directional light direct lighting。
   - 更新 [renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp) 和 [renderer/PBRDeferredLightingPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.h)，deferred lighting pass 现在写入 `viewMatrix`，复用 `PBRShadowResourceBinder` 绑定已有 CSM shadow resources，并返回 shadow binding stats。
   - 更新 [renderer/PBRShadowResourceBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowResourceBinder.h) 和 [renderer/PBRShadowResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowResourceBinder.cpp)，新增 `getCsmLayerCount(...)`，让 verification / stats 能直接报告 deferred path 使用的 CSM layer 数。
   - 更新 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，Debug UI 与 `--verify-pbr-deferred` 可显示 `pbrDeferredCsmShadowBound` 和 `pbrDeferredCsmShadowLayers`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把 `pbr_csm_shadow.glsl` 加入 VS 工程和 PBR shader filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 deferred PBR 当前复用现有 CSM shadow 资源、后续再演进 shadow atlas 的边界。
261. 完成第一百二十一次 PBR deferred CSM shadow sampling 验证：
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [shaders/pbr/pbr_csm_shadow.glsl](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_csm_shadow.glsl) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，输出 `rendererPasses=5`、`pbrDrawCalls=0`、`pbrGBufferDrawCalls=25`、`pbrDeferredLightingDrawCalls=1`、`pbrDeferredCsmShadowBound=yes`、`pbrDeferredCsmShadowLayers=5`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，forward PBR 验证保持 `rendererPasses=7`、`pbrDepthPrepassDrawCalls=25`、`pbrDrawCalls=25`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，G-buffer debug 验证保持 `rendererPasses=9`、`pbrGBufferDrawCalls=25`、`pbrGBufferDebugDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，IBL debug 验证保持 `rendererPasses=8` 与 `iblDebugDrawCalls=1`。
   - [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 非黑比例 `100%`，RGB 均值约 `160.99 / 123.69 / 83.49`。
   - [out/pbr_deferred_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_verification.ppm) 非黑比例约 `22.0043%`，RGB 均值约 `26.76 / 21.96 / 17.89`。
   - [out/pbr_gbuffer_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_gbuffer_debug_verification.ppm) 非黑比例约 `22.0009%`，RGB 均值约 `44.22 / 34.54 / 24.64`。
   - [out/pbr_ibl_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_ibl_debug_verification.ppm) 非黑比例 `100%`，RGB 均值约 `145.33 / 154.29 / 165.66`。
262. 完成第一百三十一轮 PBR deferred light buffer：
   - 新增 [renderer/PBRDeferredLightBuffer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightBuffer.h) 和 [renderer/PBRDeferredLightBuffer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightBuffer.cpp)，把 directional light、ambient light 和最多 16 个 point lights 打包到 `GL_SHADER_STORAGE_BUFFER`。
   - 更新 [shaders/pbr/pbr_deferred_lighting.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_deferred_lighting.frag)，删除 `DirectionalLight` / `PointLight` uniform 数组，改为通过 `layout(std430, binding = 3)` 读取 deferred light buffer。
   - 更新 [renderer/PBRDeferredLightingPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.h) 和 [renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp)，deferred pass 每帧绑定一次 `PBRDeferredLightBuffer`，不再逐字段写入 point light uniforms。
   - 更新 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，Debug UI 与 `--verify-pbr-deferred` 可显示 `pbrDeferredLightBufferBound` 和 `pbrDeferredLightBufferPointLights`。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把 `PBRDeferredLightBuffer` 加入 VS 工程和 renderer filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 deferred light data 从 uniform list 迁移到 SSBO 的边界。
263. 完成第一百二十二次 PBR deferred light buffer 验证：
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [renderer/PBRDeferredLightBuffer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightBuffer.cpp) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，输出 `rendererPasses=5`、`pbrDrawCalls=0`、`pbrGBufferDrawCalls=25`、`pbrDeferredLightingDrawCalls=1`、`pbrDeferredCsmShadowBound=yes`、`pbrDeferredLightBufferBound=yes`、`pbrDeferredLightBufferPointLights=2/16`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，forward PBR 验证保持 `rendererPasses=7`、`pbrDepthPrepassDrawCalls=25`、`pbrDrawCalls=25`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，G-buffer debug 验证保持 `rendererPasses=9`、`pbrGBufferDrawCalls=25`、`pbrGBufferDebugDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，IBL debug 验证保持 `rendererPasses=8` 与 `iblDebugDrawCalls=1`。
   - [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 非黑比例 `100%`，RGB 均值约 `160.99 / 123.69 / 83.49`。
   - [out/pbr_deferred_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_verification.ppm) 非黑比例约 `22.0043%`，RGB 均值约 `26.76 / 21.96 / 17.89`。
   - [out/pbr_gbuffer_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_gbuffer_debug_verification.ppm) 非黑比例约 `22.0009%`，RGB 均值约 `44.22 / 34.54 / 24.64`。
   - [out/pbr_ibl_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_ibl_debug_verification.ppm) 非黑比例 `100%`，RGB 均值约 `145.33 / 154.29 / 165.66`。
264. 完成第一百三十二轮 PBR shadow atlas resource layout：
   - 新增 [renderer/PBRShadowAtlasRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderTargets.h) 和 [renderer/PBRShadowAtlasRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderTargets.cpp)，集中管理后续 PBR shadow atlas pass 会使用的 FBO、directional CSM depth texture array 和 point shadow depth texture array。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 和 [renderer/RendererFrameContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameContext.h)，把 `PBRShadowAtlasRenderTargets` 挂入 renderer lifetime 和 frame context。
   - 更新 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，`ShadowMaps` pass 会 prepare atlas resources，并把当前 directional layer count / point face count 写入 frame stats。
   - 更新 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，Debug UI 与 `--verify-pbr*` 可观察 `pbrShadowAtlasReady`、directional layers 和 point faces。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把 `PBRShadowAtlasRenderTargets` 加入 VS 工程和 renderer filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录本轮只建立 PBR shadow atlas 资源所有权和可观测状态，暂不替换现有 shadow 渲染/采样。
265. 完成第一百二十三次 PBR shadow atlas resource layout 验证：
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [renderer/PBRShadowAtlasRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderTargets.cpp) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，输出 `pbrShadowAtlasReady=yes`、`pbrShadowAtlasDirectionalLayers=5`、`pbrShadowAtlasPointFaces=12`，同时 deferred path 保持 `pbrDeferredLightingDrawCalls=1`、`pbrDeferredCsmShadowBound=yes`、`pbrDeferredLightBufferBound=yes`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，forward PBR 验证保持 `rendererPasses=7`、`pbrDepthPrepassDrawCalls=25`、`pbrDrawCalls=25`，并输出 `pbrShadowAtlasReady=yes`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，G-buffer debug 验证保持 `rendererPasses=9`、`pbrGBufferDrawCalls=25`、`pbrGBufferDebugDrawCalls=1`，并输出 `pbrShadowAtlasReady=yes`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，IBL debug 验证保持 `rendererPasses=8` 与 `iblDebugDrawCalls=1`，并输出 `pbrShadowAtlasReady=yes`。
   - [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 非黑比例 `100%`，RGB 均值约 `160.99 / 123.69 / 83.49`。
   - [out/pbr_deferred_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_verification.ppm) 非黑比例约 `22.0043%`，RGB 均值约 `26.76 / 21.96 / 17.89`。
   - [out/pbr_gbuffer_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_gbuffer_debug_verification.ppm) 非黑比例约 `22.0009%`，RGB 均值约 `44.22 / 34.54 / 24.64`。
   - [out/pbr_ibl_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_ibl_debug_verification.ppm) 非黑比例 `100%`，RGB 均值约 `145.33 / 154.29 / 165.66`。
266. 完成第一百三十三轮 PBR shadow atlas render pass：
   - 新增 [renderer/PBRShadowAtlasRenderPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderPass.h) 和 [renderer/PBRShadowAtlasRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderPass.cpp)，复用现有 shadow shaders 与 `ShadowMeshDraw`，把当前 shadow casters 额外渲染到 PBR atlas targets。
   - 更新 [renderer/PBRShadowAtlasRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderTargets.h) 和 [renderer/PBRShadowAtlasRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderTargets.cpp)，atlas stats 新增 point faces rendered、directional draw calls 和 point draw calls。
   - 更新 [renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h)、[renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp) 和 [renderer/RendererFrameContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameContext.h)，把 `PBRShadowAtlasRenderPass` 挂入 renderer lifetime 和 frame context。
   - 更新 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，`ShadowMaps` pass 现在会在旧 shadow maps 之后额外写入 PBR shadow atlas。
   - 更新 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，Debug UI 与 `--verify-pbr*` 可观察 atlas draw calls 和 rendered faces。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把 `PBRShadowAtlasRenderPass` 加入 VS 工程和 renderer filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录本轮让 PBR shadow atlas 具备可验证生产链路，但尚未切换 shader sampling。
267. 完成第一百二十四次 PBR shadow atlas render pass 验证：
   - 执行 `git diff --check`；除既有 LF/CRLF 提示外无 whitespace error。
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过，新增 [renderer/PBRShadowAtlasRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderPass.cpp) 已进入 VS 工程。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，输出 `pbrShadowAtlasReady=yes`、`pbrShadowAtlasPointFacesRendered=12`、`pbrShadowAtlasDirectionalDrawCalls=160`、`pbrShadowAtlasPointDrawCalls=384`，同时 deferred path 保持 `pbrDeferredLightingDrawCalls=1`、`pbrDeferredCsmShadowBound=yes`、`pbrDeferredLightBufferBound=yes`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，forward PBR 验证保持 `rendererPasses=7`、`pbrDepthPrepassDrawCalls=25`、`pbrDrawCalls=25`，并输出 atlas draw calls `160 / 384`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，G-buffer debug 验证保持 `rendererPasses=9`、`pbrGBufferDrawCalls=25`、`pbrGBufferDebugDrawCalls=1`，并输出 atlas draw calls `160 / 384`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，IBL debug 验证保持 `rendererPasses=8` 与 `iblDebugDrawCalls=1`，并输出 atlas draw calls `160 / 384`。
   - [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 非黑比例 `100%`，RGB 均值约 `160.99 / 123.69 / 83.49`。
   - [out/pbr_deferred_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_verification.ppm) 非黑比例约 `22.0043%`，RGB 均值约 `26.76 / 21.96 / 17.89`。
   - [out/pbr_gbuffer_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_gbuffer_debug_verification.ppm) 非黑比例约 `22.0009%`，RGB 均值约 `44.22 / 34.54 / 24.64`。
   - [out/pbr_ibl_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_ibl_debug_verification.ppm) 非黑比例 `100%`，RGB 均值约 `145.33 / 154.29 / 165.66`。
268. 完成第一百三十四轮 PBR shadow atlas sampling binding：
   - 更新 [renderer/MaterialBindingContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\MaterialBindingContext.h)，把 `PBRShadowAtlasRenderTargets` 接入 PBR material binding context。
   - 更新 [renderer/PBRShadowResourceBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowResourceBinder.h) 和 [renderer/PBRShadowResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowResourceBinder.cpp)，新增 `bindDetailed(...)` 与 binding source stats；binder 现在优先绑定 PBR atlas directional depth texture array，atlas 不可用时 fallback 到 legacy CSM shadow resources。
   - 更新 [renderer/PBRDeferredLightingPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.h) 和 [renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp)，deferred lighting pass 会记录当前 directional CSM shadow 是否来自 PBR atlas。
   - 更新 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp) 和 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)，把 atlas targets 传入 PBR binding context，并把 atlas shadow binding 状态写入 frame stats。
   - 更新 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，`--verify-pbr-deferred` 与 Debug UI 可观察 `pbrDeferredCsmShadowAtlasBound`。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录这一步完成的是 directional CSM atlas 采样切换，point shadow atlas sampling 仍未接入 PBR shading。
269. 完成第一百二十五次 PBR shadow atlas sampling binding 验证：
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，输出 `pbrDeferredLightingDrawCalls=1`、`pbrDeferredCsmShadowBound=yes`、`pbrDeferredCsmShadowLayers=5`、`pbrDeferredCsmShadowAtlasBound=yes`、`pbrDeferredLightBufferBound=yes` 和 `pbrDeferredLightBufferPointLights=2/16`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，forward PBR 验证保持 `rendererPasses=7`、`pbrDepthPrepassDrawCalls=25`、`pbrDrawCalls=25`，并输出 atlas draw calls `160 / 384`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer`，G-buffer producer 验证保持 `pbrGBufferDrawCalls=25`、`pbrGBufferReady=yes` 和 `pbrGBufferSize=1280x720`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，G-buffer debug 验证保持 `rendererPasses=9`、`pbrGBufferDrawCalls=25`、`pbrGBufferDebugDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，IBL debug 验证保持 `rendererPasses=8` 与 `iblDebugDrawCalls=1`。
   - [out/pbr_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_verification.ppm) 非黑比例 `100%`，RGB 均值约 `160.99 / 123.69 / 83.49`。
   - [out/pbr_gbuffer_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_gbuffer_verification.ppm) 非黑比例 `100%`，RGB 均值约 `160.99 / 123.69 / 83.49`。
   - [out/pbr_gbuffer_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_gbuffer_debug_verification.ppm) 非黑比例约 `22.0009%`，RGB 均值约 `44.22 / 34.54 / 24.64`。
   - [out/pbr_deferred_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_verification.ppm) 非黑比例约 `22.0043%`，RGB 均值约 `26.76 / 21.96 / 17.89`。
   - [out/pbr_ibl_debug_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_ibl_debug_verification.ppm) 非黑比例 `100%`，RGB 均值约 `145.33 / 154.29 / 165.66`。
270. 完成第一百三十五轮 PBR shadow atlas pass key 拆分：
   - 更新 [renderer/RendererFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.h) 和 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，新增 `PBRShadowAtlas` pass key；`ShadowMaps` pass 只负责 legacy shadow maps，PBR atlas 写入改由独立 pass 执行。
   - 更新 [renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp) 和 [config/renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini)，默认 renderer pass order 加入 `PBRShadowAtlas`，Debug UI 的 pass key 列表和示例配置说明可删除该 key 以关闭额外 atlas 渲染成本。
   - 更新 [renderer/PBRShadowAtlasRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderTargets.h)、[renderer/PBRShadowAtlasRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderTargets.cpp) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，每帧开始重置 atlas frame stats，避免省略 `PBRShadowAtlas` pass 后继续采样上一帧 atlas。
   - 更新 [application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，新增 `--verify-pbr-no-atlas` 与 `--verify-pbr-deferred-no-atlas`，用于验证 atlas pass 可关闭和 fallback 行为。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR atlas 写入已从 `ShadowMaps` 内部副作用推进为 profile 可组合 pass。
271. 完成第一百二十六次 PBR shadow atlas pass key 验证：
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，默认 deferred path 输出 `rendererPasses=6`、`pbrShadowAtlasReady=yes`、`pbrDeferredCsmShadowAtlasBound=yes`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-no-atlas`，禁用 atlas pass 后输出 `rendererPasses=5`、`pbrShadowAtlasReady=no`、`pbrShadowAtlasDirectionalDrawCalls=0`、`pbrDeferredCsmShadowBound=yes`、`pbrDeferredCsmShadowAtlasBound=no`，证明 fallback 到 legacy CSM 且未采样过期 atlas。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，forward PBR 默认验证保持 `pbrDepthPrepassDrawCalls=25`、`pbrDrawCalls=25`，默认 pass count 变为 `rendererPasses=8`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer`，G-buffer producer 验证保持 `pbrGBufferDrawCalls=25`、`pbrGBufferReady=yes` 和 `pbrGBufferSize=1280x720`，默认 pass count 变为 `rendererPasses=9`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，G-buffer debug 验证保持 `pbrGBufferDebugDrawCalls=1`，默认 pass count 变为 `rendererPasses=10`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，IBL debug 验证保持 `iblDebugDrawCalls=1`，默认 pass count 变为 `rendererPasses=9`。
   - [out/pbr_deferred_no_atlas_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_no_atlas_verification.ppm) 非黑比例约 `22.0043%`，RGB 均值约 `26.76 / 21.96 / 17.89`。
   - 其余 PPM 输出继续非空：forward / G-buffer 非黑比例 `100%`，G-buffer debug 非黑比例约 `22.0009%`，deferred 非黑比例约 `22.0043%`，IBL debug 非黑比例 `100%`。
272. 完成第一百三十六轮 PBR point shadow atlas sampling：
   - 新增 [shaders/pbr/pbr_point_shadow.glsl](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_point_shadow.glsl)，集中保存 point shadow atlas 的 cube-face selection、2D array layer mapping 和 3x3 PCF sampling。
   - 更新 [renderer/PBRShadowResourceBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowResourceBinder.h) 和 [renderer/PBRShadowResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowResourceBinder.cpp)，PBR shadow binder 现在会在 point atlas ready 时绑定 texture unit 9，并写入每个 point light 的 layer base、near/far、bias、PCF radius 和 enabled flag。
   - 更新 [shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag) 和 [shaders/pbr/pbr_deferred_lighting.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_deferred_lighting.frag)，forward / deferred PBR point light direct lighting 现在都会乘以 `1.0 - calculatePbrPointShadow(...)`。
   - 更新 [renderer/PBRDeferredLightingPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.h)、[renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp)、[renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，deferred PBR stats 现在可观察 point shadow atlas 是否实际绑定以及绑定光源数量。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把 `pbr_point_shadow.glsl` 加入 VS 工程和 pbr shader filter。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 PBR atlas 的 directional 与 point shadow producer-consumer 闭环已接通。
273. 完成第一百二十七次 PBR point shadow atlas sampling 验证：
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，输出 `pbrDeferredCsmShadowAtlasBound=yes`、`pbrDeferredPointShadowAtlasBound=yes`、`pbrDeferredPointShadowAtlasLights=2` 和 `pbrDeferredLightBufferPointLights=2/16`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-no-atlas`，输出 `pbrShadowAtlasReady=no`、`pbrDeferredCsmShadowAtlasBound=no`、`pbrDeferredPointShadowAtlasBound=no` 和 `pbrDeferredPointShadowAtlasLights=0`，证明禁用 atlas pass 后不会采样过期 point atlas。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，forward PBR shader 编译运行通过，保持 `pbrDepthPrepassDrawCalls=25` 与 `pbrDrawCalls=25`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer`，G-buffer producer 验证保持 `pbrGBufferDrawCalls=25`、`pbrGBufferReady=yes` 和 `pbrGBufferSize=1280x720`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer-debug`，G-buffer debug 验证保持 `pbrGBufferDebugDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，IBL debug 验证保持 `iblDebugDrawCalls=1`。
   - [out/pbr_deferred_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_verification.ppm) 非黑比例约 `22.0009%`，RGB 均值约 `26.74 / 21.94 / 17.88`。
   - [out/pbr_deferred_no_atlas_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_no_atlas_verification.ppm) 非黑比例约 `22.0043%`，RGB 均值约 `26.76 / 21.96 / 17.89`。
   - 其余 PPM 输出继续非空：forward / G-buffer 非黑比例 `100%`，G-buffer debug 非黑比例约 `22.0009%`，IBL debug 非黑比例 `100%`。
274. 完成第一百三十七轮 PBR deferred transparent fallback：
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [application/RuntimeApplicationShell.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimeApplicationShell.cpp)，新增 `--verify-pbr-deferred-transparent` 验证入口，会在 scene prepare 后追加一个透明 PBR probe。
   - `--verify-pbr-deferred-transparent` 的 renderer pass order 会在 `PBRDeferredLighting` 后追加 `LegacyTransparentScene,PBRTransparentScene`，用于验证 deferred opaque + forward transparent 组合路径。
   - 更新 [renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 `legacyTransparentDrawCalls` 与 `pbrTransparentDrawCalls` 统计和输出。
   - 透明 probe 使用 PBRMaterial，开启 blend，关闭 depth write，opacity 为 `0.45`，仍通过 `PBRSceneRenderPass` / `PBRMaterialBinder` 绘制，因此复用当前 PBR IBL 与 shadow atlas 绑定。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录这一步先把 deferred transparent fallback 做成可验证路径，后续再考虑 OIT / weighted blended transparency。
275. 完成第一百二十八次 PBR deferred transparent fallback 验证：
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-transparent`，输出 `objects=34`、`meshes=33`、`pbrMeshes=26`、`transparentMeshes=1`、`pbrTransparentMeshes=1`，并在 renderer stats 中输出 `rendererPasses=8`、`pbrDeferredLightingDrawCalls=1`、`pbrDrawCalls=1`、`pbrTransparentDrawCalls=1`。
   - `--verify-pbr-deferred-transparent` 同时保持 `pbrDeferredCsmShadowAtlasBound=yes`、`pbrDeferredPointShadowAtlasBound=yes`、`pbrDeferredPointShadowAtlasLights=2` 和 `pbrDeferredLightBufferPointLights=2/16`，说明透明 fallback 没有绕开当前 PBR shadow / light 资源链路。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，默认 deferred 验证保持 `transparentMeshes=0`、`pbrTransparentDrawCalls=0`，证明透明 probe 只在专用验证入口启用。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-no-atlas`，禁用 atlas fallback 验证保持 `pbrShadowAtlasReady=no`、`pbrDeferredCsmShadowAtlasBound=no`、`pbrDeferredPointShadowAtlasBound=no`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`，forward PBR 验证保持 `pbrDrawCalls=25`、`pbrTransparentDrawCalls=0`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-gbuffer` 与 `--verify-pbr-gbuffer-debug`，G-buffer producer / debug consumer 验证保持 `pbrGBufferDrawCalls=25` 和 `pbrGBufferDebugDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-ibl-debug`，IBL debug 验证保持 `iblDebugDrawCalls=1`。
   - [out/pbr_deferred_transparent_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_transparent_verification.ppm) 非黑比例约 `27.6978%`，RGB 均值约 `32.00 / 29.79 / 26.83`；普通 deferred capture 非黑比例约 `22.0009%`，RGB 均值约 `26.74 / 21.94 / 17.88`。
276. 完成第一百三十八轮 PBR deferred emissive material parity：
   - 更新 [renderer/PBRGBufferRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferRenderTargets.h) 和 [renderer/PBRGBufferRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferRenderTargets.cpp)，G-buffer 新增 emissive color attachment，color attachment 数量从 3 扩展到 4。
   - 更新 [renderer/PBRGBufferPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferPass.cpp) 和 [shaders/pbr/pbr_gbuffer.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_gbuffer.frag)，G-buffer pass 写入 emissive color / intensity 和 emissive map contribution。
   - 更新 [renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp) 和 [shaders/pbr/pbr_deferred_lighting.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_deferred_lighting.frag)，deferred lighting 绑定 emissive attachment 并把 emissive 加到最终 lighting 输出；同时修正空像素判断，允许 emissive-only material 在 albedo 为 0 时仍输出。
   - 更新 [renderer/PBRGBufferDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferDebugPass.cpp)、[shaders/diagnostics/pbr_gbuffer_debug.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\diagnostics\pbr_gbuffer_debug.frag)、[renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp) 和 [config/renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini)，G-buffer debug mode 新增 `7 = Emissive`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 `--verify-pbr-deferred-emissive`，在 verification scene 中追加 opaque emissive PBR probe，并输出 `pbrEmissiveMeshes`。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 emissive 已成为 deferred PBR 的可验证 material parity 项。
277. 完成第一百二十九次 PBR deferred emissive material parity 验证：
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-emissive`，输出 `objects=34`、`meshes=33`、`pbrMeshes=26`、`pbrEmissiveMeshes=1`、`pbrDepthPrepassDrawCalls=26`、`pbrGBufferDrawCalls=26` 和 `pbrDeferredLightingDrawCalls=1`。
   - [out/pbr_deferred_emissive_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_emissive_verification.ppm) 非黑比例约 `26.7658%`，RGB 均值约 `27.79 / 32.37 / 28.41`；普通 deferred capture 非黑比例约 `22.0009%`，RGB 均值约 `26.74 / 21.94 / 17.88`，证明 emissive attachment 已影响 deferred 输出。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，默认 deferred 验证保持 `pbrEmissiveMeshes=0`、`pbrGBufferDrawCalls=25`、`pbrDeferredLightingDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-no-atlas`，禁用 atlas fallback 验证保持 `pbrShadowAtlasReady=no`、`pbrDeferredCsmShadowAtlasBound=no` 和 `pbrDeferredPointShadowAtlasBound=no`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-transparent`，透明 fallback 验证保持 `pbrTransparentMeshes=1`、`pbrTransparentDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`、`--verify-pbr-gbuffer`、`--verify-pbr-gbuffer-debug` 和 `--verify-pbr-ibl-debug`，forward / G-buffer / debug / IBL 验证均通过并导出非空 PPM。
278. 完成第一百三十九轮 PBR deferred material IBL params：
   - 更新 [renderer/PBRGBufferRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferRenderTargets.h) 和 [renderer/PBRGBufferRenderTargets.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferRenderTargets.cpp)，G-buffer 新增 material params color attachment，color attachment 数量从 4 扩展到 5。
   - 更新 [renderer/PBRGBufferPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferPass.cpp) 和 [shaders/pbr/pbr_gbuffer.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_gbuffer.frag)，G-buffer pass 写入 `useIBL`、`iblDiffuseStrength` 和 `iblSpecularStrength`。
   - 更新 [shaders/pbr/pbr_lighting.glsl](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_lighting.glsl)、[renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp) 和 [shaders/pbr/pbr_deferred_lighting.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_deferred_lighting.frag)，deferred lighting 改为按像素读取 material IBL diffuse/specular strength。
   - 更新 [renderer/PBRGBufferDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRGBufferDebugPass.cpp)、[shaders/diagnostics/pbr_gbuffer_debug.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\diagnostics\pbr_gbuffer_debug.frag)、[renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp) 和 [config/renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini)，G-buffer debug mode 新增 `8 = IBL Params`。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 `--verify-pbr-deferred-material-ibl`，在 verification scene 中追加 opaque custom IBL PBR probe，并输出 `pbrCustomIblMeshes`。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 deferred PBR 已补齐 per-material IBL strength parity。
279. 完成第一百三十次 PBR deferred material IBL params 验证：
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-material-ibl`，输出 `objects=34`、`meshes=33`、`pbrMeshes=26`、`pbrCustomIblMeshes=1`、`pbrDepthPrepassDrawCalls=26`、`pbrGBufferDrawCalls=26` 和 `pbrDeferredLightingDrawCalls=1`。
   - [out/pbr_deferred_material_ibl_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_material_ibl_verification.ppm) 非黑比例约 `27.9262%`，RGB 均值约 `38.14 / 33.32 / 27.15`；普通 deferred capture 非黑比例约 `22.0009%`，RGB 均值约 `26.74 / 21.94 / 17.88`，证明 material IBL params 已影响 deferred 输出。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`，默认 deferred 验证保持 `pbrCustomIblMeshes=0`、`pbrGBufferDrawCalls=25` 和 `pbrDeferredLightingDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-no-atlas`，禁用 atlas fallback 验证保持 `pbrShadowAtlasReady=no`、`pbrDeferredCsmShadowAtlasBound=no` 和 `pbrDeferredPointShadowAtlasBound=no`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-transparent`，透明 fallback 验证保持 `pbrTransparentMeshes=1`、`pbrTransparentDrawCalls=1` 且 `pbrCustomIblMeshes=0`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-emissive`，emissive 验证保持 `pbrEmissiveMeshes=1`、`pbrGBufferDrawCalls=26` 和 `pbrDeferredLightingDrawCalls=1`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`、`--verify-pbr-gbuffer`、`--verify-pbr-gbuffer-debug` 和 `--verify-pbr-ibl-debug`，forward / G-buffer / debug / IBL 验证均通过并导出非空 PPM。
280. 完成第一百四十轮 PBR alpha mask first stage：
   - 更新 [materials/pbrMaterial/PBRMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.h) 和 [materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)，`PBRMaterial` / `PBRMaterialProfile` 新增 `useAlphaMask` 与 `alphaCutoff`，并接入 Inspector / profile schema。
   - 更新 [renderer/PBRSurfaceResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSurfaceResourceBinder.cpp)、[shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag) 和 [shaders/pbr/pbr_gbuffer.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_gbuffer.frag)，forward PBR 与 deferred G-buffer 都会按 albedo map alpha 和 cutoff 执行 `discard`。
   - 更新 [renderer/PBRDepthPrepass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDepthPrepass.cpp)，alpha-masked PBR mesh 会跳过 PBR depth prepass，避免 cutout holes 被整片预写深度。
   - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 `--verify-pbr-deferred-alpha-mask`，在 verification scene 中追加使用 [Texture/window.png](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\Texture\window.png) 的 alpha-masked PBR probe，并输出 `pbrAlphaMaskedMeshes`。
   - 更新 [config/pbr_material.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_material.example.ini)、[config/pbr_preview.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_preview.example.ini)、[config/pbr_experiment.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\pbr_experiment.example.ini) 和 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 alpha mask profile 字段和当前 shadow alpha-aware 尚未完成的边界。
281. 完成第一百三十一次 PBR alpha mask first stage 验证：
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-alpha-mask`，输出 `objects=34`、`meshes=33`、`pbrMeshes=26`、`pbrAlphaMaskedMeshes=1`、`pbrDepthPrepassDrawCalls=25`、`pbrGBufferDrawCalls=26` 和 `pbrDeferredLightingDrawCalls=1`，证明 alpha-masked probe 进入 G-buffer 但没有进入 PBR depth prepass。
   - [out/pbr_deferred_alpha_mask_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_alpha_mask_verification.ppm) 非黑比例约 `22.9489%`，RGB 均值约 `26.51 / 21.89 / 18.04`；普通 deferred capture 非黑比例约 `22.0009%`，RGB 均值约 `26.74 / 21.94 / 17.88`，证明 alpha mask probe 已影响 deferred 输出。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`、`--verify-pbr-deferred-no-atlas`、`--verify-pbr-deferred-transparent`、`--verify-pbr-deferred-emissive` 和 `--verify-pbr-deferred-material-ibl`，现有 deferred PBR 验证均通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`、`--verify-pbr-gbuffer`、`--verify-pbr-gbuffer-debug` 和 `--verify-pbr-ibl-debug`，forward / G-buffer / debug / IBL 验证均通过并导出非空 PPM。
282. 完成第一百四十一轮 PBR alpha-aware shadow：
   - 新增 [renderer/PBRAlphaShadowBinder.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRAlphaShadowBinder.h) 和 [renderer/PBRAlphaShadowBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRAlphaShadowBinder.cpp)，集中判断 alpha-masked PBR mesh，并为 shadow shader 绑定 albedo map alpha 与 cutoff。
   - 新增 [shaders/pbr/pbr_alpha_shadow.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_alpha_shadow.vert)、[shaders/pbr/pbr_alpha_shadow.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_alpha_shadow.frag)、[shaders/pbr/pbr_alpha_point_shadow.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_alpha_point_shadow.vert) 和 [shaders/pbr/pbr_alpha_point_shadow.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_alpha_point_shadow.frag)，用于 directional / point shadow alpha discard。
   - 更新 [renderer/DirectionalShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\DirectionalShadowRenderPass.cpp)、[renderer/PointShadowRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PointShadowRenderPass.cpp) 和 [renderer/PBRShadowAtlasRenderPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderPass.cpp)，普通 mesh 继续走 depth-only shader，alpha-masked PBR mesh 走 alpha-aware shadow shader。
   - 更新 [renderer/ShaderLibrary.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.h)、[renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)、[renderer/ShadowRenderStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShadowRenderStats.h)、[renderer/PBRShadowAtlasRenderTargets.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRShadowAtlasRenderTargets.h)、[renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，新增 legacy shadow 与 PBR atlas alpha-masked draw call 统计。
   - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 与 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把新增 C++ 和 shader 文件纳入工程。
   - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，记录 alpha mask 已从主视图扩展到 shadow depth producer。
283. 完成第一百三十二次 PBR alpha-aware shadow 验证：
   - 执行 `Debug|x64` + `LinkIncremental=false` 构建通过。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred-alpha-mask`，输出 `pbrAlphaMaskedMeshes=1`、`directionalAlphaMaskedShadowDrawCalls=5`、`pointAlphaMaskedShadowDrawCalls=12`、`pbrShadowAtlasDirectionalAlphaMaskedDrawCalls=5`、`pbrShadowAtlasPointAlphaMaskedDrawCalls=12`、`pbrDepthPrepassDrawCalls=25` 和 `pbrGBufferDrawCalls=26`。
   - [out/pbr_deferred_alpha_mask_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_deferred_alpha_mask_verification.ppm) 非黑比例约 `22.9489%`，RGB 均值约 `26.97 / 22.22 / 18.26`；普通 deferred capture 非黑比例约 `22.0009%`，RGB 均值约 `26.74 / 21.94 / 17.88`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr-deferred`、`--verify-pbr-deferred-no-atlas`、`--verify-pbr-deferred-transparent`、`--verify-pbr-deferred-emissive` 和 `--verify-pbr-deferred-material-ibl`，现有 deferred PBR 验证均通过；默认非 alpha 场景的 alpha-masked shadow draw calls 保持 `0`。
   - 执行 [x64/Debug/text2.exe](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\x64\Debug\text2.exe) `--verify-pbr`、`--verify-pbr-gbuffer`、`--verify-pbr-gbuffer-debug` 和 `--verify-pbr-ibl-debug`，forward / G-buffer / debug / IBL 验证均通过并导出非空 PPM。

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
- 当前后处理参数已收敛到 runtime-level `PostProcessSettings`，exposure、tone mapping mode、Bloom 开关、threshold、intensity、iterations 都能通过 DebugControllerPanel 修改，且 UI 字段已改为由 `PropertySchema` 自动生成；`ScreenMaterial` 只保留 postprocess 输入贴图。
- 当前 IBL / environment 资源边界已建立，`EnvironmentRenderTargets` 管理 environment / irradiance / prefilter cubemap、BRDF LUT 和 capture FBO。
- 当前 IBL 预计算流程已拆到 `IBLPrecomputePass`，并可通过 `EnvironmentProfile` 在 scene setup 阶段加载 HDR environment、创建 capture cube / BRDF quad 并触发预计算。
- 当前 PBR shader 已支持直接光 + 可选 IBL 组合，`MaterialBinder` 会在 PBR 材质启用 IBL 且 environment ready 时绑定 irradiance / prefilter / BRDF LUT。
- 当前 PBR shader 已输出线性 HDR color，不再在材质 shader 内部执行 tone mapping / gamma，最终显示转换统一交给 screen postprocess。
- 当前 `EnvironmentProfile` 已接入 DebugControllerPanel UI，可在运行时编辑 HDR path / texture unit、切换 prepare 预计算，并手动触发 IBL precompute；字段 UI 已改为由 `PropertySchema` 自动生成。
- 当前 `EnvironmentProfile` 已支持 `config/environment_profile.local.ini` 本地保存 / 加载，UI 可保存和重载 profile；仓库保留 `config/environment_profile.example.ini` 作为字段示例。
- 当前 `EnvironmentProfile` 已支持 procedural HDR equirectangular source，可通过 DebugControllerPanel 或 local profile 在无外部 HDR 文件时触发 IBL precompute。
- 当前默认 `PBR Preview Sphere` 会请求 IBL；environment 未 ready 时 shader 侧自动关闭，environment ready 后可直接验证 PBR IBL 采样链路。
- 当前 `PBR Preview Sphere` 已由 `PBRPreviewProfile` 驱动，可通过 `config/pbr_preview.local.ini` 调整位置、几何细分、PBR surface 参数、IBL 强度和 normal map，而不需要修改 `SceneSetup.cpp`。
- 当前 `PBRPreviewProfile` 支持 material grid，可按 metallic / roughness 范围生成多球阵列，在同一 environment / postprocess 下批量比较 PBR 参数。
- 当前 `PBRMaterial` 已提供 texture slot schema 与 uniform slot schema，PBR Inspector 字段和 `MaterialBinder` shader 写入共享材质侧声明。
- 当前 `PBRPreviewProfile` 已通过 `PBRMaterialProfile` 管理 PBR surface / IBL 材质参数，preview preset 与运行时 `PBRMaterial` 之间有明确转换入口。
- 当前 `PropertySchema` 已从 `PropertyInspector` 拆出，profile 数据层不再依赖 ImGui 绘制层；`EnvironmentProfile`、`PostProcessSettings` 和 `PBRPreviewProfile` 已能通过 property descriptor 自动生成 Debug UI。
- 当前 `PBRExperimentProfile` 支持用 `config/pbr_experiment.local.ini` 统一覆盖 environment、postprocess、PBR preview grid、light rig 和 camera rig，并已复用各 profile 的 property schema 应用 prefixed key。
- 当前 profile 配置解析基础设施已收敛到 `ProfileConfigParser`；`ProfileConfigIO` 已开始让 descriptor 同时驱动 UI 与 ini load/save，当前已迁移 `PostProcessSettings`、`EnvironmentProfile` 和 `PBRPreviewProfile`。
- 当前 `FrameRenderTargets` 已支持窗口 resize 后重建 MSAA scene target、resolved HDR target 和 Bloom targets，并刷新 screen material 的 postprocess 输入贴图。
- 当前 `PostProcessSettings` 已支持 `config/postprocess_settings.local.ini` 本地保存 / 加载，UI 可保存和重载 profile；仓库保留 `config/postprocess_settings.example.ini` 作为字段示例；其读写路径已迁移到 `ProfileConfigIO` schema 驱动。
- 当前 `EnvironmentProfile` 的本地保存 / 加载路径已迁移到 `ProfileConfigIO` schema 驱动，Environment / IBL UI 与 ini 字段共享同一份 descriptor。
- 当前 `PBRPreviewProfile` 的本地保存 / 加载路径已迁移到 `ProfileConfigIO` schema 驱动，Position / Albedo 这类 vec3 UI 字段继续兼容拆分 ini key。
- 当前 `PBRMaterialProfile` 已支持独立 `config/pbr_material.local.ini` 保存 / 加载，PBR preview 与 PBR experiment 都能通过 `materialProfilePath` 引用独立材质 preset。
- 当前 `PBRExperimentProfile` 已支持 `config/pbr_experiment.local.ini` 保存 / 加载 environment、postprocess、PBR preview、light rig 和 camera rig，并已接入 DebugControllerPanel 的组合 preset save/reload 入口。
- 当前 `PBRLightRigProfile` 已接管默认场景灯光初始化，Debug UI 保存 experiment preset 前会从运行时灯光回写 profile，重载后会应用到运行时灯光对象。
- 当前 `PBRCameraRigProfile` 已接入 experiment preset，Debug UI 保存 preset 前会从主相机回写 profile，重载后会应用到主相机；aspect 仍由 `RuntimeViewport` 根据窗口尺寸维护。
- 当前 runtime context 已从 `main.cpp` 的本地 struct 移出到 `AppRuntimeContext`，主入口不再直接定义所有 runtime state 类型。
- 当前 application lifecycle 骨架已从 `main.cpp` 拆出到 `RuntimeBootstrapper`，`main()` 只负责日志等级与生命周期回调绑定。
- 当前 profile loading 阶段已从 `main.cpp` 拆出到 `RuntimeProfileLoader`，environment、postprocess、PBR preview 和 PBR experiment 的分层加载顺序集中在 application 层。
- 当前 runtime resize 边界已从 `main.cpp` 拆出到 `RuntimeViewport`，窗口尺寸变化会统一同步 viewport、PerspectiveCamera aspect、FrameRenderTargets 和 ScreenMaterial postprocess 输入贴图。
- 当前 runtime input 边界已从 `main.cpp` 拆出到 `RuntimeInputController`，CameraControl 输入分发和中键临时 FOV 缩放不再由主入口直接维护。
- 当前 runtime scene preparation 边界已从 `main.cpp` 拆出到 `RuntimeScenePreparer`，scene setup、legacy experiment preparation 和相关 context 构建集中在 application 层。
- 当前 runtime frame orchestration 边界已从 `main.cpp` 拆出到 `RuntimeFrameRunner`，每帧 render / postprocess / UI callback 顺序集中在 application 层。
- 当前 runtime gui host 边界已从 `main.cpp` 拆出到 `RuntimeGuiHost`，ImGui backend 初始化和每帧 host 生命周期集中在 application 层。
- 当前 runtime editor panel coordinator 已从 `main.cpp` 拆出到 `RuntimeEditorPanelCoordinator`，Debug UI / hierarchy / selection inspector 的 context wiring 集中在 application 层。
- 当前 runtime camera lifecycle 已从 `main.cpp` 拆出到 `RuntimeCameraLifecycle`，默认 camera / camera control 创建与清理集中在 application 层。
- 当前 runtime frame pipeline 已从 `RuntimeFrameRunner` 拆出到 `RuntimeFramePipeline`，当前 frame pass 顺序具备独立扩展边界。
- 当前 runtime window lifecycle 已从 `main.cpp` 拆出到 `RuntimeWindowLifecycle`，window setup 和 Application callback glue 集中在 application 层。
- 当前 `main.cpp` 的 runtime field alias 和未使用 legacy 参数已清理，启动参数集中到本地 `MainStartupConfig`。
- 当前 runtime startup sequence 已聚合到 `RuntimeApplicationShell`，`main.cpp` 基本只保留程序入口职责。
- 当前 `RuntimeFramePipeline` 的步骤已拆成显式 pass 类型，PBR pipeline 后续可以按 pass 类型继续扩展。
- 当前 `RuntimeFramePipelineProfile` 已接入 runtime context、profile loader、Debug UI 和本地 ini 读写，scene color / resolve / Bloom / screen composite pass 可运行时切换并保存。
- 当前 pass 的 key、启用条件和执行入口已集中到 `RuntimeFramePassRegistry`，pipeline 主流程不再直接依赖具体 pass toggle 字段。
- 当前已新增 `--verify-pbr` 验证模式，可强制构建 procedural IBL + 5x5 PBR material grid，并导出 default framebuffer PPM；后续 PBR 改动不能再只用旧 Phong 短启动作为验证。
- 当前 `RuntimeFramePipeline` 已从固定 `defaultPasses()` 推进为 profile-driven pass plan，`RuntimeFramePipelineProfile::passOrder` 可以控制当前 pass key 顺序，无效配置会回退默认顺序。
- 当前 renderer 已新增 PBR 专用 scene pass，PBR mesh 从 render queue 分类到 PBR 子队列后由 `PBRSceneRenderPass` 渲染；`--verify-pbr` 已验证 `pbrDrawCalls=25`。
- 当前 renderer 已新增 PBR depth prepass，`--verify-pbr` 已验证 `pbrDepthPrepassDrawCalls=25` 且 `pbrDrawCalls=25`。
- 当前 renderer frame stats 已接入 Debug UI，普通运行时可以直接观察 PBR depth / scene pass 是否实际执行。
- 当前 renderer 内部 pass 顺序已收敛到 `RendererFramePassRegistry`，并已支持通过 `RendererFramePassProfile` 配置 pass order；`PBRShadowAtlas`、`IBLDebug`、`PBRGBuffer`、`PBRGBufferDebug` 与 `PBRDeferredLighting` 已作为可组合 PBR pass 接入，后续 clustered lighting / shadow debug 可以继续按 key 增加、插入和本地 profile 验证，而不必扩写 `Renderer::render()` 主流程。
- 当前 directional shadow 与 point shadow 已拆成独立 render pass，`ShadowRenderer` 只保留调度 facade 职责，后续可逐步替换为 PBR shadow atlas 资源布局。
- 当前 forward lighting uniform 绑定已从 `MaterialBinder` 拆到 `LightResourceBinder`，后续 PBR lighting 可集中演进为 UBO / SSBO / clustered light list。
- 当前 PBR 材质 shader 绑定已从通用 `MaterialBinder` 拆到 `PBRMaterialBinder`，后续 PBR-specific uniform / IBL / shadow binding 可以独立演进。
- 当前 mesh indexed draw 已收敛到 `MeshDraw`，legacy scene、PBR scene、PBR depth、shadow 和 IBL capture 共享同一个普通 / instanced mesh draw 入口。
- 当前材质绑定上下文已收敛到 `MaterialBindingContext`，renderer pass 到 material binder 不再逐项传递 camera / lights / environment 参数。
- 当前 PBR depth prepass 的 depth shader frame/object uniform 写入已收敛到 `DepthPrepassBinder`，后续 G-buffer / depth-only PBR pass 可以复用该绑定边界。
- 当前 PBR IBL 资源绑定已收敛到 `PBRIBLResourceBinder`，PBRMaterialBinder 不再直接维护 irradiance / prefilter / BRDF LUT 贴图绑定细节。
- 当前 PBR surface 参数和贴图绑定已收敛到 `PBRSurfaceResourceBinder`，PBRMaterialBinder 进一步收敛为 PBR 材质绑定编排器。
- 当前 PBR shadow 资源绑定已收敛到 `PBRShadowResourceBinder`，PBRMaterialBinder 不再直接依赖通用 CSM shadow binder 和固定 shadow texture unit。
- 当前 PBR object-level uniforms 已收敛到 `PBRObjectUniformBinder`，PBRMaterialBinder 只保留 PBR forward binding 编排职责。
- 当前 PBR scene pass 已直接调用 `PBRMaterialBinder`，默认 PBR forward path 不再经过通用 `MaterialBinder` switch。
- 当前 forward PBR 与 deferred PBR 已共享 `shaders/pbr/pbr_lighting.glsl` 和 `shaders/pbr/pbr_csm_shadow.glsl`，BRDF / IBL lighting math 与 CSM shadow sampling 不再各自维护重复实现。
- 当前 PBR G-buffer 生产点已接入 renderer pass 系统，`--verify-pbr-gbuffer` 已验证 `pbrGBufferDrawCalls=25`、`pbrGBufferReady=yes` 和 `pbrGBufferSize=1280x720`。
- 当前 PBR G-buffer debug consumer 已接入 renderer pass 系统，`--verify-pbr-gbuffer-debug` 已验证 `pbrGBufferDebugDrawCalls=1`，且导出的 debug capture 非黑比例约 `22.0009%`。
- 当前 PBR deferred lighting consumer 已接入 renderer pass 系统，优先采样 PBR atlas directional CSM texture，fallback 到 legacy CSM shadow resources，并使用 SSBO-backed deferred light buffer；`--verify-pbr-deferred` 已验证 `pbrDrawCalls=0`、`pbrGBufferDrawCalls=25`、`pbrDeferredLightingDrawCalls=1`、`pbrDeferredCsmShadowBound=yes`、`pbrDeferredCsmShadowLayers=5`、`pbrDeferredCsmShadowAtlasBound=yes`、`pbrDeferredLightBufferBound=yes`、`pbrDeferredLightBufferPointLights=2/16`，且导出的 deferred capture 非黑比例约 `22.0043%`。
- 当前 renderer 已持有、通过独立 `PBRShadowAtlas` pass 写入、并优先用于 PBR directional CSM 和 point light shadow sampling 的 PBR shadow atlas，`--verify-pbr*` 已验证当前 scene 下 `pbrShadowAtlasReady=yes`、`pbrShadowAtlasDirectionalLayers=5`、`pbrShadowAtlasPointFacesRendered=12`、`pbrShadowAtlasDirectionalDrawCalls=160`、`pbrShadowAtlasPointDrawCalls=384`；`--verify-pbr-deferred` 已验证 `pbrDeferredPointShadowAtlasBound=yes` 与 `pbrDeferredPointShadowAtlasLights=2`，`--verify-pbr-deferred-no-atlas` 已验证删除 `PBRShadowAtlas` pass 后 directional / point atlas 都不会绑定。
- 当前 PBR deferred path 已有 G-buffer producer / debug consumer / lighting consumer、directional / point atlas shadow sampling、SSBO light buffer、shadow atlas 写入链路、deferred opaque + forward transparent fallback、emissive material parity、per-material IBL strength parity、alpha mask first stage，以及 alpha-aware shadow；`--verify-pbr-deferred-emissive` 已验证 emissive PBR probe 会通过 G-buffer emissive attachment 影响 deferred lighting 输出，`--verify-pbr-deferred-material-ibl` 已验证 custom IBL probe 会通过 G-buffer material params attachment 影响 deferred lighting 输出，`--verify-pbr-deferred-alpha-mask` 已验证 alpha-masked PBR probe 进入 G-buffer、跳过 PBR depth prepass，并通过 legacy shadow map 与 PBR shadow atlas 的 alpha-aware shader 写入 cutout shadow depth。
- 当前剩余明显问题：C 盘空间仍偏低，完整 MSBuild / runtime smoke 需要继续关注输出体积；PBR IBL 已有自动化 scene/capture 验证但还没有人工视觉审阅；工程内仍没有默认真实 HDR environment 资源；PBR deferred path 还缺更多 material feature parity、clustered light culling / tile index list，以及更正式的透明策略例如 OIT / weighted blended transparency。
- 下一步建议目标：开始把 deferred light buffer 演进为 clustered / tiled light index list，或继续补 PBR asset import 的 material mapping。
- 已在桌面生成并更新重构报告 [PBR_refactor_report.md](C:\Users\asus\Desktop\PBR_refactor_report.md)，内容覆盖当前分支最新状态、已完成工作、与重构前工程的区别、runtime / renderer / PBR / UI profile 架构差异、Mermaid 示意图、验证证据和后续建议。
- 完成第一百四十二轮 PBR verification automation：
  - 新增 [tools/verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，默认执行 `Debug|x64` 构建并顺序运行所有 PBR verification mode。
  - 脚本会为每个模式写入 `out/pbr_verify_<mode>.log`，并生成 `out/pbr_verification_summary.txt`，summary 会解析 PPM capture 的尺寸、文件大小、非黑比例和 RGB 均值。
  - 脚本支持 `-SkipBuild` 和 `-Modes`，可用来快速只跑局部回归，例如 `deferred` 或 `deferred-alpha-mask`。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1`，MSBuild `Debug|x64` 通过，`forward`、`forward-no-atlas`、`ibl-debug`、`gbuffer`、`gbuffer-debug`、`deferred`、`deferred-no-atlas`、`deferred-transparent`、`deferred-emissive`、`deferred-material-ibl` 和 `deferred-alpha-mask` 全部通过并生成有效 PPM。
- 完成第一百四十三轮 Assimp PBR material import mapping：
  - 新增 [application/AssimpMaterialImporter.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\AssimpMaterialImporter.h) 和 [application/AssimpMaterialImporter.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\AssimpMaterialImporter.cpp)，把 Assimp material 到项目 material 的转换从 `AssimpLoader::processMesh(...)` 中拆出。
  - 更新 [application/assimpLoader.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\assimpLoader.h) 和 [application/assimpLoader.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\assimpLoader.cpp)，默认 `load(...)` 继续生成 Phong，新增 options overload 和 `loadPBR(...)` 显式生成 `PBRMaterial`。
  - 更新 [materials/pbrMaterial/PBRMaterial.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.h)、[materials/pbrMaterial/PBRMaterial.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\materials\pbrMaterial\PBRMaterial.cpp)、[renderer/PBRSurfaceResourceBinder.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRSurfaceResourceBinder.cpp)、[shaders/pbr/pbr.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr.frag) 和 [shaders/pbr/pbr_gbuffer.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_gbuffer.frag)，PBR metallic / roughness / AO map 现在支持可配置采样通道。
  - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [tools/verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，新增 `--verify-pbr-import`，默认回归脚本纳入 `import` 模式。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1`，MSBuild `Debug|x64` 通过，12 个 PBR verification mode 全部通过；新增 `import` 模式输出 `pbrImportedMeshes=1`、`pbrDrawCalls=26`，并生成有效 [out/pbr_import_verification.ppm](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\out\pbr_import_verification.ppm)。
- 完成第一百四十四轮桌面重构报告更新：
  - 已重写 [PBR_refactor_report.md](C:\Users\asus\Desktop\PBR_refactor_report.md)，报告覆盖当前 `text2-refactor` 分支状态、已完成重构工作、与原始 `text2` 工程的架构差异、runtime / renderer / PBR / UI profile / verification 的分层说明。
  - 报告内嵌 Mermaid 架构示意图，包含旧工程耦合结构、当前 runtime 总结构、renderer pass 架构、PBR deferred 数据流、shadow atlas 数据流、PropertySchema UI/profile 流程，以及当前未提交的 PBR deferred tiled light grid 第一阶段。
  - 报告明确区分了已推送稳定成果 `1cfb907 Add PBR material import mapping` 与当前本地未提交 tiled light grid 中间态，并注明 tiled grid 已通过 focused deferred 验证但尚未执行完整 12 模式回归。
  - 本轮只更新文档，没有执行新的构建或 PBR 回归。
- 完成第一百四十五轮 PBR deferred tiled light grid first stage：
  - 新增 [renderer/PBRDeferredTiledLightGrid.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.h) 和 [renderer/PBRDeferredTiledLightGrid.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.cpp)，CPU 侧按当前 camera / viewport / point light bounds 构建 screen-space tile light list。
  - 更新 [renderer/PBRDeferredLightingPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.h) 和 [renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp)，deferred lighting pass 现在会绑定 tiled grid SSBO，并在 grid 不可用时 fallback 到全局 point light loop。
  - 更新 [shaders/pbr/pbr_deferred_lighting.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\pbr\pbr_deferred_lighting.frag)，新增 tile buffer binding `4`、tile index buffer binding `5`，按 `gl_FragCoord` 选择当前 tile 并遍历 tile 内 point light indices。
  - 更新 [renderer/RendererFramePassProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.h)、[renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp)、[renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp) 和 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 tiled grid 开关、tile size、stats、Debug UI 与 verification 输出。
  - 更新 [text2.vcxproj](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj) 和 [text2.vcxproj.filters](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\text2.vcxproj.filters)，把 tiled grid 新文件纳入 VS 工程。
  - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，补充 `PBR Deferred Tiled Light Grid First Stage` 技术记录。
  - 同步修正 [PBR_refactor_report.md](C:\Users\asus\Desktop\PBR_refactor_report.md)，把 tiled grid 状态从“focused deferred 验证”更新为“完整 12 模式回归已通过，等待提交推送”。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1`，MSBuild `Debug|x64` 通过，12 个 PBR verification mode 全部通过；deferred 输出确认 `pbrDeferredTiledLightGridBound=yes`、`pbrDeferredTiledLightGridSize=80x45`、`pbrDeferredTiledLightGridTileSize=16`、`pbrDeferredTiledLightGridIndices=7200`、`pbrDeferredTiledLightGridMaxTileLights=2`。
- 完成第一百四十六轮 PBR deferred tiled light culling verification：
  - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 和 [application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h)，新增 `--verify-pbr-deferred-tiled-lights` 验证入口和 `enablePbrTiledLightProbe` 配置。
  - 更新 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 sparse / high-attenuation point light rig，让 tiled grid 在专用验证模式下必须产生小于全局遍历上限的 light index count。
  - 更新 [renderer/PBRDeferredLightBuffer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightBuffer.cpp)，修正 deferred point light SSBO 的 intensity 打包，`deferredPointLightColorIntensity.a` 现在使用 `PointLight::getIntensity()`。
  - 更新 [tools/verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，默认 PBR 回归从 12 个模式扩展为 13 个模式，并对 `deferred-tiled-lights` 增加 tiled culling 断言：必须绑定 tiled grid，且 `pbrDeferredTiledLightGridIndices` 必须小于 `tileColumns * tileRows * pointLightCount`。
  - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，补充 `PBR Deferred Tiled Light Culling Verification` 技术记录。
  - 同步更新 [PBR_refactor_report.md](C:\Users\asus\Desktop\PBR_refactor_report.md)，把 PBR verification 状态改为 13 个模式，并补充 `deferred-tiled-lights` 的 `3311 < 7200` 断言结果。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1`，MSBuild `Debug|x64` 通过，13 个 PBR verification mode 全部通过；新增 `deferred-tiled-lights` 模式输出 `pbrDeferredTiledLightGridSize=80x45`、`pbrDeferredLightBufferPointLights=2/16`、`pbrDeferredTiledLightGridIndices=3311`，小于全局遍历上限 `7200`。
- 完成第一百四十七轮 PBR deferred tiled light heatmap debug pass：
  - 新增 [renderer/PBRDeferredTiledLightDebugPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightDebugPass.h) 和 [renderer/PBRDeferredTiledLightDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightDebugPass.cpp)，独立构建 / 绑定 tiled light grid 并绘制 fullscreen heatmap quad。
  - 新增 [shaders/diagnostics/pbr_deferred_tiled_light_debug.vert](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\diagnostics\pbr_deferred_tiled_light_debug.vert) 和 [shaders/diagnostics/pbr_deferred_tiled_light_debug.frag](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\shaders\diagnostics\pbr_deferred_tiled_light_debug.frag)，从 tiled tile buffer SSBO binding `4` 读取 tile light count 并输出 heatmap。
  - 更新 [renderer/ShaderLibrary.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.h)、[renderer/ShaderLibrary.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\ShaderLibrary.cpp)、[renderer/RendererFrameContext.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameContext.h)、[renderer/RendererFramePassRegistry.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.h)、[renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[renderer/renderer.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.h) 和 [renderer/renderer.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\renderer.cpp)，把 `PBRDeferredTiledLightDebug` 接入 renderer pass 系统。
  - 更新 [renderer/RendererFramePassProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.h)、[renderer/RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp)、[config/renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini)、[renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，新增 heatmap max lights / intensity 配置与 debug draw call 统计。
  - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)、[application/RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h)、[application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [tools/verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，新增 `--verify-pbr-deferred-tiled-heatmap`，默认 PBR 回归扩展为 14 个模式，并新增 `-DiscardCaptures` 低磁盘空间验证选项。
  - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，补充 `PBR Deferred Tiled Light Heatmap Debug Pass` 技术记录。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，14 个 PBR verification mode 全部通过；新增 `deferred-tiled-heatmap` 模式输出 `pbrDeferredTiledLightDebugDrawCalls=1`、`pbrDeferredTiledLightGridSize=80x45`、`pbrDeferredTiledLightGridIndices=3311`，capture 解析结果非黑比例 `100%`、RGB 均值约 `185.09 / 173.14 / 87.33`。
- 完成第一百四十八轮收尾同步：
  - 已提交并推送 `c6ed195 Add PBR tiled light heatmap debug pass` 到 `github/codex/text2-refactor`。
  - 已更新桌面报告 [PBR_refactor_report.md](C:\Users\asus\Desktop\PBR_refactor_report.md)，把报告状态同步到 heatmap debug pass、14 模式验证和 `-DiscardCaptures` 低磁盘空间流程。
  - `imgui.ini` 仍保持未提交状态，因为它是运行时 UI 布局状态，不属于本轮重构代码。
- 完成第一百四十九轮 PBR deferred tiled light bounds cleanup：
  - 更新 [renderer/PBRDeferredTiledLightGrid.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.cpp)，screen-space light bounds 从单侧 `right/up` 投影采样改为 `+right/-right/+up/-up` 四点采样；正常投影成功时不再强制使用 `64px` 最小半径，只在采样失败时保留保守 fallback。
  - 新增 [tools/msbuild_no_link_debug.targets](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\msbuild_no_link_debug.targets)，用于验证构建时导入 `/DEBUG:NONE`，避免低磁盘空间下 linker PDB 导致 `LNK1201/LNK1318`。
  - 更新 [tools/verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，新增 `-NoLinkDebugInfo` 构建选项，summary 输出该选项状态，并让 `-Modes` 支持逗号分隔输入。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`，`Debug|x64` 构建通过，`deferred-tiled-lights` 验证通过，输出 `pbrDeferredTiledLightGridIndices=3311`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，14 个 PBR verification mode 全部通过。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures -Modes deferred-tiled-lights,deferred-tiled-heatmap`，确认逗号分隔 `-Modes` 可正确选择两个 tiled verification mode。
- 完成第一百五十轮 PBR deferred tiled light occupancy stats：
  - 更新 [renderer/PBRDeferredTiledLightGrid.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.h) 和 [renderer/PBRDeferredTiledLightGrid.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.cpp)，`PBRDeferredTiledLightGridStats` 新增 occupied / empty tile count，并在构建 tile offset/count buffer 时统计。
  - 更新 [renderer/PBRDeferredLightingPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.h)、[renderer/PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp)、[renderer/RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h) 和 [renderer/RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，把 occupancy stats 汇入 renderer frame stats。
  - 更新 [application/RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [tools/editor/DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，runtime verification / Debug UI 会显示 `pbrDeferredTiledLightGridOccupiedTiles=<occupied>/<total>` 与 empty tile count。
  - 更新 [tools/verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，tiled culling 断言现在会校验 occupied tile count 大于 `0` 且小于 tile 总数，并检查 reported tile count 等于 `columns * rows`。
  - 清理旧 clone [text2](C:\Code\CodeOfC++\OpenGL_test\text2) 的 `.vs` 与旧 `x64` 构建缓存，释放约 `4.9GB` 磁盘空间；未删除源码、配置或 git 历史。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`，`Debug|x64` 构建通过，输出 `pbrDeferredTiledLightGridIndices=3311`、`pbrDeferredTiledLightGridOccupiedTiles=3119/3600`、`pbrDeferredTiledLightGridEmptyTiles=481`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，14 个 PBR verification mode 全部通过。
- 完成第一百五十一轮 PBR deferred tiled light circle tile clip：
  - 更新 [renderer/PBRDeferredTiledLightGrid.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.cpp)，`ScreenBounds` 现在保留 screen center / radius，并在 AABB 候选 tile 内增加 `tileIntersectsCircularBounds(...)` 过滤。
  - 新增的 tile-circle 测试会计算 tile rectangle 到 light center 的最近点，用 `radius + 1px` 判断是否与 light screen circle 相交；center 投影失败的保守全屏 fallback 不启用 circle clip，避免漏光。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`，`Debug|x64` 构建通过，sparse tiled 输出从 `3311 / 3119 occupied / 481 empty` 改进为 `2890 / 2846 occupied / 754 empty`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，14 个 PBR verification mode 全部通过；`deferred-tiled-heatmap` 同步输出 `pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridOccupiedTiles=2846/3600`。
- 完成第一百五十二轮 PBR deferred tiled light flat index builder：
  - 更新 [renderer/PBRDeferredTiledLightGrid.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.cpp)，将 `std::vector<std::vector<int>> tileLightLists` 改为 flat `{ tileIndex, lightIndex }` entries + per-tile count + prefix offset scatter。
  - GPU SSBO layout 保持不变，tile buffer 仍使用 binding `4`，index buffer 仍使用 binding `5`；本轮只优化 CPU builder 的中间数据结构，减少每帧 per-tile small vector 容器开销。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`，`Debug|x64` 构建通过，focused tiled 输出保持 `pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridOccupiedTiles=2846/3600`、`pbrDeferredTiledLightGridEmptyTiles=754`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，14 个 PBR verification mode 全部通过。
- 完成第一百五十三轮 PBR deferred tiled light scratch buffer reuse：
  - 更新 [renderer/PBRDeferredTiledLightGrid.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.h) 和 [renderer/PBRDeferredTiledLightGrid.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.cpp)，把 tile count、flat entries、tile offset/count、light indices 和 scatter write offsets 从 `bind()` 局部临时 vector 提升为 `PBRDeferredTiledLightGrid` 成员 scratch buffers。
  - GPU SSBO layout 和 shader 读取路径保持不变；本轮只减少 CPU builder 每帧局部容器创建，并让 tiled light grid 的数据布局更接近后续 clustered / GPU culling 的 flat buffer 形态。
  - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，补充 `PBR Deferred Tiled Light Scratch Buffer Reuse` 技术记录。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`，`Debug|x64` 构建通过，focused tiled 输出保持 `pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridOccupiedTiles=2846/3600`、`pbrDeferredTiledLightGridEmptyTiles=754`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，14 个 PBR verification mode 全部通过；`deferred-tiled-lights` 与 `deferred-tiled-heatmap` 均保持 `2890` indices、`2846/3600` occupied、`754` empty。
- 完成第一百五十四轮桌面重构报告更新：
  - 已重写 [PBR_refactor_report.md](C:\Users\asus\Desktop\PBR_refactor_report.md)，报告覆盖当前分支最新提交 `c53bbc0 Reuse PBR tiled light grid scratch buffers`、已完成工作、与原始工程的架构差异、runtime / renderer / PBR / UI profile / tiled lighting 的 Mermaid 示意图、验证证据、剩余问题和下一步建议。
  - 报告明确当前工程已经完成 runtime 降耦合、renderer pass 化、PBR forward / deferred 主链路、PBR shadow atlas、Assimp PBR import、tiled light grid first stage、heatmap debug 和 14 模式自动化验证，同时明确生产级完整 PBR、clustered / GPU culling、真实资产视觉基准和完整透明策略尚未完成。
  - 报告生成前已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，14 个 PBR verification mode 全部通过。
- 完成第一百五十五轮 PBR deferred tiled light culling efficiency stats：
  - 更新 [renderer/PBRDeferredTiledLightGrid.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.h)、[renderer/PBRDeferredTiledLightGrid.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.cpp)、[renderer\PBRDeferredLightingPass.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.h)、[renderer\PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp)、[renderer\RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h) 和 [renderer\RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)，新增 tiled point light count、full index count 与 culled index count 统计。
  - 更新 [application\RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [tools\editor\DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，runtime verification / Debug UI 现在会显示 tiled light grid 的 full indices、actual indices、culled indices 和 skipped percentage。
  - 更新 [tools\verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，tiled culling 断言现在会校验 `fullIndices == tileColumns * tileRows * pointLightCount`、`culledIndices == fullIndices - actualIndices`，且专用 tiled culling 模式必须裁掉至少一个 index。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`，`Debug|x64` 构建通过，focused tiled 输出 `pbrDeferredTiledLightGridFullIndices=7200`、`pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridCulledIndices=4310`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，14 个 PBR verification mode 全部通过；`deferred-tiled-lights` 与 `deferred-tiled-heatmap` 均确认 `7200 -> 2890`，裁掉 `4310` 个 tiled point-light index 入口。
- 完成第一百五十六轮 PBR deferred tiled light tile size verification：
  - 更新 [application\RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h) 和 [application\RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 `pbrDeferredTileSizeOverride`，verification 可以显式覆盖 renderer frame pass profile 的 `pbrDeferredTileSize`。
  - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，新增 `--verify-pbr-deferred-tiled-lights-32` 命令行入口，复用 sparse tiled light probe 并将 tile size override 设置为 `32`。
  - 更新 [tools\verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，默认 PBR 回归从 14 个模式扩展为 15 个模式，新增 `deferred-tiled-lights-32`，并对该模式断言 `pbrDeferredTiledLightGridTileSize=32`。
  - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，补充 `PBR Deferred Tiled Light Tile Size Verification` 技术记录。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights-32`，`Debug|x64` 构建通过，输出 `pbrDeferredTiledLightGridSize=40x23`、`pbrDeferredTiledLightGridTileSize=32`、`pbrDeferredTiledLightGridFullIndices=1840`、`pbrDeferredTiledLightGridIndices=762`、`pbrDeferredTiledLightGridCulledIndices=1078`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，15 个 PBR verification mode 全部通过。
- 完成第一百五十七轮 PBR deferred tiled light cutoff profile：
  - 更新 [renderer\RendererFramePassProfile.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.h)、[renderer\RendererFramePassProfile.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassProfile.cpp) 和 [config\renderer_frame_pass.example.ini](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\config\renderer_frame_pass.example.ini)，新增 `pbrDeferredTiledLightCutoff` profile 字段，默认保持 `0.01`。
  - 更新 [renderer\PBRDeferredTiledLightGrid.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.h)、[renderer\PBRDeferredTiledLightGrid.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.cpp)、[renderer\PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp) 和 [renderer\PBRDeferredTiledLightDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightDebugPass.cpp)，deferred lighting 与 heatmap debug 现在都使用 profile cutoff 估算 light bounds。
  - 更新 [renderer\RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[renderer\RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[application\RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [tools\editor\DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，runtime stats / Debug UI 会输出当前 tiled light cutoff。
  - 更新 [application\RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h)、[main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp) 和 [tools\verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，新增 `--verify-pbr-deferred-tiled-lights-cutoff-005`，默认 PBR 回归从 15 个模式扩展为 16 个模式，并对该模式断言 `pbrDeferredTiledLightGridCutoff=0.05`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights-cutoff-005`，`Debug|x64` 构建通过，输出 `pbrDeferredTiledLightGridCutoff=0.050000`、`pbrDeferredTiledLightGridFullIndices=7200`、`pbrDeferredTiledLightGridIndices=714`、`pbrDeferredTiledLightGridCulledIndices=6486`、`pbrDeferredTiledLightGridOccupiedTiles=714/3600`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，16 个 PBR verification mode 全部通过。
- 完成第一百五十八轮 PBR deferred untiled fallback verification：
  - 更新 [application\RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h)、[application\RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，新增 `--verify-pbr-deferred-untiled-lights`，复用 sparse tiled light probe 但强制关闭 `pbrDeferredTiledLightsEnabled`。
  - 更新 [renderer\PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp)、[renderer\RendererFrameStats.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFrameStats.h)、[renderer\RendererFramePassRegistry.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\RendererFramePassRegistry.cpp)、[application\RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp) 和 [tools\editor\DebugControllerPanel.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\editor\DebugControllerPanel.cpp)，runtime stats / Debug UI 现在会输出 `pbrDeferredTiledLightsEnabled`。
  - 更新 [tools\verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，默认 PBR 回归从 16 个模式扩展为 17 个模式，并新增 untiled fallback 断言：deferred lighting 必须绘制、light buffer 必须绑定并有 point lights、tiled path 必须 disabled、tiled grid 不应绑定。
  - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，补充 `PBR Deferred Untiled Fallback Verification` 技术记录。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-untiled-lights`，`Debug|x64` 构建通过，输出 `pbrDeferredLightingDrawCalls=1`、`pbrDeferredLightBufferBound=yes`、`pbrDeferredLightBufferPointLights=2/16`、`pbrDeferredTiledLightsEnabled=no`、`pbrDeferredTiledLightGridBound=no`。
  - 执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，17 个 PBR verification mode 全部通过。
- 完成第一百五十九轮桌面重构详细报告更新：
  - 已重写 [PBR_refactor_report.md](C:\Users\asus\Desktop\PBR_refactor_report.md)，报告覆盖当前已完成工作、与原始 `text2` 的架构差异、runtime / renderer / UI profile / PBR deferred / tiled light grid 的 Mermaid 示意图、17 模式验证状态、当前未提交的 `PBRDeferredTiledLightGridConfig` 改动，以及后续 PBR / clustered lighting 缺口。
  - 报告中记录最新已推送提交为 `11af4a2 Verify PBR deferred untiled fallback`，并明确当前工作区除 `imgui.ini` 外仍有 tiled grid config API 收敛改动尚未提交。
  - 报告引用最近一次完整验证结果：`powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，17 个 PBR verification mode 全部通过。
- 完成第一百六十轮 PBR deferred tiled light grid config object：
  - 更新 [renderer\PBRDeferredTiledLightGrid.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.h) 和 [renderer\PBRDeferredTiledLightGrid.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightGrid.cpp)，新增 `PBRDeferredTiledLightGridConfig`，并将 `PBRDeferredTiledLightGrid::bind(...)` 从松散 `tileSize/lightCutoff` 参数改为接收 config object。
  - 更新 [renderer\PBRDeferredLightingPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredLightingPass.cpp) 和 [renderer\PBRDeferredTiledLightDebugPass.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\renderer\PBRDeferredTiledLightDebugPass.cpp)，deferred lighting pass 与 tiled heatmap debug pass 现在都从 `RendererFramePassProfile` 构造同一类 tiled grid config 后传入 grid builder。
  - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，补充 `PBR Deferred Tiled Light Grid Config Object` 技术记录，说明该接口形态为后续 clustered / GPU culling 参数扩展预留空间。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`，`Debug|x64` 构建通过，focused tiled 输出保持 `pbrDeferredTiledLightGridFullIndices=7200`、`pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridCulledIndices=4310`。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures -Modes deferred-tiled-heatmap`，heatmap consumer 验证通过，输出 `pbrDeferredTiledLightDebugDrawCalls=1`、`pbrDeferredTiledLightGridBound=yes`、`pbrDeferredTiledLightGridIndices=2890`。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，17 个 PBR verification mode 全部通过。
- 完成第一百六十一轮 PBR texture set verification probe：
  - 更新 [application\RuntimePBRVerification.h](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.h) 和 [application\RuntimePBRVerification.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\application\RuntimePBRVerification.cpp)，新增 `enablePbrTextureSetProbe`，并在 verification scene 中用 `fbx/bag` 贴图集创建 `PBR Texture Set Probe`。
  - 新 probe 绑定 `fbx/bag/diffuse.jpg`、`specular.jpg`、`roughness.jpg`、`ao.jpg` 和 `normal.png`，覆盖 PBR albedo / metallic / roughness / AO / normal map 采样链路。
  - 更新 [main.cpp](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\main.cpp)，新增 `--verify-pbr-texture-set` 命令行入口和 `out/pbr_texture_set_verification.ppm` capture path。
  - 更新 [tools\verify_pbr.ps1](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\tools\verify_pbr.ps1)，默认 PBR 回归新增 `texture-set` 模式，并断言 `pbrTexturedMeshes > 0` 与 `pbrDrawCalls >= 26`。
  - 更新 [work.md](C:\Code\CodeOfC++\OpenGL_test\text2-refactor\work.md)，补充 `PBR Texture Set Verification Probe` 技术记录。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes texture-set`，`Debug|x64` 构建通过，输出 `pbrTexturedMeshes=1`、`pbrMeshes=26`、`pbrDrawCalls=26`、capture 非黑比例 `99.9951%`。
  - 已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`，18 个 PBR verification mode 全部通过。
- 完成第一百六十二轮桌面重构报告同步：
  - 已更新 [PBR_refactor_report.md](C:\Users\asus\Desktop\PBR_refactor_report.md)，把报告同步到最新提交 `513f663 Verify PBR texture set probe`。
  - 报告中将默认 PBR verification 状态更新为 18 个模式，并补充 `texture-set` probe、`pbrTexturedMeshes=1`、`pbrDrawCalls=26` 和 capture 非黑比例 `99.9951%`。
  - 报告中已删除 `PBRDeferredTiledLightGridConfig` 仍未提交的过期说明，当前工作区只保留未提交的 `imgui.ini` 运行时布局状态。

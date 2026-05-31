# 重构工作文档

## 目的

这份文档用于记录 `text2-refactor` 的重构目标、当前判断、技术选型、阶段计划和待确认问题。

我们的目标不是立刻大改全部代码，而是先把“为什么要重构、准备怎么重构、哪些部分先动、哪些部分暂时不动”说清楚，避免一边改一边失控。

## 当前观察

基于仓库根目录的文件结构和现有 `README.md`，目前可以先做出这些初步判断：

- 项目是一个基于 Visual Studio 2022 的 C++/OpenGL 学习型工程。
- 根目录下存在大量 `.cpp/.h` 文件，模块边界还比较扁平，职责分散在根目录。
- 项目中同时存在 `renderer/`、`shaders/`、`materials/`、`mesh/`、`light/`、`tools/` 等目录，但仍有很多核心源文件直接堆在根目录。
- 现有 README 也明确提到项目“有点乱（a little bit mess）”，说明这次重构重点更偏向结构整理、职责拆分和可维护性提升，而不是只做局部代码美化。
- 当前这份 clone 是一个干净工作区，更适合做结构性调整和渐进式重构。

## 重构总目标

希望通过这次重构，让项目逐步具备下面这些特征：

1. 目录结构清晰，代码放置位置有明确规则。
2. 核心模块职责明确，避免“一个文件做太多事”。
3. 渲染流程、资源管理、场景对象、材质系统之间的关系更容易理解。
4. 新增一个效果、材质、后处理或对象时，改动范围可预测。
5. 编译依然稳定，重构过程中尽量保持可运行。
6. 后续继续扩展时，不需要再把逻辑继续堆回 `main.cpp` 或根目录大文件。

## 非目标

为了控制范围，第一阶段建议不把下面这些事情混在一起做：

- 不先追求“全部代码风格统一”。
- 不先引入重量级外部框架替换现有架构。
- 不先把渲染逻辑完全重写成 ECS、Render Graph 或大型引擎式架构。
- 不先改动所有 shader 内容，除非它们阻碍结构调整。
- 不在第一步就追求完美抽象，优先做职责切分和可维护性提升。

## 建议的重构方向

### 方向 A：先做项目结构重组

核心思路：

- 先整理目录和模块边界。
- 让文件“归位”，减少根目录中的杂糅代码。
- 尽量不改变外部行为，先降低理解成本。

适合先处理的内容：

- `Application`
- `camera` 及各种 `*camera*control`
- `light`
- `material`
- `object / scene / geometry / mesh`
- `renderer`
- `shader / texture / framebuffer`

预期收益：

- 风险较低。
- 能迅速改善可读性。
- 为后续更深层的逻辑重构打基础。

### 方向 B：再做职责拆分

核心思路：

- 从“大文件”和“混合职责类”开始拆。
- 把“资源创建”“渲染命令”“场景遍历”“后处理流程”“调试/测试代码”分开。

优先怀疑对象：

- `main.cpp`
- `renderer.cpp/.h`
- `Application.cpp/.h`
- `geometry.cpp`
- `shader.cpp`
- `texture.cpp`

预期收益：

- 代码逻辑更容易定位。
- 功能扩展时不容易互相污染。

### 方向 C：最后做 API 整理

核心思路：

- 在结构清晰后，再重新设计调用接口。
- 尽量减少模块之间的隐式依赖。
- 明确谁负责初始化、谁负责拥有资源、谁负责释放资源。

可能涉及：

- Renderer 对外接口
- Material/Shader 绑定关系
- Object 与 Renderable 的抽象边界
- Scene 更新与渲染入口
- 资源生命周期管理

## 建议的重构顺序

### 第一阶段：建立边界，不急着深改

- 盘点当前主要模块及其依赖关系。
- 找出最大、最乱、最常改动的文件。
- 整理新的目录结构方案。
- 把明显属于同一模块的文件迁移到对应目录。
- 保持项目可编译。

### 第二阶段：处理主流程

- 收敛 `main.cpp` 的职责。
- 明确 `Application` 是否作为程序主入口协调者。
- 梳理初始化流程：窗口、OpenGL、资源、场景、渲染器、UI。
- 梳理每帧流程：输入、更新、提交渲染、后处理、显示。

### 第三阶段：处理渲染系统

- 明确 `renderer` 模块的职责边界。
- 区分基础渲染、后处理、特殊效果。
- 把 Bloom、shadow、framebuffer 等功能组织到统一结构下。
- 尽量减少 renderer 对具体对象实现细节的耦合。

### 第四阶段：处理资源和场景

- 整理 shader、texture、material、mesh、geometry 的关系。
- 明确资源对象是否拥有 GPU 资源生命周期。
- 明确 object/scene 只负责组织数据，还是也承担渲染行为。

## 可以重点讨论的技术问题

### 1. 目录结构应该怎么定

可以考虑把源码按职责集中，比如：

- `src/app`
- `src/render`
- `src/render/postprocess`
- `src/render/materials`
- `src/scene`
- `src/camera`
- `src/resource`
- `src/core`
- `assets/shaders`

也可以保留现有工程习惯，不强行改成现代 `src/include` 样式，而是做“温和整理版”。

这里要讨论的是：我们更想要“最小迁移成本”，还是“未来扩展最舒服”的结构。

### 2. `main.cpp` 应该保留多少逻辑

理想情况下，`main.cpp` 应该只保留：

- 程序启动
- 创建 `Application`
- 执行主循环
- 程序退出

如果它现在承载了场景创建、资源加载、调试开关、渲染流程、输入处理等大量逻辑，那么它应该是第一批拆分对象之一。

### 3. Renderer 的职责边界

需要明确：

- Renderer 是“发起渲染”的系统，还是“同时管理所有渲染资源”的上帝类。
- Bloom、shadow、screen pass、framebuffer 是否应该作为子模块存在。
- Renderer 是否应该直接知道具体对象类型，还是只消费更抽象的提交数据。

这是整个项目最值得先谈清楚的一块。

### 4. Material / Shader / Texture 的关系

需要明确：

- Material 是否只是 Shader 参数容器。
- Shader 是否负责 uniform 设置的全部细节。
- Texture 资源绑定是否应该由 Material 统一协调。
- 是否存在“一个材质类对应一个 shader 文件组”的强耦合问题。

如果这一层关系不清晰，后面加任何渲染效果都会越来越重。

### 5. Scene / Object / Geometry 的职责

需要明确：

- Object 是否同时承担变换、资源引用和 draw 行为。
- Scene 是否只是容器，还是也负责更新和渲染排序。
- Geometry 和 Mesh 是否存在职责重叠。

如果边界不清晰，就容易出现“逻辑该放哪都能放”的问题。

### 6. 资源生命周期管理

需要明确：

- OpenGL 资源何时创建、何时销毁。
- 是否存在裸指针、多处共享、释放责任不明确的问题。
- 是否要逐步引入 RAII 风格的包装。

这一点会直接影响重构稳定性。

## 第一版建议策略

我建议我们优先采用“渐进式重构”，而不是一次性推翻：

1. 先梳理架构和依赖。
2. 再移动文件和拆分职责。
3. 最后收口接口和命名。

理由：

- 这个项目目前更像持续积累的学习工程，不适合直接做大爆炸式改造。
- 如果先强行重写，很容易丢失已有功能或把工程搞到不可编译。
- 先做结构整理，能够最快建立全局理解。

## 下一步建议

我建议下一步先做以下事情：

1. 统计项目中体量最大、职责最混杂的源文件。
2. 画出一版简化模块图。
3. 给出第一版新的目录结构草案。
4. 选一个切入点开始第一轮重构，优先考虑 `main.cpp` 和 `renderer` 相关部分。

## 待确认问题

下面这些问题会直接影响重构路线，后续我们需要逐步确认：

- 这次重构最优先解决的是“目录混乱”，还是“渲染流程难维护”？
- 我们是否允许在第一阶段移动大量文件路径？
- 我们是否接受顺手修复明显命名问题和拼写问题？
- 我们是否要兼顾以后继续加新效果，比如后处理、PBR、实例化渲染等？
- 我们是否希望最后形成更偏“教学/学习友好”的结构，而不是更偏“引擎化”的结构？

## 当前结论

现阶段最稳妥的路线是：

- 先把重构当成“架构整理 + 职责拆分”；
- 先不要把目标设成“全面重写渲染引擎”；
- 先建立稳定的模块边界；
- 从 `main.cpp`、`renderer`、资源系统、场景对象关系这四块入手。

---

## 2026-05-20 全项目重构分析（refactor 分支）

本节只分析 `text2-refactor` 当前状态，也就是 Git 分支 `codex/text2-refactor` 上的重构版本；不把 `master` 的旧根目录布局作为当前事实。

### 当前证据

- 当前分支：`codex/text2-refactor`。
- 当前 GitHub 远程：`https://github.com/XXTongC/LearnOpenGL.git`，远程分支为 `codex/text2-refactor`。
- 当前未提交变更：仅有本地 UI 状态文件 `imgui.ini`。
- 排除 `third_party` 与构建输出后，项目自身约有 `111` 个 `.cpp/.h` 文件。
- 当前最大的项目源文件：
  - `renderer/renderer.cpp`：约 `1429` 行。
  - `framework/geometry.cpp`：约 `720` 行。
  - `main.cpp`：约 `469` 行。
  - `tools/editor/EditorPanels.cpp`：约 `396` 行。
  - `framework/texture.cpp`：约 `391` 行。

### 已经完成的结构改善

- 根目录源码已经按物理目录归类到 `application`、`camera`、`framework`、`framebuffer`、`light`、`materials`、`renderer`、`tools`、`legacy`、`wrapper` 等模块。
- `main.cpp` 的默认场景装配已经迁入 `tools/sceneSetup`。
- 编辑器面板已经迁入 `tools/editor`，形成 `hierarchy -> selection -> inspector` 的基础编辑器流。
- 材质参数 UI 已经开始转为声明式 `MaterialInspector`，新材质可以通过声明属性生成 UI。
- 历史实验代码已经集中到 `tools/legacyExperiments`，不再继续以内联大段注释的方式堆在主流程里。
- `glad.c` 已从用户下载目录依赖收回到仓库内 `third_party/src/glad.c`。

### 主要重构风险与后续热点

1. `main.cpp` 仍然承担过多运行期状态。

   当前 `main.cpp` 已经比早期清晰，但仍然保存 `AppRuntimeContext`、全局引用别名、ImGui 控制面板、Camera 创建、回调转发和每帧调度。它现在更像一个临时 composition root，下一步应该把这些职责继续拆成 `Runtime`、`CameraSystem`、`DebugControllerPanel` 或类似模块。

2. `renderer/renderer.cpp` 是当前最大且耦合最高的文件。

   `Renderer` 同时负责 shader 创建、渲染状态切换、材质分发、shadow map 渲染、实例化绘制、MSAA resolve、uniform 设置和具体材质类型转换。这个文件是后续最重要的降耦合对象，但不能一次性重写，应该先拆出低风险子模块：
   - `RenderState`：depth / stencil / blend / culling / polygon offset。
   - `ShaderLibrary`：`MaterialType -> Shader` 映射和 shader 初始化。
   - `ShadowRenderer`：directional / CSM / point shadow pass。
   - `MaterialBinder`：按材质类型上传 uniform 和贴图。

3. UI 系统已经有声明式起点，但还没有统一到全部可编辑对象。

   `MaterialInspector` 已经是正确方向；但 `Light`、`Camera`、`Shadow` 仍由 `EditorPanels.cpp` 直接手写 ImGui 控件。后续应把它们也改成类似 `visitEditableProperties(...)` 的可检查对象接口，这样 `EditorPanels` 只负责选择对象和调度 inspector。

4. include 路径仍有历史痕迹。

   当前项目能构建，但 `main.cpp`、`renderer.cpp`、`tools/*`、`light/shadow/*` 中同时存在根 include、模块 include 和 `../` 相对 include。目录重组后，建议制定一条规则：项目内部统一从工程 include root 写模块路径，例如 `mesh/instancedMesh.h`、`camera/perspectivecamera.h`，逐步移除跨目录 `../`。

5. 资源生命周期还不是 RAII 风格。

   当前 Camera 和 CameraControl 仍由 `main.cpp` 中裸指针创建和释放；`Application` 仍是 singleton；OpenGL 对象的释放责任分散在各类析构和显式 destroy 中。后续应优先把运行时拥有关系改成 `std::unique_ptr` 或明确 owner 对象，再处理 GPU resource wrapper 的析构一致性。

6. `framework/geometry.cpp` 体量偏大，但风险低于 Renderer。

   `Geometry` 主要是几何生成逻辑，问题更多是文件体量和职责聚合，而不是跨模块耦合。后续可以把 plane / box / sphere / screen plane 的生成迁到 `geometry/GeometryFactory` 或分文件实现，但优先级低于 `Renderer` 和 `main.cpp`。

7. `legacy` 与实验入口需要保持隔离。

   `tools/legacyExperiments/LegacyExperimentRunner.h` 已经把实验入口集中起来，但它是 header-only，包含较多具体资源和材质依赖。后续建议拆出 `.cpp`，让头文件只暴露实验开关 API，减少任何包含该头文件的编译单元被迫接触大量实现依赖。

### 建议的下一批重构顺序

1. 先从 `main.cpp` 拆出 Debug Controller UI。

   这一步风险较低，因为 `hierarchy / inspector` 已经证明 UI 可以从主文件迁出。建议新增 `tools/editor/DebugControllerPanel.h/.cpp`，把当前 `renderIMGUI()` 中的 `"controller"` 窗口迁出去，只保留 ImGui frame 生命周期和 editor panel 调用。

2. 再统一项目内部 include 风格。

   当前构建虽然通过，但 include 写法还不一致。这个清理应该在继续移动更多文件前完成，避免后续每次拆模块都遇到路径噪音。

3. 然后拆 `Renderer` 的状态控制和 shader 管理。

   第一刀不碰渲染行为，只把 `setDepthState`、`setStencilState`、`setColorBlendState`、`setFaceCullingState`、`setPolygonOffsetState` 迁入 `RenderState` 类或命名空间。第二刀再处理 shader 初始化和选择。

4. 最后推进声明式 inspector 扩展。

   把 Light / Camera / Shadow 的可编辑属性也描述成数据，减少 `EditorPanels.cpp` 中的类型判断和手写控件。这个方向会让后续实验环境切换时 UI 维护成本继续下降。

### 当前建议结论

下一步最符合“降耦合”的动作不是继续做可视化效果，而是拆 `renderIMGUI()` 中剩余的 debug controller 面板。它的收益明确、风险可控，也能延续上一轮 `tools/editor` 模块化成果。

### 当前模块依赖地图

下面是当前 refactor 分支的实际依赖形态，用于指导后续拆分顺序：

- `main.cpp` 仍是 composition root，直接连接 `Application`、Camera、Renderer、Scene、Framebuffer、LegacyExperiment、SceneSetup、EditorPanels 和 ImGui frame 生命周期。
- `tools/sceneSetup` 负责默认场景装配，依赖 Renderer、Scene、Light、Material、Mesh、Framebuffer，是当前“场景内容定义”的集中位置。
- `tools/editor` 负责 hierarchy / selection / inspector，但仍直接认识 Light、Camera、Shadow、Mesh 和 MaterialInspector。
- `tools/inspector` 当前只抽象了 Material 的可编辑属性，尚未覆盖 Light / Camera / Shadow。
- `renderer` 是渲染核心，但目前向上认识 Scene / Mesh / Light / Camera，向下认识 Framebuffer / Shader / Texture，同时横向认识几乎所有具体 Material 类型。
- `materials` 保存材质状态和部分 UI 属性描述；但具体材质上传逻辑仍主要集中在 Renderer。
- `framework` 保存 Object / Scene / Geometry / Shader / Texture；其中 Geometry 与 Texture 仍有较大实现体量。
- `application` 主要承接窗口 Application 和 Assimp loader；loader 当前会接触 Renderer、Mesh、Texture、Shader、Material，说明资源加载和渲染资源创建尚未完全分层。
- `legacy` 与 `tools/legacyExperiments` 已经隔离历史实验入口，但 `LegacyExperimentRunner` 仍是 header-only，实现依赖暴露偏多。

### 依赖问题分级

高优先级：

- Renderer 对具体 Material 类型的依赖过重。现在新增一个材质通常需要修改 `MaterialType`、材质类、shader 初始化、shader 选择和 `Renderer::renderObject()` 分支，扩展成本集中在 Renderer。
- `main.cpp` 仍保存运行时所有权和 UI frame 编排。虽然已经收敛过，但它仍同时承担初始化、每帧调度、回调转发、Camera 生命周期和 debug controller UI。
- Camera / CameraControl 仍使用裸指针和手动 `delete`。这会阻碍后续把运行时状态移动到独立 owner 对象中。

中优先级：

- include 风格不统一。当前仍混用根 include、模块 include 和跨目录 `../`，这会让物理目录重组后的边界不够清晰。
- EditorPanels 对 Light / Camera / Shadow 的字段认识过多。它已经承担面板渲染，后续不应该继续累积具体对象编辑细节。
- `LegacyExperimentRunner` 作为 header-only 模块会把大量实验依赖传播给包含方。

低优先级：

- `framework/geometry.cpp` 体量偏大，但它的职责相对集中，可以等 Renderer 和 main 继续瘦身后再拆。
- `framework/texture.cpp` 体量中等，当前更像资源封装问题，不是最先影响模块边界的点。
- `tools/ObjectPool` 当前不像主流程关键依赖，可暂缓。

### 执行 Backlog

第一组：低风险降耦合，建议连续完成。

1. 新增 `tools/editor/DebugControllerPanel.h/.cpp`，迁出 `main.cpp` 中 `"controller"` 面板。
2. 将 `main.cpp` 的 ImGui 逻辑收敛成 frame 生命周期：`NewFrame -> draw panels -> Render`。
3. 统一项目内部 include 风格，优先把明显的 `../` 改为从工程 include root 开始的模块路径。
4. 把 `LegacyExperimentRunner` 从 header-only 拆成 `.h/.cpp`，降低包含方依赖。

第二组：中风险结构拆分，需要每步构建验证。

1. 从 `Renderer` 中拆出 `RenderState`，只迁移 depth / stencil / blend / culling / polygon offset 状态设置。
2. 从 `Renderer` 中拆出 `ShaderLibrary`，让 shader 初始化和 `MaterialType -> Shader` 映射离开主渲染文件。
3. 把 Camera / CameraControl 所有权从裸指针改成 `std::unique_ptr`，再把创建逻辑迁出 `main.cpp`。
4. 为 Light / Camera / Shadow 增加声明式可编辑属性接口，让 `EditorPanels` 只负责调度 inspector。

第三组：较高风险架构演进，等前两组稳定后再做。

1. 从 `Renderer::renderObject()` 中拆出 `MaterialBinder`，逐步减少 Renderer 对具体材质类的直接 `static_pointer_cast`。
2. 拆出 `ShadowRenderer`，独立 directional / CSM / point shadow pass。
3. 重新定义 Loader 与 Renderer 的边界，避免 Assimp loader 直接承担过多渲染资源绑定职责。
4. 再评估 GeometryFactory / ResourceManager 是否值得引入，避免过早抽象。

### 下一步验收标准

下一轮如果执行 Debug Controller Panel 拆分，验收标准应该是：

- `main.cpp` 中不再直接包含 debug controller 的 ImGui 控件细节。
- `renderIMGUI()` 只保留 ImGui frame 生命周期和模块化 panel 调用。
- `tools/editor` 下新增的 debug controller 模块进入 `text2.vcxproj` 和 `text2.vcxproj.filters`。
- `Debug|x64` 构建通过，且不提交本地 `imgui.ini`。

### 2026-05-20 PBR 前置重构进展

已完成两项直接服务于 PBR 路径的前置拆分：

- Debug Controller Panel 已从 `main.cpp` 移入 `tools/editor/DebugControllerPanel.*`，入口文件不再直接承载调试控件细节。
- Renderer 的通用 OpenGL 状态应用已移入 `renderer/RenderState.*`，材质状态和具体材质 uniform 绑定开始分离。
- Shader 初始化与 `MaterialType -> Shader` 映射已移入 `renderer/ShaderLibrary.*`，后续新增 PBR shader 不需要继续把 shader 成员堆进 `Renderer`。

下一步建议改为清理 PBR 接入的核心扩展点：

1. 新增 `PBRMaterial` 的空壳和 `MaterialType::PBRMaterial`，先定义数据模型：albedo、metallic、roughness、ao、normal、emissive 等。
2. 先不实现完整 PBR 光照公式，只把 PBR material 注册到 `ShaderLibrary`，并准备 `shaders/pbr/pbr.vert` / `pbr.frag` 的最小可编译版本。
3. 再从 `Renderer::renderObject()` 中拆出 `MaterialBinder`，让 Phong 和 PBR 的 uniform 上传逐步分开。
4. 之后再处理 IBL 资源：irradiance map、prefilter map、BRDF LUT，这一部分应该独立成 environment lighting 模块。

后续我们可以持续直接修改这份 `work.md`，把抽象讨论逐渐收敛成具体执行计划。

### 2026-05-20 PBR 最小路径接入

本轮开始把 PBR 从“后续计划”推进成真实工程扩展点：

- 已新增 `PBRMaterial`，当前先作为材质数据模型存在，覆盖 albedo、metallic、roughness、ao、normal、emissive 和对应贴图槽。
- 已新增 `shaders/pbr/pbr.vert` 与 `shaders/pbr/pbr.frag`，当前实现的是直接光照版 GGX PBR，不包含 IBL。
- 已通过 `ShaderLibrary` 注册 `MaterialType::PBRMaterial`，避免重新把 shader 成员堆回 `Renderer`。
- 已在 `Renderer::renderObject()` 中增加临时 PBR uniform 上传分支，用于验证材质路径完整；这部分下一步应迁入 `MaterialBinder`，否则 Renderer 仍会继续膨胀。

当前刻意没有实现或仍需补强的内容：

- normal map 已在后续接入 PBR shader，但 tangent / bitangent / TBN 的生成质量仍需要统一验证，尤其是 loader 导入模型和程序生成几何的 tangent 数据一致性。
- IBL 还没有接入，irradiance map、prefilter map、BRDF LUT 应该在后续单独引入 environment lighting 模块。
- PBR 贴图的 texture unit 分配仍沿用 `Texture` 自身状态，后续资源绑定系统需要统一管理槽位，避免复杂材质间冲突。

下一步建议继续拆 `MaterialBinder`：

1. 先把 Phong 和 PBR 的 uniform 上传从 `Renderer::renderObject()` 中迁出。
2. 保持 `Renderer` 只负责取 shader、应用状态和 draw call。
3. 再逐步把 Shadow / Env / Instance 材质绑定迁移出去，最终让新增材质不再直接修改 Renderer 主流程。

### 2026-05-20 MaterialBinder 初步拆分

`MaterialBinder` 已开始承接材质 uniform 上传职责：

- 当前已迁移 `PhongMaterial` 与 `PBRMaterial`，这两类材质不再直接在 `Renderer::renderObject()` 中展开参数绑定细节。
- `Renderer` 目前仍保留旧材质 switch，但会先尝试 `MaterialBinder::bind(...)`；这允许我们后续逐类迁移，不需要一次性重写所有历史材质。
- 这一步对 PBR 路径的意义是：后续扩展 PBR 参数、贴图、IBL 采样或 BRDF LUT 时，主要修改 `PBRMaterial` 与 `MaterialBinder`，而不是继续加重 Renderer 主流程。

后续迁移顺序建议：

1. 把 `PhongNormalMaterial` 与 `PhongParallaxMaterial` 迁入 `MaterialBinder`，同时为 tangent/TBN 管线做准备。
2. 再迁移 `PhongShadowMaterial` 与 point/csm shadow 材质，拆分出 shadow-specific binder 或 `ShadowRenderer`。
3. 最后迁移 Env / Instance / Grass 等特殊材质，避免一开始被历史特殊分支拖慢主路径重构。

### 2026-05-20 TBN 材质绑定迁移

`PhongNormalMaterial` 与 `PhongParallaxMaterial` 已迁入 `MaterialBinder`：

- normal map 和 parallax map 的 sampler 绑定现在集中在 `MaterialBinder`，Renderer 不再直接认识这两类材质。
- 这一步把 TBN 相关材质的绑定模式和 PBR 的绑定模式放到同一个模块里，后续可以直接复用 normal/parallax 的经验给 `PBRMaterial` 接入 normal map。
- 当前仍未改变 shader 的 TBN 计算方式；现有 normal/parallax shader 仍使用 `aTangent`、`aNormal` 和 `TBN` 输出。下一步如果要让 PBR normal map 正式生效，应先统一 PBR vertex shader 的 tangent 输入和 TBN 输出。

下一步建议：

1. 继续验证 PBR normal map 所需的 tangent 数据生成质量，尤其是程序生成几何和 Assimp 导入模型之间的一致性。
2. 再将 `PhongShadowMaterial` / `PhongPointShadowMaterial` / `PhongCSMShadowMaterial` 迁出 Renderer，或者直接规划 `ShadowRenderer`，避免 shadow 特例继续挤在普通材质分支里。

### 2026-05-20 PBR normal map 接入

`PBRMaterial::mNormalMap` 已从数据槽推进到 shader 路径：

- `pbr.vert` 现在接收 `aTangent`，并输出 `TBN`。
- `pbr.frag` 新增 `normalMap` / `useNormalMap`，启用 normal map 时会将 tangent-space normal 转换到世界空间后参与 GGX 光照。
- `MaterialBinder` 已绑定 `PBRMaterial::mNormalMap`，与 albedo、metallic、roughness、ao、emissive 使用同一套可选贴图开关模式。

后续需要继续验证的点：

1. 程序生成几何与 Assimp 导入几何的 tangent 数据是否完整、方向是否一致。
2. 通用 `Geometry` 构造函数已补充 tangent fallback，但 Assimp 导入路径是否总能提供正确 UV / normal 仍需要实际模型验证。
3. PBR 仍缺 IBL，因此材质观感还只是 direct lighting PBR，不是完整生产级 PBR。

### 2026-05-20 Geometry tangent fallback

为降低 PBR normal map 对几何输入的脆弱性，`Geometry` 层已补充 tangent fallback：

- 对没有显式 tangent 输入的通用构造函数，根据 position / uv / index 自动计算 tangent。
- 当 UV 退化导致 tangent 无法计算时，根据 normal 生成稳定正交 tangent。
- 对显式 tangent 构造函数增加 `aTangent` attribute guard，避免 shader 不需要 tangent 时触发无效 attribute 绑定。

这一步让 `pbr.vert` 可以安全依赖 `aTangent`，但仍不能替代真实资产验证。后续应选择一个带 normal map 的模型进行运行时检查，确认导入路径、程序生成几何和 shader 的 TBN 方向一致。

### 2026-05-20 Geometry attribute guard

Geometry 的 attribute 绑定已进一步收敛：

- 新增统一的 `bindFloatAttribute(...)`，当 shader 未使用某个 attribute 时跳过 VBO 绑定，避免无效 location 触发 OpenGL 错误。
- PBR shader 不再依赖 `aColor`，因此没有 vertex color 的导入模型不会被 vertex color 乘成黑色。
- position / uv / normal / color / tangent 的绑定路径开始统一，后续可以继续把 Geometry 构造逻辑拆成 `VertexArrayBuilder` 或 `GeometryBufferBuilder`。

下一步建议：

1. 做一次运行时 PBR sample：创建一个 `PBRMaterial` mesh，确认 shader 编译、attribute 绑定、direct lighting 和 normal map 路径能正常跑通。
2. 如果运行时验证通过，再开始拆 `ShadowRenderer`，把 shadow pass 从 Renderer 主流程中迁出。

### 2026-05-20 默认场景 PBR preview

默认场景已接入一个低成本 PBR preview：

- `SceneSetup` 新增 `preparePBRPreview(...)`，在 world scene 中创建 `PBR Preview Sphere`。
- preview sphere 使用 `PBRMaterial`、PBR shader、程序生成 sphere geometry，以及 `Texture/normal/normal_map.png` 作为 normal map。
- 这让默认运行路径覆盖 PBR shader、MaterialBinder、TBN attribute 和 normal map 绑定，不再只是存在未使用的 PBR 代码。

后续仍需要真实运行程序确认：

1. PBR shader 在运行时是否编译 / link 成功。
2. `PBR Preview Sphere` 是否在默认灯光下可见。
3. normal map 方向是否符合当前 TBN 约定。

### 2026-05-20 Shader 运行时诊断补强

在继续拆 `ShadowRenderer` 前，先补强 shader 运行时诊断：

- `Shader::loadShader(...)` 现在会在 shader 文件无法打开时输出具体路径，避免缺失文件静默变成空 shader source。
- `#include` 解析会检查引号格式，格式错误时输出当前 shader 文件路径。
- vertex compile、fragment compile 与 program link 的错误输出现在带具体阶段和 shader 路径组合，方便后续验证 PBR shader、IBL shader、shadow shader 时快速定位问题。

这一步不改变渲染行为，但能降低后续 PBR / shadow / IBL 接入时的调试成本。下一步建议继续拆 `ShadowRenderer`，把 shadow pass 从 `Renderer` 主流程迁出去。

### 2026-05-20 ShadowRenderer 初步拆分

Shadow map pass 已从 `Renderer` 主实现中拆出：

- 新增 `ShadowRenderer`，集中负责 CSM 方向光 shadow map 和 point light shadow map 的绘制。
- `Renderer::render(...)` 现在只把相机、不透明队列、方向光、点光源和 `ShaderLibrary` 委派给 `ShadowRenderer`，不再直接实现 shadow pass 细节。
- `ShadowRenderer` 内部保留原有状态保存 / viewport 恢复 / instanced mesh 绘制逻辑，并对空 point light 列表增加早退，避免不必要访问 shared depth texture。

这一步的目标不是重写阴影算法，而是建立模块边界。后续可以继续把 shadow 材质 uniform 绑定迁到独立 binder，让 PBR 材质只消费统一的 shadow resources，而不是直接依赖 Renderer 旧分支。

### 2026-05-20 Shadow 材质绑定迁移

Shadow 相关材质的 uniform 上传已从 `Renderer::renderObject()` 迁入 `MaterialBinder`：

- `PhongShadowMaterial` 现在走 `MaterialBinder`，复用通用矩阵、法线矩阵、灯光和 Phong 贴图绑定逻辑。
- `PhongCSMShadowMaterial` 的 CSM layer、shadow map array、light matrices、PCSS 参数绑定集中到 `MaterialBinder`。
- `PhongPointShadowMaterial` 的 point shadow texture array、point light far/near、directional fallback matrix 和 debug uniform 也集中到 `MaterialBinder`。
- `Renderer` 删除了对应旧 case、旧 helper 和具体 shadow material include，职责进一步收敛为选择 shader、应用 render state、调用 binder 和 draw mesh。

这一步让 shadow resource 的消费路径开始从 Renderer 主流程里剥离。后续 PBR 接 shadow 时，应优先复用这一层 shadow 资源绑定思路，而不是在 PBR 分支中重新硬编码 shadow map / light matrix / bias 参数。

### 2026-05-20 ShadowResourceBinder 抽象

Shadow 资源绑定已从 `MaterialBinder` 中继续抽出为可复用模块：

- 新增 `ShadowResourceBinder`，集中提供 CSM shadow resource、point shadow resource、directional fallback shadow 参数绑定。
- `MaterialBinder` 的 shadow 材质分支现在只负责材质自身参数、矩阵、通用光照和调用 shadow resource binder。
- CSM 的 cascade layers、shadow map array、light matrices、PCSS 参数不再直接散落在具体材质绑定函数中。
- point shadow 的 texture array、point light far/near、debug uniform 也通过同一个模块绑定。

这一步的意义是为 PBR shader 接入 shadow 做准备：PBR 后续不需要复制 Phong shadow 材质分支，只需要在自己的 binding 阶段调用统一的 shadow resource 入口，并在 shader 侧消费对应 uniform。

### 2026-05-20 PBR CSM shadow 接入

PBR 材质已开始消费统一的 shadow resource：

- `PBRMaterial` 绑定阶段调用 `ShadowResourceBinder::bindCSMShadowResources(...)`，当前使用 texture unit `8`，避免和常见 PBR 贴图槽冲突。
- `pbr.frag` 新增 CSM 相关 uniform：`shadowMapSampler`、`csmLayerCount`、`csmLayers`、`lightMatrices`、`bias`、`pcfRadius`。
- PBR fragment shader 根据 `viewMatrix` 和 `csmLayers` 选择 cascade layer，并用 `lightMatrices[layer]` 采样 shadow map array。
- 当前先采用 3x3 PCF 的最小 shadow visibility，将方向光 PBR 结果乘以 `(1.0 - directionalShadow)`；point light shadow 和 IBL 仍未接入。

验证边界：

1. C++ / VS 工程构建已通过。
2. 本机未找到 `glslangValidator`，无法做离线 GLSL validator 检查。
3. 短启动程序未输出 `Shader Compile Error` / `Shader Link Error`，说明当前默认运行路径没有触发 PBR shader 编译或链接错误；视觉结果仍需要人工观察窗口确认。

### 2026-05-20 简单材质绑定迁移

`Renderer::renderObject()` 中的简单材质分支继续向 `MaterialBinder` 收敛：

- `WhiteMaterial`、`DepthMaterial`、`ScreenMaterial`、`CubeMaterial`、`CubeSphereMaterial` 已迁入 `MaterialBinder`。
- `CubeMaterial` / `CubeSphereMaterial` 原本会把 mesh position 同步到 camera position，这个 skybox 行为已在迁移后保留。
- `Renderer` 删除了对应旧 case 和不再需要的材质 include，当前剩余旧分支主要是 Opacity、Env、Instance、GrassInstance。
- 顺手移除了未使用的 `ScreenShot.h` include，并把旧分支中的 `glfwGetTime()` 显式转成 `float`，使本轮构建回到 `0` warning。

这一步继续压缩 Renderer 主流程。后续如果继续迁移 Opacity / Env / Instance，`Renderer::renderObject()` 将更接近只负责材质分发失败时的 legacy fallback。

### 2026-05-20 Opacity / Env 材质绑定迁移

`Renderer::renderObject()` 中的 Opacity 与环境反射类材质分支已迁入 `MaterialBinder`：

- `OpacityMaskMaterial` 的 diffuse、opacity mask、MVP、normal matrix、通用光照和 shininess 上传集中到 `MaterialBinder`。
- `PhongEnvMaterial` 与 `PhongEnvSphereMaterial` 的 diffuse、specular mask、environment sampler、矩阵和光照上传集中到 `MaterialBinder`。
- `Renderer` 删除了这三个旧 case 和对应具体材质 include，当前 fallback 只剩 instanced mesh 相关材质。

这一步继续减少 `Renderer` 对具体材质类型的直接依赖。下一步建议迁移 `PhongInstanceMaterial` 与 `GrassInstanceMaterial`，但这两个分支还包含 instance matrix 更新策略和草地材质运行时参数，应该单独处理，避免把行为差异在迁移时混掉。

### 2026-05-20 Instanced 材质绑定迁移

`Renderer::renderObject()` 中最后两个具体材质分支已迁入 `MaterialBinder`：

- `PhongInstanceMaterial` 的 diffuse、specular mask、MVP、通用光照、shininess 和 `matricesUpdateState` 上传集中到 `MaterialBinder`。
- `GrassInstanceMaterial` 的草地、风、云、透明 mask 参数，以及 instance matrix attribute 更新逻辑集中到 `MaterialBinder`。
- 新增 `setInstanceMatrixUniforms(...)` helper，统一维护 uniform matrix path 与 attribute matrix path 的 shader 开关。
- `Renderer` 删除了具体材质 include、旧 switch 和失效的历史绘制注释块，现在主路径只负责应用 render state、选择 shader、调用 binder、draw mesh。

这一步完成了材质绑定从 `Renderer` 主流程中的整体剥离。后续重点应该转向更高层的 pass 边界：把当前 frame 流程拆成更明确的 shadow / scene / postprocess 阶段，为 PBR + IBL + 后处理组合提供稳定结构。

### 2026-05-20 SceneRenderPass 初步拆分

主 scene 绘制阶段已从 `Renderer` 中拆出：

- 新增 `SceneRenderPass`，负责按 opaque / transparent 队列绘制 mesh。
- `SceneRenderPass` 内部集中处理 `RenderState::applyMaterialState(...)`、shader begin/end、`MaterialBinder::bind(...)` 和普通 / instanced draw call。
- `Renderer` 删除 `drawMesh(...)` 与 `renderObject(...)`，现在只负责 frame buffer 绑定、基础 GL frame state、队列构建和排序、shadow pass 调度、scene pass 调度。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已收录新的 pass 文件，保持 VS 工程结构一致。

这一步建立了 shadow pass 与 scene pass 的明确边界。后续还应继续拆 `RenderQueue` / frame state / postprocess 边界，让 PBR + IBL 可以作为 scene pass 的材质路径扩展，而不是继续压进 `Renderer` 主流程。

### 2026-05-20 Assimp loader warning 清理

`SceneRenderPass` 拆分后的构建暴露出既有 loader warning：

- `assimpLoader.cpp` 与 `assimpInstanceLoader.cpp` 中多个循环使用 `int` 与 Assimp 的 unsigned 计数字段比较。
- 已将相关循环索引改为 `unsigned int`，与 `mNumMeshes`、`mNumChildren`、`mNumVertices`、`mNumFaces`、`mNumIndices` 的类型对齐。
- 顺手移除了 `assimpInstanceLoader.cpp` 顶部重复 include。

这一步不改变模型加载语义，目标是恢复 `0 warning` 构建，降低后续 PBR / pass 拆分验证时的噪音。

### 2026-05-20 RenderQueue 拆分

opaque / transparent 队列构建已从 `Renderer` 中拆出：

- 新增 `RenderQueue`，负责清空队列、递归收集 scene 中的 mesh / instanced mesh、按材质透明状态分组。
- 透明队列的相机深度排序迁入 `RenderQueue::sortTransparentObjects(...)`，保留原有排序规则。
- `Renderer::render(...)` 现在只调用 `mRenderQueue.build(scene, camera)`，随后把 opaque 队列交给 `ShadowRenderer`，把 opaque / transparent 队列交给 `SceneRenderPass`。
- `Renderer` 删除队列成员和 `projectObject(...)`，职责进一步收敛为 frame state 与 pass 编排。

这一步让渲染队列成为独立概念。后续 PBR / IBL 如果需要区分 forward opaque、transparent、skybox、probe capture 或特定材质队列，可以在 `RenderQueue` 扩展，而不是继续修改 `Renderer` 主流程。

### 2026-05-20 FrameRenderState 拆分

每帧开始时的固定 GL 状态准备已从 `Renderer` 中拆出：

- 新增 `FrameRenderState`，负责绑定目标 framebuffer、设置 depth / stencil / blend / polygon offset 初始状态，并清理 color / depth / stencil buffer。
- `Renderer::render(...)` 现在通过 `mFrameRenderState.begin(fbo)` 进入一帧，不再直接维护这组 GL 状态细节。
- `setClearColor(...)` 暂时保留在 `Renderer`，`msaaResolve(...)` 已在后续 `PostProcessPass` 拆分中移出。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已收录新的 frame state 文件。

这一步让 frame state 成为独立边界。后续可以继续把 postprocess pass 或 render target 管理拆出来，使 PBR / IBL 的 render target、prepass、environment capture 不再和主 `Renderer` 入口耦合。

### 2026-05-20 PostProcessPass 初步拆分

MSAA resolve 已从 `Renderer` 中拆出，作为后处理边界的第一步：

- 新增 `PostProcessPass`，当前负责 `resolveMultisample(...)`，把 multisample framebuffer blit 到 resolve framebuffer。
- `Renderer` 删除 `msaaResolve(...)`，避免继续把后处理工具函数堆在主渲染编排器里。
- `runFrame()` 现在通过 `frameRenderTargets` 取出 multisample / resolved target，再交给 `postProcessPass.resolveMultisample(...)` 执行 resolve。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已收录新的 postprocess pass 文件，保持 VS 工程结构一致。

这一步只是建立边界，不试图一次性完成完整后处理系统。后续可以继续把 screen composite、tone mapping、Bloom 和 HDR render target 管理迁入 `PostProcessPass` 或独立 render target orchestration 层，为 PBR / IBL 输出路径提供稳定承接点。

### 2026-05-20 FrameRenderTargets 初步拆分

主颜色渲染目标已从 `main.cpp` / `SceneSetup` 的散落字段中收拢到 `FrameRenderTargets`：

- 新增 `FrameRenderTargets`，当前统一创建和持有 multisample scene framebuffer 与 resolved HDR framebuffer。
- `SceneSetup` 不再直接暴露 `framebufferMultisample` / `framebufferResolve` 两个引用，而是通过 `frameRenderTargets.initialize(...)` 准备主帧目标。
- screen pass 的 `ScreenMaterial::mScreenTexture` 现在来自 `frameRenderTargets.getResolvedColorAttachment()`。
- `runFrame()` 通过 `frameRenderTargets.getSceneFbo()` 渲染 world scene，并通过 `PostProcessPass` resolve 到 resolved HDR target。

这一步把 PBR 后续必需的 render target orchestration 起点独立出来。当前类还只管理主 scene target，后续可以继续扩展 HDR scene color、Bloom ping-pong target、depth target、IBL capture target 与 resize/recreate 策略。

### 2026-05-20 PostProcessPass screen composite 拆分

屏幕合成已从普通 scene render 路径中移出：

- `PostProcessPass` 新增 `renderScreenComposite(...)`，直接把 resolved HDR color attachment 绘制到目标 framebuffer。
- `runFrame()` 不再调用 `renderer->render(sceneInScreen, ...)` 执行屏幕输出，避免 screen quad 继续经过 render queue / shadow pass / scene pass。
- `SceneSetup` 现在显式保留 `screenQuad` 引用，并继续把它挂到 `sceneInScreen`，用于 hierarchy / inspector 可视化，不作为实际绘制入口。
- screen composite 当前负责默认 framebuffer 绑定、viewport、基础后处理 GL state、exposure/tone mapping shader uniform 和 fullscreen quad draw。

这一步让后处理输出成为独立 pass。后续 Bloom、tone mapping 选项、gamma 策略和 HDR/LDR 输出切换可以继续放进 `PostProcessPass`，而不是继续伪装成普通 scene 中的一个 mesh。

### 2026-05-20 ScreenMaterial 绑定清理

`ScreenMaterial` 已从普通 scene 材质绑定路径中移除：

- `MaterialBinder` 删除 `ScreenMaterial` include、`bindScreenMaterial(...)` helper 和 `MaterialType::ScreenMaterial` 分支。
- `ScreenMaterial` 的 texture / exposure / fullscreen draw 绑定现在只由 `PostProcessPass::renderScreenComposite(...)` 负责。
- 普通 scene pass 不再需要知道 screen composite 的 shader uniform 细节。

这一步完成 screen composite 从 scene material binding 中的脱钩。后续如果要支持多种 postprocess 材质或 tone mapping 配置，应扩展 `PostProcessPass` / postprocess settings，而不是重新把它放回 `SceneRenderPass` 或 `MaterialBinder`。

### 2026-05-20 Bloom 与 Framebuffer 解耦

Bloom 的亮度提取实现已从 framebuffer 层移回 renderer/Bloom：

- `framebuffer.cpp` 删除对 `renderer/Bloom/Bloom.h` 的 include，不再实现 `Bloom::extractBright(...)`。
- `Bloom::extractBright(...)` 迁入 `renderer/Bloom/Bloom.cpp`，并改为 `const std::shared_ptr<Framebuffer>&` 参数。
- `Bloom::extractBright(...)` 增加空指针保护、VAO 解绑，并保持当前 extract bright shader 行为不变。
- `Bloom` 的 mip level 计算增加显式 `static_cast<int>`，避免后续重新编译时出现浮点到整数转换噪音。

这一步清理了 framebuffer 到 renderer/Bloom 的反向依赖。后续接 Bloom 时，可以由 `PostProcessPass` 或独立 Bloom pass 调用 Bloom，而不是让底层 framebuffer 文件承担渲染 pass 行为。

### 2026-05-20 Bloom bright extraction 接入

Bloom 的第一段后处理链已接入运行时，但暂时不改变最终屏幕合成结果：

- `FrameRenderTargets` 新增 `bloomBright` HDR framebuffer，用于保存 bright extraction 输出。
- `SceneSetup` 初始化 `Bloom` 实例，和主 frame render targets 一起作为运行时渲染资源准备。
- `PostProcessPass` 新增 `extractBloomBright(...)`，统一调度 `Bloom::extractBright(...)`。
- `runFrame()` 在 MSAA resolve 之后、screen composite 之前执行 bright extraction：resolved HDR color -> bloom bright target。

这一步只生成 bloom bright 中间结果，不做 blur/composite。这样可以先验证 Bloom shader、目标 framebuffer 和 pass 调度链路，再在后续步骤中接 blur ping-pong 与最终合成，避免一次性改变最终画面输出。

### 2026-05-20 Bloom blur ping-pong 接入

Bloom 的 blur 中间链路已接入运行时，但仍暂不合成回最终屏幕输出：

- 新增 `shaders/bloom/blur.vert` 与 `shaders/bloom/blur.frag`，实现 separable Gaussian blur。
- `FrameRenderTargets` 新增 `bloomPing` / `bloomPong` 两个 HDR framebuffer，用于 blur ping-pong。
- `Bloom` 新增 `blurPingPong(...)` 与内部 `drawTextureToTarget(...)`，负责在 bright target、ping、pong 之间迭代模糊。
- `PostProcessPass` 新增 `blurBloom(...)`，统一调度 Bloom blur。
- `runFrame()` 在 bright extraction 后执行 blur ping-pong，但 screen composite 仍只使用 resolved HDR color。

这一步验证了 Bloom blur shader、ping-pong framebuffer 与 pass 调度链路。后续只需要增加 composite shader / screen shader 输入，将 blurred bloom texture 与 HDR color 合成，再进入 tone mapping。

### 2026-05-20 Bloom screen composite 接入

Blurred Bloom texture 已合成进最终 screen composite：

- `ScreenMaterial` 新增 `mBloomTexture`、`mBloomIntensity`、`mBloomEnabled`，并暴露到 inspector。
- `SceneSetup` 将 `FrameRenderTargets::getBloomPong()` 的 color attachment 接入 `ScreenMaterial::mBloomTexture`。
- `PostProcessPass::renderScreenComposite(...)` 绑定 bloom texture 到 texture unit 2，并上传 `enableBloom` 与 `bloomIntensity`。
- `shaders/screen/screen.frag` 在 tone mapping 前执行 HDR color + blurred bloom color 合成。
- `Bloom` bright threshold 默认值从 `0.0` 调整为 `1.0`，避免把全部 HDR 颜色都当成 bloom 源。

这一步完成了当前 Bloom 链路的闭环：resolved HDR color -> bright extraction -> ping-pong blur -> screen composite -> tone mapping / gamma。后续还应继续把 tone mapping 模式、Bloom threshold / iterations / intensity 等参数系统化，并处理 resize/recreate。

### 2026-05-20 PostProcessSettings 参数收敛

后处理参数已从多个零散字段收敛到 `PostProcessSettings`：

- 新增 `PostProcessSettings`，统一维护 exposure、tone mapping mode、Bloom 开关、Bloom threshold、Bloom intensity 和 Bloom iterations。
- `ScreenMaterial` 不再直接持有 `mExposure` / `mBloomIntensity` / `mBloomEnabled`，而是持有 `mSettings`。
- `MaterialInspector` 增加 `Int` 属性类型，使 tone mapping mode 与 Bloom iterations 能从 inspector 修改。
- `Bloom::extractBright(...)` 不再使用内部固定 threshold，改为由 `PostProcessSettings::bloomThreshold` 每帧传入。
- `runFrame()` 根据 `ScreenMaterial::mSettings` 决定是否执行 Bloom extract / blur，并使用配置的 iterations。
- `PostProcessPass::renderScreenComposite(...)` 从同一份 settings 上传 exposure、tone mapping mode、Bloom enabled 和 Bloom intensity。
- `shaders/screen/screen.frag` 支持 exposure / Reinhard 两种 tone mapping mode。

这一步让后处理参数变成一个明确配置对象。后续可以把它从 `ScreenMaterial` 进一步提升到 runtime / renderer 级别，或者直接扩展为 UI 可保存的 profile；PBR 输出进入 HDR + tone mapping + Bloom 链路时会更稳定。

### 2026-05-20 EnvironmentRenderTargets 初步接入

IBL / environment 资源边界已建立，但暂不改变 PBR shader 的最终着色结果：

- 新增 `EnvironmentRenderTargets`，集中持有 environment cubemap、irradiance cubemap、prefilter cubemap、BRDF LUT，以及用于生成这些资源的 capture FBO / RBO。
- 新增 `EnvironmentRenderTargetSettings`，统一记录 cubemap / LUT 尺寸、texture unit 和 prefilter mip 数量。
- `Renderer` 现在持有 `EnvironmentRenderTargets` 并在构造时初始化，后续 PBR / IBL pass 可以通过明确接口消费这些资源。
- `EnvironmentRenderTargets` 提供 cubemap face 与 BRDF LUT capture attachment 接口，后续接 equirectangular to cubemap、irradiance convolution、prefilter 和 BRDF integration shader 时不需要再临时散落 FBO 绑定代码。
- 本轮没有把 IBL sampler 接入 `PBRMaterial` / `MaterialBinder`，避免在资源边界未稳定前直接改变 PBR 视觉输出。

这一步把 PBR 的下一阶段拆成更清晰的顺序：先实现 environment capture / IBL precompute pass，再把 irradiance、prefilter 和 BRDF LUT 作为统一资源传给 PBR 材质绑定。后续不应再把这些 render target 直接塞进 `FrameRenderTargets` 或普通 framebuffer 工具函数里。

### 2026-05-20 IBLPrecomputePass 边界建立

IBL 预计算流程已从资源所有权中拆出：

- 新增 `IBLPrecomputePass`，负责调度 equirectangular HDR texture -> environment cubemap、environment cubemap -> irradiance cubemap、environment cubemap -> prefilter cubemap、BRDF LUT 生成。
- `EnvironmentRenderTargets` 继续只负责资源持有和 capture attachment，不承担具体预计算算法。
- `ShaderLibrary` 新增 IBL utility shader 注册与 getter，包括 equirectangular to cubemap、irradiance convolution、prefilter 和 BRDF LUT。
- 新增 `shaders/ibl/*`，为后续实际烘焙 IBL 资源准备 shader 路径。
- `Renderer` 持有 `IBLPrecomputePass` 并暴露 getter，后续 scene setup 或 runtime profile 可以显式触发 IBL 预计算。

这一步仍不改变 PBR shader 输出，只把 IBL 资源生成流程变成可调用模块。下一步应接入 HDR environment texture 加载和 capture cube / BRDF screen quad 的创建策略，再决定是在启动阶段预计算还是在 environment profile 变化时重新生成。

### 2026-05-20 EnvironmentProfile 可选预计算入口

IBL 预计算已有 runtime 入口，但默认仍关闭，避免在没有 HDR 资源时改变当前画面：

- 新增 `EnvironmentProfile`，记录 HDR equirectangular path、HDR texture unit 和是否在 scene prepare 阶段执行预计算。
- 新增 `EnvironmentTextureLoader`，使用 `stbi_loadf` 加载 HDR equirectangular texture，并包装为现有 `Texture` 对象。
- `Renderer` 新增 `precomputeEnvironment(...)`，按 environment capture、irradiance、prefilter、BRDF LUT 的顺序调用 `IBLPrecomputePass`。
- `SceneSetup` 新增 `prepareEnvironmentPrecompute(...)`，在 profile 启用且配置 HDR path 时创建 capture cube / BRDF LUT quad 并触发预计算。
- `AppRuntimeContext` 持有 `EnvironmentProfile`，后续 UI 或配置系统可以修改同一份 profile 来重建 IBL 资源。

这一步让 IBL 从“有 pass”推进到“有可配置触发点”。当前项目没有 `.hdr/.exr` 资源，所以默认不启用；后续只需要给 `EnvironmentProfile::hdrEquirectangularPath` 配置有效 HDR 文件并打开 `precomputeOnPrepare`，就可以生成 IBL 贴图。下一步应把 irradiance / prefilter / BRDF LUT 绑定进 `PBRMaterial` 的 shader path。

### 2026-05-20 PBR 可选 IBL 绑定路径

PBR shader path 已能消费 IBL 资源，但默认仍关闭：

- `PBRMaterial` 新增 `Use IBL`、`IBL Diffuse Strength`、`IBL Specular Strength` inspector 参数。
- `EnvironmentRenderTargets` 新增 `hasPrecomputedEnvironment` 状态，只有 `Renderer::precomputeEnvironment(...)` 完整成功后才标记为 ready。
- `Renderer` 将 `EnvironmentRenderTargets` 传入 `SceneRenderPass`，再由 `MaterialBinder` 在绑定 PBR 材质时决定是否上传 IBL sampler。
- `MaterialBinder` 只有在 PBR 材质启用 IBL 且 environment 已预计算完成时，才绑定 irradiance cubemap、prefilter cubemap 和 BRDF LUT。
- `shaders/pbr/pbr.frag` 新增直接光 + 可选 IBL ambient 组合，默认 `useIBL=0` 时保持原 ambient 路径。

这一步完成了 PBR 采样 IBL 资源所需的绑定链路。后续重点是提供 HDR environment 资源、打开 profile 预计算，并在 UI / 配置层控制 `EnvironmentProfile`，让用户不改代码就能切换环境。

### 2026-05-20 EnvironmentProfile UI 入口

`EnvironmentProfile` 已有 Debug UI 入口和手动预计算触发：

- `Renderer::precomputeEnvironment(const EnvironmentProfile&)` 统一负责 HDR 加载、capture cube / BRDF quad 创建和 IBL 预计算调度。
- `SceneSetup` 改为调用 `Renderer` 的高层 profile 入口，不再重复创建 IBL capture mesh 或直接加载 HDR texture。
- `DebugControllerPanel` 新增 `Environment / IBL` 控制区，支持编辑 HDR path、HDR texture unit、prepare 阶段预计算开关、查看 IBL ready 状态，并手动触发 `Precompute IBL Now`。
- 默认 profile 仍不配置 HDR path，也不自动预计算，所以当前启动路径和原画面保持稳定。

这一步把 environment 切换从代码路径推进到 UI / profile 路径。后续应优先补一份默认 HDR environment 或 profile 持久化机制，用真实 HDR 资源验证 PBR IBL 效果，而不是只验证默认关闭路径。

### 2026-05-20 EnvironmentProfile 持久化入口

`EnvironmentProfile` 已支持本地配置保存 / 加载：

- 新增 `EnvironmentProfileStorage`，负责读取和写入简单 key-value 格式的 environment profile。
- 默认运行时配置路径为 `config/environment_profile.local.ini`，并通过 `.gitignore` 忽略，避免用户在 UI 中保存本地 HDR 路径后污染 Git 提交。
- 仓库新增 `config/environment_profile.example.ini`，作为可提交的字段示例和默认格式说明。
- 启动流程在 `prepare()` 前尝试加载 local profile，因此 `precomputeOnPrepare` 和 HDR path 可以从配置驱动 scene setup。
- `DebugControllerPanel` 的 `Environment / IBL` 区域新增 Save / Reload 操作，可以在运行时保存或重新读取 environment profile。

这一步让 PBR / IBL environment 从“运行时手动输入”推进到“可持久化实验配置”。下一步应使用真实 HDR 文件验证从 local profile 加载、启动预计算、PBR 材质启用 IBL 的完整效果链路。

### 2026-05-20 PostProcessSettings 提升到 Runtime

后处理参数已从 `ScreenMaterial` 移到 runtime 配置：

- `AppRuntimeContext` 新增 `PostProcessSettings postProcessSettings`，作为 tone mapping 和 Bloom 参数的唯一运行时来源。
- `runFrame()` 使用 runtime settings 决定是否执行 Bloom extract / blur，并传入 threshold / iterations。
- `PostProcessPass::renderScreenComposite(...)` 显式接收 `PostProcessSettings`，不再从 `ScreenMaterial` 读取 exposure、tone mapping mode 或 Bloom 参数。
- `ScreenMaterial` 现在只保留 screen/depth/bloom texture 输入，inspector 中也只展示后处理输入贴图。
- `DebugControllerPanel` 新增 `Post Process` 控制区，直接编辑 runtime-level settings。

这一步把后处理从“材质属性”恢复为“渲染管线配置”。PBR 输出进入 HDR -> Bloom -> tone mapping 时，后续可以继续把这些设置持久化为 profile 或迁移到更明确的 renderer/runtime settings 模块，而不需要依赖一个 screen quad 材质对象。

### 2026-05-20 FrameRenderTargets Resize 生命周期

主帧渲染目标已支持窗口 resize 后重建：

- `FrameRenderTargets` 新增 `resize(width, height)`，复用当前 MSAA samples，重建 multisample scene target、resolved HDR target、Bloom bright/ping/pong targets。
- `FrameRenderTargets::initialize(...)` 对 0 尺寸做保护，避免最小化窗口时创建非法 framebuffer。
- `FrameRenderTargets` 新增 resolved depth-stencil、Bloom pong color attachment getter，避免调用方继续穿透到具体 framebuffer。
- `OnResize(...)` 现在会更新全局尺寸、viewport、perspective camera aspect，并在 framebuffer 重建后刷新 `ScreenMaterial` 的 screen/depth/bloom texture 输入。
- `SceneSetup` 初始 screen pass 也改为通过统一 getter 绑定 resolved color、resolved depth-stencil 和 Bloom pong texture。

这一步修复了 HDR / Bloom render target 只在启动时创建的问题。后续调整窗口尺寸时，PBR 输出链路仍会进入匹配当前 framebuffer 尺寸的 HDR、Bloom 和 tone mapping 目标；后续还可以继续把 camera resize 行为从 `main.cpp` 收敛到独立 camera/runtime 模块。

### 2026-05-20 PostProcessSettings 持久化入口

`PostProcessSettings` 已支持本地配置保存 / 加载：

- 新增 `PostProcessSettingsStorage`，负责读取和写入 exposure、tone mapping mode、Bloom 开关、threshold、intensity 和 iterations。
- 默认运行时配置路径为 `config/postprocess_settings.local.ini`，复用 `config/*.local.ini` ignore 规则，避免本地调参结果污染提交。
- 仓库新增 `config/postprocess_settings.example.ini`，记录可提交的 postprocess profile 字段示例。
- 启动流程会在 `prepare()` 前加载 postprocess settings，让 HDR / Bloom / tone mapping 参数从配置驱动运行时。
- `DebugControllerPanel` 的 `Post Process` 区域新增 profile 文件路径显示、Save 和 Reload 操作。

这一步让后处理参数从“运行时临时调节”推进到“可保存的实验配置”。后续接真实 HDR environment 验证 PBR IBL 时，tone mapping 和 Bloom 参数可以随 profile 保存，不需要每次启动后重新调整。

### 2026-05-20 PBR 线性 HDR 输出修正

PBR shader 输出已与统一后处理链路对齐：

- `shaders/pbr/pbr.frag` 删除材质 shader 内部的 Reinhard tone mapping 和 gamma correction。
- PBR scene pass 现在输出线性 HDR color 到 resolved HDR / Bloom / screen composite 链路。
- tone mapping mode、exposure 和 gamma correction 统一由 `shaders/screen/screen.frag` 根据 `PostProcessSettings` 处理。

这一步避免 PBR 结果在 scene pass 和 screen pass 中被重复 tone mapping / gamma。后续验证 IBL 时，PBR direct light、IBL ambient、Bloom 和最终 tone mapping 会处在同一条 HDR 输出链路上，调参结果也更可预测。

### 2026-05-20 Procedural HDR Environment 入口

IBL 预计算现在不再强依赖外部 `.hdr/.exr` 文件：

- `EnvironmentProfile` 新增 procedural environment 开关、分辨率、sky / ground / sun intensity 参数。
- `EnvironmentTextureLoader` 新增 `loadEquirectangular(...)` 统一入口；当 profile 启用 procedural source 时，运行时生成 RGB16F equirectangular texture，否则继续加载 HDR 文件。
- `Renderer::precomputeEnvironment(const EnvironmentProfile&)` 和 `SceneSetup` 改为检查 `hasEnvironmentSource()`，HDR 文件和 procedural source 都能触发同一套 IBL precompute pass。
- `DebugControllerPanel` 的 `Environment / IBL` 区域新增 procedural 参数 UI，可在不改代码、不引入外部资源的情况下手动触发 IBL 预计算。
- `config/environment_profile.example.ini` 记录 procedural 字段，后续本地 `config/environment_profile.local.ini` 可直接保存这一类测试环境。

这一步的目的不是替代真实 HDR environment，而是提供一条可提交、可复现、无资源授权和体积问题的 IBL 验证路径。后续应在 PBR preview 材质上默认打开 `Use IBL` 或提供一键测试 profile，用 procedural source 先验证完整链路，再切换到真实 HDR 资源做视觉质量确认。

### 2026-05-20 PBR Preview IBL 实验入口

默认测试场景的 `PBR Preview Sphere` 现在会请求 IBL：

- `preparePBRPreview(...)` 创建的 `PBRMaterial` 默认设置 `mUseIBL=true`。
- 如果 environment 尚未完成预计算，`MaterialBinder` 仍会根据 `EnvironmentRenderTargets::hasPrecomputedEnvironment()` 在 shader 侧关闭 IBL，因此缺省启动路径保持安全。
- 当 `EnvironmentProfile` 启用 procedural source 并完成 precompute 后，同一个 preview sphere 会自动进入 IBL 采样路径，不需要再选中材质手动打开 `Use IBL`。

这一步把上一轮 procedural environment 从“可生成 IBL 资源”推进到“默认 PBR 测试对象会消费 IBL 资源”。后续建议把这个 preview setup 进一步提升为可配置的 PBR test scene / material preset，避免把实验场景参数长期写死在 `SceneSetup.cpp` 中。

### 2026-05-20 PBR Preview Profile 配置化

默认 PBR 测试球已从硬编码实验参数迁移到 profile：

- 新增 `PBRPreviewProfile`，集中描述 preview sphere 的启用开关、位置、半径、细分数、PBR surface 参数、IBL 强度和 normal map。
- 新增 `PBRPreviewProfileStorage`，从 `config/pbr_preview.local.ini` 读取本地 preset；仓库提供 `config/pbr_preview.example.ini` 作为字段模板。
- `AppRuntimeContext` 持有 `pbrPreviewProfile` 和 profile path，并在 `prepare()` 前加载，保证 scene setup 阶段只消费已解析配置。
- `SceneSetup::preparePBRPreview(...)` 不再直接写死材质和 mesh 参数，而是根据 profile 构建 `PBR Preview Sphere`。
- `config/*.local.ini` 已覆盖本地 profile，用户可以为不同 PBR / IBL 实验保留本机 preset，不会污染 Git 分支。

这一步把 PBR 验证从“改 C++ 代码调实验球”推进到“改 local profile 调实验球”。后续可以把 profile 扩展为多 material preset / 多球阵列，用同一套 environment 与 postprocess 设置同时比较 roughness、metallic 和 IBL 强度。

### 2026-05-20 PBR Preview Material Grid

PBR preview profile 现在支持材质对比阵列：

- `PBRPreviewProfile` 新增 `useMaterialGrid`、行列数、spacing、grid radius、metallic range 和 roughness range。
- 单球模式保持兼容；`useMaterialGrid=0` 时仍按 `metallic` / `roughness` 创建一个 `PBR Preview Sphere`。
- grid 模式会复用同一个 sphere geometry 和 normal map，根据列插值 metallic、根据行插值 roughness，生成一组 `PBR Preview Mx Ry` 测试球。
- grid 行列数在 scene setup 中限制到 `1..10`，避免 local profile 写错导致一次创建过多 mesh。
- `config/pbr_preview.example.ini` 已启用 5x5 示例阵列，用户复制为 local profile 后即可在同一 environment / postprocess 下比较 PBR 参数。

这一步让 PBR / IBL 验证从单点观察推进到批量对比。下一步更有价值的是把这组 preview 对象的运行时 UI 暴露出来，或者把 environment / postprocess / preview 三类 profile 合并成一个更高层的 experiment preset。

### 2026-05-20 PBR Experiment Profile

PBR 实验配置现在有一个高层覆盖入口：

- 新增 `PBRExperimentProfileStorage`，默认读取 `config/pbr_experiment.local.ini`。
- experiment preset 使用 `environment.*`、`postprocess.*`、`pbrPreview.*` 三类前缀，能在一个文件中覆盖 environment source / IBL precompute、HDR postprocess 和 PBR preview grid。
- 启动流程仍先加载原有 `environment_profile.local.ini`、`postprocess_settings.local.ini`、`pbr_preview.local.ini`，再加载 `pbr_experiment.local.ini` 作为最终覆盖层。
- 仓库新增 `config/pbr_experiment.example.ini`，提供 procedural IBL + postprocess + 5x5 PBR material grid 的组合示例。
- `enabled=0` 时 experiment preset 会被读取但不应用，方便临时保留配置文件。

这一步解决的是实验配置分散的问题。后续如果继续推进，建议把 `PBRExperimentProfile` 接入 Debug UI 的 Load/Reload/Save 或 preset 下拉，避免运行时仍需要手动编辑 local ini。

### 2026-05-20 ProfileConfigParser 共享解析工具

Profile 解析的重复基础设施已收敛：

- 新增 `tools/config/ProfileConfigParser`，统一提供 `trim`、`startsWith`、`readKeyValueFile`、`parseBool`、`parseFloat`、`parseInt`、`parseUnsigned`。
- `EnvironmentProfileStorage`、`PostProcessSettingsStorage`、`PBRPreviewProfileStorage`、`PBRExperimentProfileStorage` 不再各自维护一份 key-value 文件遍历和基础类型解析函数。
- 各 profile 仍保留自己的字段映射逻辑，避免把配置语义塞进通用 parser。
- VS 工程已纳入 `ProfileConfigParser.cpp/.h`，后续新增 PBR material preset 或 experiment preset 不需要再复制解析工具。

这一步降低了继续扩展 PBR 配置体系的维护成本。后续更合理的方向是进一步把字段映射做成声明式 schema，让配置文件、Debug UI 和 inspector 可以共享同一份属性描述。

### 2026-05-20 Redundant third_party Archive Cleanup

仓库清理了未引用的 `third_party.zip`：

- `third_party.zip` 是已跟踪的大体积归档文件，但当前工程已经直接使用解压后的 `third_party/` 目录。
- 通过全文搜索确认仓库代码、工程文件和配置没有引用 `third_party.zip`。
- 删除归档不改变构建输入，只减少仓库体积并释放本地 C 盘空间。

这一步不是渲染功能重构，但它恢复后续完整 build / smoke 的可靠性。当前 PBR 路径重构已经开始依赖频繁编译和短启动验证，保留冗余二进制归档会让每轮验证更容易被磁盘空间阻塞。

### 2026-05-20 PropertyInspector 与 PBR Preview 自动 UI

PBR preview profile 已开始接入系统化属性 UI：

- 从 `MaterialInspector` 中拆出通用 `PropertyInspector`，集中维护 `PropertyBuilder`、`PropertyDescriptor` 和 `drawProperties(...)`。
- `PropertyInspector` 新增可编辑 string 字段，后续路径类配置不需要再手写固定 char buffer。
- `MaterialInspector` 保留材质专属的类型名和贴图描述逻辑，只复用通用属性绘制器。
- `PBRPreviewProfile` 新增 `visitEditableProperties(...)`，用同一组 descriptor 描述 enabled、position、geometry、material grid、PBR surface、IBL 和 normal map 配置。
- `DebugControllerPanel` 新增 `PBR Preview Profile` 折叠区，直接消费 `PBRPreviewProfile::visitEditableProperties(...)` 生成 UI，并支持保存 / 重载 `config/pbr_preview.local.ini`。
- 当前 geometry / grid 修改仍在 scene prepare 时生效，运行时 UI 会明确提示需要保存后重启或重新 prepare 才会重建 preview objects。

这一步是自动 UI 的第一层落地：新增 PBR preview profile 字段时，不再需要在 Debug 面板中重复写一组 `ImGui::Slider...`。后续更合理的推进方式是把 `EnvironmentProfile` 和 `PostProcessSettings` 也迁移到同一套 property descriptor，再把 descriptor 与 ini load/save 的 key schema 合并。

### 2026-05-20 PropertySchema 与 Profile UI 继续收敛

Profile UI 的通用层继续拆分：

- 新增 `PropertySchema`，只包含 `PropertyKind`、`PropertyDescriptor` 和 `PropertyBuilder`，不依赖 ImGui。
- `PropertyInspector` 收敛为 ImGui 绘制层，只负责把 property schema 渲染成控件。
- `EnvironmentProfile` 新增 `visitEditableProperties(...)`，用 schema 描述 HDR path、texture unit、procedural environment、precompute 开关和 procedural intensity 参数。
- `PostProcessSettings` 新增 `visitEditableProperties(...)`，用 schema 描述 exposure、tone mapping mode 和 Bloom 参数。
- `DebugControllerPanel` 的 Environment / Post Process 字段 UI 改为消费 profile schema，不再手写对应的 `InputText`、`SliderInt`、`SliderFloat`、`Checkbox` 控件。

这一步把自动 UI 从 PBR Preview 扩展到 environment 与 postprocess。更关键的是 schema 与 ImGui 绘制器被拆开，后续配置读写、preset diff、inspector 或非 ImGui 工具都可以复用字段描述，而不是被 UI 框架绑定。

### 2026-05-20 ProfileConfigIO 与 PostProcess Schema 存取

Postprocess 配置读写已开始复用 property schema：

- `PropertyDescriptor` 新增 `configKey`，`PropertyBuilder` 新增 `addConfigFloat`、`addConfigInt`、`addConfigBool`、`addConfigString`。
- 新增 `ProfileConfigIO`，根据 descriptor 的 `configKey`、字段类型和 getter / setter 执行通用 ini load/save。
- `PostProcessSettings::visitEditableProperties(...)` 的 exposure、tone mapping、Bloom 字段现在同时描述 UI 与配置 key。
- `PostProcessSettingsStorage::loadFromFile(...)` / `saveToFile(...)` 不再维护手写 key 分支和手写输出顺序，而是通过 `ProfileConfigIO` 消费同一份 property schema。
- `ProfileConfigParser` 继续负责底层 key-value 文件读取和基础类型解析，`ProfileConfigIO` 负责把这些 key-value 应用到 descriptor。

这一步先只迁移 `PostProcessSettings`，因为它字段少、无 vector 拆分、风险最低。方向验证通过后，可以继续迁移 `EnvironmentProfile`，再处理 `PBRPreviewProfile` 中 position / albedo 这类 UI 是 vec3、配置是多个 key 的字段映射。

### 2026-05-20 EnvironmentProfile Schema 存取

Environment profile 配置读写已迁移到 `ProfileConfigIO`：

- `EnvironmentProfile::visitEditableProperties(...)` 的 HDR path、texture unit、procedural source、precompute 和 procedural intensity 字段现在同时描述 UI 与配置 key。
- `EnvironmentProfileStorage::loadFromFile(...)` / `saveToFile(...)` 不再维护手写 key 分支和手写输出逻辑，而是通过 `ProfileConfigIO` 消费同一份 property schema。
- `EnvironmentProfile.cpp` 删除了不再需要的 `<filesystem>`、`<fstream>` 和 `ProfileConfigParser` 直接依赖。
- Environment / IBL Debug UI、local ini 读写和后续 preset tooling 现在共享同一份字段描述。

这一步把 environment 与 postprocess 都推进到 schema-driven 配置读写。后续剩下最主要的配置重复点是 `PBRPreviewProfile` 的 position / albedo 这类 vec3 字段目前在 UI 中是一项，在 ini 中仍拆成多个 key；下一步需要给 schema 增加“一个 UI 属性对应多个 config key”的能力，或者明确把 profile 文件格式升级为 vector key。

### 2026-05-20 PBRPreviewProfile Schema 存取

PBR preview 配置读写已迁移到 `ProfileConfigIO`：

- `PropertyDescriptor` 新增 `configKeys`，支持一个 UI 属性映射到多个配置 key。
- `PropertyBuilder` 新增 `addConfigVec3(...)` 和 `addConfigColor3(...)`，用于保持 `Position`、`Albedo` 这类 UI 字段的一体化，同时继续读写 `positionX/Y/Z`、`albedoR/G/B` 这类兼容旧 ini 的拆分字段。
- `ProfileConfigIO` 现在能按 component 读写 `Vec3` / `Color3` descriptor，不需要 profile 自己维护三个 key 分支。
- `PBRPreviewProfile::visitEditableProperties(...)` 现在同时描述 Debug UI 与 `config/pbr_preview.local.ini` 的完整字段。
- `PBRPreviewProfileStorage::loadFromFile(...)` / `saveToFile(...)` 不再维护手写 key 分支和手写输出逻辑。

这一步把 `PostProcessSettings`、`EnvironmentProfile` 和 `PBRPreviewProfile` 都推进到 schema-driven 配置读写。下一步更有价值的重复点是 `PBRExperimentProfile`：它仍然手写 `environment.*`、`postprocess.*`、`pbrPreview.*` 的 prefixed key 应用逻辑，应该改为复用各 profile 的 schema。

### 2026-05-20 PBRExperimentProfile Schema 覆盖

PBR experiment preset 的 prefixed key 应用逻辑已开始复用 profile schema：

- `ProfileConfigIO` 新增 `applyPropertyConfigValue(...)`，允许调用方把单个 key-value 应用到已有 `PropertyBuilder`。
- `loadPropertyConfig(...)` 内部也复用同一个单 key 应用函数，避免普通 profile load 与 experiment preset 形成两套行为。
- `PBRExperimentProfileStorage::loadFromFile(...)` 现在为 environment、postprocess 和 PBR preview 的临时副本分别构建 schema，再把 `environment.*`、`postprocess.*`、`pbrPreview.*` 去掉前缀后交给对应 schema。
- 删除了 `PBRExperimentProfile.cpp` 中重复维护的 environment / postprocess / PBR preview 字段分支。

这一步让单 profile local ini 与高层 experiment ini 共用字段来源。后续如果新增 PBR material 参数，正常路径应该是先在 profile schema 增加 descriptor，然后 local profile、Debug UI 和 experiment preset 同时获得该字段能力。

### 2026-05-20 PBR Texture Slot Schema

PBR 材质贴图槽开始从手写绑定收敛到声明式描述：

- `PBRMaterial` 新增 `PBRTextureSlot` / `PBRConstTextureSlot`，集中描述每个 PBR texture slot 的 Inspector label、sampler uniform、use flag uniform 和 texture 指针。
- `PBRMaterial::getTextureSlots()` 通过一份内部 metadata 表生成 slot 列表，避免 sampler 名称和 use flag 名称散落在 binder / inspector 多处。
- `PBRMaterial::visitEditableProperties(...)` 现在遍历 texture slot 列表生成贴图状态文本，不再为 albedo / metallic / roughness / AO / normal / emissive 各写一行。
- `MaterialBinder::bindPBRMaterial(...)` 现在遍历同一份 texture slot 列表绑定可选贴图和对应 use flag，不再手写 6 组 `bindOptionalTexture(...)`。

这一步没有改变 shader uniform 名称和现有视觉输出，但建立了正式的 PBR texture slot 边界。后续新增 clearcoat、sheen、transmission 或 ORM packed texture 时，应先扩展 slot metadata，再处理 shader 采样逻辑。

### 2026-05-20 PBR Uniform Slot Schema

PBR 材质参数 uniform 也开始从手写绑定收敛到声明式描述：

- `PBRMaterial` 新增 `PBRVec3UniformSlot` / `PBRFloatUniformSlot` 及 const 版本，集中描述 Inspector label、shader uniform 名、字段指针和 float UI 范围。
- `PBRMaterial::getVec3UniformSlots()`、`getSurfaceFloatUniformSlots()`、`getIblFloatUniformSlots()` 都由内部 metadata 表生成。
- `PBRMaterial::visitEditableProperties(...)` 复用 uniform slot 生成 PBR surface / IBL 参数 UI，避免 UI label、范围和字段指针分散维护。
- `MaterialBinder::bindPBRMaterial(...)` 复用同一份 uniform slot 写入 `pbrAlbedo`、`pbrMetallic`、`pbrRoughness`、`pbrAo`、`pbrEmissive*` 和 IBL strength uniform。

这一步继续保持现有 shader uniform 名称不变，目标是降低新增 PBR 参数时的重复修改点。当前 `useIBL` 仍由 Binder 根据材质开关与 environment readiness 计算，因此暂时不纳入普通 bool slot。

### 2026-05-20 PBR Material Profile

PBR preview preset 已开始复用正式的材质 profile：

- `PBRMaterial` 新增 `PBRMaterialProfile`，集中保存 albedo、metallic、roughness、AO、emissive 和 IBL strength 等可配置材质参数。
- `PBRMaterialProfile::visitEditableProperties(...)` 负责生成可读写 ini 的 PBR surface / IBL schema，继续保持 `albedoR/G/B`、`metallic`、`roughness`、`ao`、`useIBL` 等既有 key 不变，并新增 emissive 相关 key。
- `PBRMaterialProfile::applyTo(...)` / `copyFrom(...)` 提供 profile 与真实 `PBRMaterial` 之间的转换入口。
- `PBRPreviewProfile` 不再直接维护一套 PBR surface / IBL 字段，而是持有 `PBRMaterialProfile material`。
- `SceneSetup::createPBRPreviewMaterial(...)` 现在先把 `profile.material` 应用到 `PBRMaterial`，material grid 再覆盖 metallic / roughness。
- `config/pbr_preview.example.ini` 和 `config/pbr_experiment.example.ini` 已补齐 emissive 字段示例。

这一步把“实验 preset 的材质参数”和“运行时 PBRMaterial”之间的边界明确下来。后续如果做独立材质库或 material preset 文件，可以直接复用 `PBRMaterialProfile`，不需要重新定义一套字段。

### 2026-05-20 Runtime Viewport Boundary

窗口 resize / camera aspect / postprocess 输入贴图刷新已从 `main.cpp` 拆出：

- 新增 `RuntimeViewport`，集中处理运行时 framebuffer 尺寸校验、OpenGL viewport 同步、PerspectiveCamera aspect 同步。
- `RuntimeViewport::applyResize(...)` 接收一个轻量 `RuntimeResizeContext`，统一更新应用级宽高、重建 `FrameRenderTargets`，并在 render target 重建后刷新 `ScreenMaterial` 的 resolved HDR、depth stencil 和 bloom 输入贴图。
- `main.cpp` 的 `OnResize(...)` 不再直接操作 `glViewport`、`PerspectiveCamera::mAspect`、`FrameRenderTargets::resize(...)` 或 screen material 贴图字段，只负责把窗口回调参数转交给 runtime viewport 边界。
- 初始化 viewport 和 ImGui 绘制前的 default framebuffer viewport 设置也统一通过 `RuntimeViewport::applyViewport(...)` 执行。
- `RuntimeViewport.cpp/.h` 已加入 Visual Studio 工程和 filters，保持 VS 分类与物理文件一致。

这一步降低了主入口对渲染目标和 camera 类型的直接耦合。后续 PBR 路径如果新增更多 resize-sensitive 资源，例如 clustered light grid、screen-space reflection history、temporal accumulation target，应继续挂到 runtime resize 协调边界，而不是回到 `main.cpp`。

### 2026-05-20 Runtime Input Boundary

Camera input / 临时 FOV 交互已从 `main.cpp` 拆出：

- 新增 `RuntimeInputController`，集中处理 scroll、keyboard、mouse、cursor 对 `CameraControl` 的分发。
- 中键按下临时缩小 `PerspectiveCamera::mFovy`、中键释放恢复 FOV 的逻辑迁移到 `RuntimeInputController::applyTemporaryFovZoom(...)`。
- `RuntimeInputContext` 显式传入当前 `Camera` 和 `CameraControl`，避免输入模块直接依赖全局变量。
- `main.cpp` 的 GLFW 回调现在只负责取 cursor position、保留 debug log，并把输入事件转交给 runtime input 边界。
- `RuntimeInputController.cpp/.h` 已加入 Visual Studio 工程和 Application filter。

这一步继续降低主入口复杂度。后续如果引入 editor camera、game camera、preview orbit camera 或 PBR 材质调试专用相机，应把输入路由扩展在 runtime input 边界内，而不是继续把 camera-specific 分支写回 `main.cpp`。

### 2026-05-20 PBR Material Preset Files

PBR material preset 已从 preview profile 中拆出独立文件入口：

- `PBRMaterialProfile` 新增 `PBRMaterialProfileStorage`，支持通过同一份 property schema 保存 / 加载 `config/pbr_material.local.ini`。
- 新增 `config/pbr_material.example.ini`，作为独立材质 preset 示例，并加入 Visual Studio 的 `资源文件\config` filter。
- `PBRPreviewProfile` 新增 `materialProfilePath`，可以在 preview profile 或 experiment profile 中引用独立材质 preset。
- `PBRPreviewProfileStorage::loadFromFile(...)` 加载 preview profile 后会应用 `materialProfilePath` 指向的材质 preset；`PBRExperimentProfileStorage::loadFromFile(...)` 也会在应用 `pbrPreview.*` 字段后解析同一引用。
- `DebugControllerPanel` 的 PBR Preview Profile 区域新增 `Save PBR Material Profile` / `Reload PBR Material Profile` 按钮；当 `materialProfilePath` 为空时，保存会默认使用 `config/pbr_material.local.ini`。
- `config/pbr_preview.example.ini` 和 `config/pbr_experiment.example.ini` 已补充 `materialProfilePath` 示例字段。

这一步让“材质参数 preset”和“preview 几何 / grid / normal map 设置”分离。后续新增 ORM packed texture、clearcoat 或 transmission 参数时，应先扩展 `PBRMaterialProfile` schema，然后独立 material preset、preview profile、experiment preset 和 Debug UI 会共享同一套字段来源。

### 2026-05-20 PBR Experiment Debug UI

PBR experiment preset 已接入运行时 Debug UI：

- `ProfileConfigIO` 新增公开的 `writePropertyConfig(...)`，允许调用方把同一份 property schema 用指定 prefix 写出。
- `PBRExperimentProfileStorage` 新增 `saveToFile(...)`，把 environment、postprocess、PBR preview 三组 schema 分别写成 `environment.*`、`postprocess.*`、`pbrPreview.*`。
- `DebugControllerContext` 新增 `pbrExperimentProfilePath`，由 `main.cpp` 传入当前 experiment preset 路径。
- `DebugControllerPanel` 新增 `PBR Experiment Preset` 面板，可保存 / 重载组合 preset，不再必须手动编辑 `config/pbr_experiment.local.ini`。
- experiment preset 的 reload 继续走 `PBRExperimentProfileStorage::loadFromFile(...)`，因此仍会应用 `pbrPreview.materialProfilePath` 指向的独立 material preset。

这一步让 environment、postprocess、preview grid、material preset 引用可以作为一个实验组合保存和恢复。后续如果增加更多 PBR 实验域，例如 light rig、camera rig 或 SSR / TAA 参数，应优先纳入 experiment preset 层，而不是分散到多个独立手工步骤。

### 2026-05-20 PBR Light Rig Profile

PBR experiment preset 已纳入 light rig 维度：

- 新增 `PBRLightRigProfile`，集中描述 ambient、directional、spot 和最多 2 个 point light 的位置、旋转、颜色、强度、specular 与 attenuation 参数。
- `SceneSetup::prepareLights(...)` 不再手写默认 light 初始化，而是把运行时 light 创建 / 更新委托给 `PBRLightRigProfile::applyTo(...)`。
- `PBRLightRigProfile::copyFrom(...)` 支持从当前运行时 light 状态回写 profile，Debug UI 保存 experiment preset 前会先同步当前灯光状态。
- `PBRExperimentProfileStorage` 现在同时读写 `lightRig.*` prefixed key，`config/pbr_experiment.example.ini` 已补齐对应字段示例。
- `DebugControllerContext` 中 light owner 改为指向 `shared_ptr` owner 的指针，避免 reload preset 时只修改 context 内部副本，确保 light rig reload 能真正更新运行时灯光对象。

这一步让一次 PBR experiment preset 能恢复 environment、postprocess、preview/material 和 light rig。下一步更合理的是补 camera rig profile，把观察位置、FOV 和 near/far 也纳入实验恢复范围；之后再继续把 application lifecycle 从 `main.cpp` 拆到 runtime bootstrapper。

### 2026-05-20 PBR Camera Rig Profile

PBR experiment preset 已纳入主相机视角维度：

- 新增 `PBRCameraRigProfile`，集中描述主相机 position、up、right、fovy、nearPlane 和 farPlane。
- `cameraRig.*` 字段接入 `PBRExperimentProfileStorage`，可以跟 environment、postprocess、preview/material 和 light rig 一起保存 / 加载。
- `PBRCameraRigProfile::applyTo(...)` 只恢复视角和裁剪参数，不写入 aspect；aspect 仍由窗口尺寸和 `RuntimeViewport` 负责，避免 preset 与 resize 逻辑冲突。
- Debug UI 保存 PBR experiment preset 前会从当前主相机回写 camera rig，重载 preset 后会应用到当前主相机。
- 启动时 `loadPBRExperimentProfile()` 在读取 experiment preset 后也会应用 camera rig，因此同一 preset 可以稳定恢复观察位置和 PBR 比较视角。

这一步补齐了 PBR 实验可复现性的关键维度。后续更值得处理的是 `main.cpp` 仍然持有大量 runtime context 和初始化编排；下一步应把 application lifecycle 拆到 runtime bootstrapper，而不是继续在主入口堆新的 profile 字段。

### 2026-05-20 Runtime Bootstrapper Boundary

Application lifecycle 已开始从 `main.cpp` 拆出：

- 新增 `AppRuntimeContext`，把 renderer、scene、postprocess、environment、PBR preview、light rig、camera rig、camera、light 和 editor 相关运行时状态从 `main.cpp` 的本地 struct 移到 application 层。
- 新增 `RuntimeBootstrapper`，集中执行 initialize -> update loop -> per-frame render -> cleanup -> destroy 的生命周期骨架。
- `main()` 现在只负责设置日志等级，并把 initialize、shouldContinue、runFrame、cleanup 和 destroy 回调交给 bootstrapper。
- 这一步没有移动具体的 prepare / profile load / render pass 细节，目的是先建立稳定边界，避免一次性改动过大。

下一步可以继续把 `initializeApplication()` 拆成 `RuntimeStartupSequence` 或更具体的 profile loading / scene preparation / ImGui startup 阶段；也可以把 `runFrame()` 的 frame orchestration 移入 runtime frame runner，使 `main.cpp` 最终只保留 callback glue 和少量兼容旧接口的过渡代码。

### 2026-05-20 Runtime Profile Loader

Profile loading 阶段已从 `main.cpp` 拆出：

- 新增 `RuntimeProfileLoader`，集中加载 environment、postprocess、PBR preview 和 PBR experiment profile。
- `RuntimeProfileLoader::loadAll(...)` 保留现有分层顺序：先加载单独 profile，再加载高层 experiment preset 作为最终覆盖层。
- Experiment preset 成功加载后，camera rig 仍会立即应用到当前主相机，保持启动时恢复观察视角的行为。
- `initializeApplication()` 不再直接维护四个 profile load 函数，只在 camera 准备完成后调用 profile loader 阶段。

这一步把 PBR 实验配置恢复逻辑收敛到 application 层。下一步可以继续抽 `RuntimeScenePreparer`，把 scene setup、legacy experiment preparation 和相关 context 构建从 `main.cpp` 移出；也可以先抽 `RuntimeGuiStartup`，把 ImGui 初始化和 frame UI 绘制入口拆清楚。

### 2026-05-20 Runtime Scene Preparer

Scene preparation 阶段已开始从 `main.cpp` 拆出：

- 新增 `RuntimeScenePreparer`，集中构建 `GL_SCENE::SetupContext` 并调用 `prepareDefaultScene(...)`。
- Legacy experiment 的运行时 context 构建和 update 入口也移动到 `RuntimeScenePreparer`，`main.cpp` 不再直接知道 legacy experiment 需要哪些 renderer / scene / material / light 字段。
- `RuntimeScenePrepareConfig` 保留窗口尺寸、skybox texture path 和 legacy grass grid 参数，作为从旧入口向 application 层传递准备参数的过渡结构。
- 默认 legacy experiment 仍保持注释禁用状态，避免这次结构重构改变启动行为。

这一步让 `initializeApplication()` 的场景准备阶段收敛成单一 application-level 调用。后续更合理的拆分是继续抽 `RuntimeFrameRunner`，把 `runFrame()` 中的 offscreen render、MSAA resolve、Bloom、screen composite 和 UI 绘制从主入口移出；之后再考虑更正式的 `FramePipeline` / `RenderPipeline` 结构。

### 2026-05-20 Runtime Frame Runner

Frame orchestration 阶段已开始从 `main.cpp` 拆出：

- 新增 `RuntimeFrameRunner`，集中执行 camera control update、legacy experiment update、offscreen scene render、MSAA resolve、Bloom bright extraction / blur、screen composite 和 frame UI callback。
- `RuntimeFrameConfig` 目前只携带 default framebuffer 尺寸，避免 frame runner 直接依赖 `GL_APP`。
- `RuntimeFrameCallbacks` 以 callback 形式保留 UI 绘制入口，先不把 ImGui host 和 frame pipeline 混在一个类中。
- `main.cpp` 的 `runFrame()` 现在只负责把 runtime context、legacy experiment runner、framebuffer size 和 `renderIMGUI` callback 交给 frame runner。

这一步把 PBR 后续最重要的每帧渲染顺序从主入口移出。后续新增 PBR depth prepass、shadow atlas、deferred G-buffer 或 SSR / TAA 时，应优先扩展 runtime frame / pipeline 边界，而不是回到 `main.cpp` 添加 pass。

### 2026-05-20 Runtime Gui Host

ImGui host 阶段已开始从 `main.cpp` 拆出：

- 新增 `RuntimeGuiHost`，集中处理 ImGui context 创建、backend 初始化、每帧 NewFrame / Render / RenderDrawData。
- `RuntimeGuiHost::renderFrame(...)` 在提交 ImGui draw data 前恢复 default framebuffer viewport，保留原本 UI 绘制前的 viewport 同步行为。
- UI 面板内容暂时通过 `RuntimeGuiFrameContext::drawPanels` callback 提供，避免把 editor panel 数据装配和 ImGui backend 生命周期混在一个类里。
- `main.cpp` 不再直接 include `imgui.h`、`imgui_impl_glfw.h` 或 `imgui_impl_opengl3.h`，只保留 `drawEditorPanels()` 作为过渡 callback。

这一步把 UI backend 生命周期从主入口移出。后续如果继续降耦合，应把 `makeDebugControllerContext()`、`makeEditorPanelContext()` 和 selection 初始化迁移到 editor/runtime panel coordinator，而不是让 `main.cpp` 长期负责 editor 数据装配。

### 2026-05-20 Runtime Editor Panel Coordinator

Editor panel 数据装配已开始从 `main.cpp` 拆出：

- 新增 `RuntimeEditorPanelCoordinator`，集中构建 `DebugControllerContext` 和 `EditorPanelContext`。
- Hierarchy / SelectionInspector 所需的 selection 初始化也移动到 coordinator，避免 `main.cpp` 直接知道 editor panel 的绘制顺序和默认 selection 规则。
- `main.cpp` 的 `drawEditorPanels()` 现在只作为 `RuntimeGuiHost` 的无捕获 callback wrapper，实际面板装配由 coordinator 执行。
- 这一步没有改变 DebugControllerPanel、EditorPanels 的内部 UI 行为，只移动数据 wiring 边界。

这一步进一步降低 `main.cpp` 对 PBR debug UI / selection inspector 的了解程度。后续可以继续处理启动和回调边界：把 window/camera lifecycle 与 GLFW callback glue 收敛到 runtime application shell，让 PBR pipeline 扩展基本不再触碰主入口。

### 2026-05-20 Runtime Camera Lifecycle

Camera lifecycle 已开始从 `main.cpp` 拆出：

- 新增 `RuntimeCameraLifecycle`，集中创建默认 `PerspectiveCamera` 和 `GameCameraControl`。
- 新增 `RuntimeCameraConfig`，用窗口 width / height 计算初始 aspect，并保留 fovy / near / far 的默认配置入口。
- `cleanupRuntime()` 改为调用 `RuntimeCameraLifecycle::cleanup(...)`，主入口不再直接 delete camera / camera control。
- `main.cpp` 不再直接 include `perspectivecamera.h`、`orthographiccamera.h` 或 `gamecameracontrol.h`。

这一步让 camera 的创建策略具备独立扩展点。后续如果 PBR 预览需要 orbit camera、editor camera、preview camera 或从 preset 恢复不同 camera controller，应优先扩展 runtime camera lifecycle / camera factory，而不是回到主入口硬编码具体 camera 类型。

### 2026-05-20 Runtime Frame Pipeline

Frame render pipeline 已从 `RuntimeFrameRunner` 中拆出：

- 新增 `RuntimeFramePipeline`，集中执行 scene-to-MSAA target、scene color resolve、Bloom、screen composite 四个当前 frame pass。
- `RuntimeFrameRunner` 只保留 per-frame 高层顺序：camera control update、legacy experiment update、frame pipeline render、UI callback。
- `RuntimeFramePipelineConfig` 当前只携带 default framebuffer 尺寸，保持 pipeline 不直接依赖 `GL_APP`。
- 现有渲染行为不变，仍然先渲染 offscreen scene，再 resolve HDR color，按 `PostProcessSettings` 决定是否跑 Bloom，最后做 screen composite。

这一步为 PBR 渲染路径准备了更明确的 pass 边界。后续新增 PBR depth prepass、shadow atlas、deferred G-buffer、SSR/TAA 或透明 pass 时，应扩展 `RuntimeFramePipeline` 或继续拆出专门的 pass 类型，而不是把具体 pass 放回 frame runner 或 main。

### 2026-05-20 Runtime Window Lifecycle

Window setup 和输入 callback glue 已从 `main.cpp` 拆出：

- 新增 `RuntimeWindowLifecycle`，集中执行 `GL_APP->init(...)`、callback context 绑定和 Application callback 注册。
- 由于 `Application` 当前只接受 C 风格函数指针，`RuntimeWindowLifecycle.cpp` 内部用一个 runtime callback context 保存 `AppRuntimeContext`、window width 和 height 指针，再由内部静态回调转发到 `RuntimeInputController` / `RuntimeViewport`。
- Resize callback 继续负责更新 runtime width / height、viewport、camera aspect、FrameRenderTargets 和 ScreenMaterial postprocess 输入贴图。
- Keyboard / mouse / cursor / scroll callback 继续转发到 `RuntimeInputController`，旧行为保持不变。
- `main.cpp` 不再定义 `OnScroll`、`OnResize`、`OnKeyboardCallback`、`OnMouseCallback`、`OnCursor`、`setAndInitWindow` 或旧的 `processInput` / `keyCallBack` 函数。

这一步把窗口初始化和输入回调 wiring 从主入口移出。后续可以继续清理 `main.cpp` 剩余的全局 alias / legacy 参数，或者把 runtime startup sequence 聚合成一个更完整的 application shell。

### 2026-05-20 Main Runtime Alias Cleanup

`main.cpp` 剩余全局 alias 和遗留实验变量已开始清理：

- 移除 `main.cpp` 中对 `gAppRuntime` 各字段的大量 `auto&` alias，主入口不再直接展开 renderer、scene、material、light、profile 等 runtime 字段。
- 移除未使用的旧实验变量，例如 `scale`、`brigtnesee`、`angle`、`specularIntensity` 和旧 `moveit()`。
- 新增 `MainStartupConfig`，集中保存 window size、skybox texture path、legacy grass grid 和 editor orbit angle。
- `RuntimeWindowLifecycle`、`RuntimeScenePreparer` 和 `RuntimeEditorPanelCoordinator` 继续通过明确 config/context 获取需要的数据，不再依赖 main 中零散全局变量。
- 精简 `main.cpp` include 列表，只保留当前入口实际需要的 runtime/application/editor 类型。

这一步让 `main.cpp` 更接近“程序入口 + startup config + callback wrapper”。后续如果继续收口，可以把 startup sequence 聚合成 `RuntimeApplicationShell`，或把 `RuntimeFramePipeline` 内部 pass 继续拆成可替换对象。

### 2026-05-20 Runtime Application Shell

Runtime startup sequence 已从 `main.cpp` 聚合到 application shell：

- 新增 `RuntimeApplicationShell`，持有 `AppRuntimeContext`、editor selection、legacy experiment runner 和 startup config。
- Shell 负责 initialize / shouldContinue / runFrame / cleanup / destroy 的具体 wiring，并通过 `makeCallbacks()` 生成 `RuntimeBootstrapperCallbacks`。
- `main.cpp` 现在只负责设置日志等级、创建 shell、把 shell callbacks 交给 `RuntimeBootstrapper`。
- `RuntimeFrameCallbacks::renderUi` 与 `RuntimeGuiFrameContext::drawPanels` 从函数指针升级为 `std::function`，支持 shell 以成员函数形式提供 UI callback。
- 原本散落在 `main.cpp` 的 window init、camera init、profile load、scene prepare、GUI init、frame run、camera cleanup 和 destroy 顺序集中到 `RuntimeApplicationShell`。

这一步让主入口基本从 runtime orchestration 中退出。后续更值得继续推进的是把 `RuntimeFramePipeline` 的 pass 细化为可替换组件，或者补一层 PBR pipeline profile / pipeline feature toggle。

### 2026-05-20 Runtime Frame Pass Types

Runtime frame pipeline 的当前步骤已拆成显式 pass 类型：

- 新增 `RuntimeFramePasses`，包含 `RuntimeSceneColorPass`、`RuntimeSceneResolvePass`、`RuntimeBloomPass` 和 `RuntimeScreenCompositePass`。
- `RuntimeFramePipeline::render(...)` 现在只排列 pass 顺序，不再直接包含每个 pass 的具体执行细节。
- 当前 pass 行为保持不变：scene color 写入 MSAA target，resolve 到 HDR target，按 postprocess settings 运行 Bloom，再进行 screen composite。
- `RuntimeFramePipelineConfig` 继续作为 pipeline 层配置传给需要 framebuffer 尺寸的 screen composite pass。

这一步把 PBR pipeline 的扩展点从“函数内部某段代码”提升为“独立 pass 类型”。后续增加 depth prepass、PBR forward pass、G-buffer、shadow atlas、SSR/TAA 或透明 pass 时，可以按同一模式添加新的 pass 类型并调整 pipeline 顺序。

### 2026-05-20 Runtime Frame Pipeline Profile

Runtime frame pipeline 已新增第一层 profile / feature toggle：

- 新增 `RuntimeFramePipelineProfile`，保存 scene color、resolve、Bloom、screen composite 四个当前 pass 的启用状态。
- 新增 `RuntimeFramePipelineProfileStorage`，通过 `PropertySchema` / `ProfileConfigIO` 读写 `config/runtime_frame_pipeline.local.ini`。
- 新增 `config/runtime_frame_pipeline.example.ini`，作为本地实验配置字段示例。
- `RuntimeProfileLoader` 启动时会加载 frame pipeline profile；没有 local 配置时保持全部 pass 默认开启，当前渲染行为不变。
- `RuntimeFramePipeline::render(...)` 每帧按 profile 决定 pass 是否执行，方便隔离 PBR 后续新增 pass 或排查后处理链路。
- `DebugControllerPanel` 新增 Runtime Frame Pipeline 控制区，可运行时切换、保存和重载 pass toggles。

这一步不是最终的 pipeline 架构，只是先把“是否启用某个 pass”从代码常量移到 profile。下一步更合理的方向是把静态 pass 类型演进为可组合 pass list / pass registry：PBR depth prepass、shadow atlas、PBR forward、IBL debug、transparency、postprocess 都能按 profile 创建和排序，而不是继续在 `RuntimeFramePipeline::render(...)` 中堆 if。

### 2026-05-20 Runtime Frame Pass Registry

Runtime frame pipeline 已从硬编码 `if` 顺序推进到 pass registry：

- 新增 `RuntimeFramePassRegistry`，用 `RuntimeFramePassDefinition` 描述 pass id、调试名、启用判断和统一执行函数。
- 当前默认 pass list 包含 Scene Color、Scene Resolve、Bloom 和 Screen Composite，顺序与上一轮保持一致。
- `RuntimeFramePipeline::render(...)` 现在只遍历 `RuntimeFramePassRegistry::defaultPasses()`，不再直接读取 profile 字段或调用具体 pass 类型。
- 当前 profile toggle 仍然生效，但启用条件被收敛到 registry 条目内，pipeline 主流程只负责执行已启用 pass。
- VS 工程已加入 `RuntimeFramePassRegistry.cpp/.h`，保持物理文件和工程 filter 一致。

这一步让 PBR 扩展的主要修改点更明确：新增 PBR pass 时，优先新增 pass 类型与 registry 条目；如果需要用户配置，再扩展 profile/schema。后续可以继续把 `defaultPasses()` 从固定静态列表演进成由 profile 构建的 pass plan，这样 PBR forward、shadow atlas、IBL debug、transparent、postprocess 等路径可以按实验 preset 创建不同 pass 组合。

### 2026-05-20 Runtime PBR Verification Mode

针对“不能只验证旧 Phong 实验场景”的问题，新增可复现 PBR 验证模式：

- `main()` 支持 `--verify-pbr` 参数，使用独立 `RuntimeApplicationShellConfig` 启动 1280x720 验证窗口。
- 验证模式会关闭 ImGui，避免 UI overlay 干扰 framebuffer 证据。
- 新增 `RuntimePBRVerification`，启动时直接覆盖 runtime profile：启用 procedural environment / IBL precompute、5x5 PBR material grid、normal map、PBR light rig、camera rig、postprocess 和所有 frame pipeline pass。
- scene prepare 完成后会输出 PBR 验证统计：object / mesh / PBR mesh / PBR preview mesh 数量，以及 `iblReady` 状态。
- 验证模式会在指定帧读取 default framebuffer 并导出 `out/pbr_verification.ppm`，用于确认实际渲染输出，而不是只看进程能否启动。

当前验证命令：

```powershell
x64\Debug\text2.exe --verify-pbr
```

本轮实际验证结果显示：`pbrMeshes=25`、`pbrPreviewMeshes=25`、`iblReady=yes`，并导出 1280x720 PPM framebuffer。后续涉及 PBR 渲染路径的改动，应该优先运行这个验证模式；普通短启动只能作为“程序没崩”的补充证据，不能代替 PBR 场景验证。

### 2026-05-20 Runtime Frame Pass Plan

Runtime frame pipeline 已从固定默认 pass list 推进到 profile 驱动的 pass plan：

- `RuntimeFramePipelineProfile` 新增 `passOrder`，默认值为 `SceneColor,SceneResolve,Bloom,ScreenComposite`。
- `config/runtime_frame_pipeline.example.ini` 新增 `passOrder` 示例字段，local profile 可以按 key 调整 pass 执行顺序或临时裁剪 pass。
- `RuntimeFramePassDefinition` 新增稳定 `key`，当前 key 为 `SceneColor`、`SceneResolve`、`Bloom`、`ScreenComposite`。
- `RuntimeFramePassRegistry::buildPassPlan(...)` 会解析 profile 的逗号分隔 pass list，去重并忽略未知 token；如果没有得到有效 pass，则回退默认顺序，避免错误配置直接黑屏。
- `RuntimeFramePipeline::render(...)` 现在遍历 build 出来的 pass plan，不再直接使用固定 `defaultPasses()`。
- `--verify-pbr` 会显式设置完整 passOrder，避免本地实验配置影响 PBR 验证证据。

这一步让后续 PBR pass 的接入方式更明确：新增 pass 类型和 registry key 后，profile 可以决定它是否进入当前 pass plan。下一步可以开始补真实 PBR 专用 pass 槽位，例如 `PBRDepthPrepass`、`PBRShadowAtlas`、`PBRForward`、`IBLDebug`，并让 `PBRExperimentProfile` 或 runtime pipeline profile 选择具体组合。

### 2026-05-20 Renderer PBR Scene Pass

Renderer 内部已开始拆出真实 PBR 渲染边界：

- `RenderQueue` 继续保留全量 opaque / transparent 队列给 shadow 等既有系统使用，同时新增 legacy / PBR 子队列。
- 新增 `PBRSceneRenderPass`，只接受 `PBRMaterial` mesh，并复用 `MaterialBinder` 写入 PBR shader、IBL、light、shadow 相关 uniform。
- `SceneRenderPass::render(...)` 现在返回 draw call 数，便于验证 legacy 路径实际绘制量。
- `Renderer` 默认路径按 `legacy opaque -> PBR opaque -> legacy transparent -> PBR transparent` 执行；shadow pass 仍使用全部 opaque caster，避免 PBR mesh 从阴影投射中丢失。
- 如果设置了 `mGlobalMaterial`，Renderer 仍回退为原通用 scene pass 渲染全队列，保留旧的全局材质 override 行为。
- 新增 `RendererFrameStats`，记录 `shadowCasterCount`、`legacySceneDrawCalls` 和 `pbrSceneDrawCalls`。
- `--verify-pbr` 会在 capture 帧输出 renderer stats，用于证明 PBR mesh 实际走了 PBR 专用 pass。

本轮验证显示 `pbrMeshes=25` 且 `pbrDrawCalls=25`，说明 PBR preview grid 已经不再只是混在旧 scene pass 里渲染。后续可以继续把 PBR 专用 pass 拆成更细的 `PBRDepthPrepass`、`PBRShadowAtlas`、`PBRForward` 或 IBL debug pass。

### 2026-05-20 Renderer PBR Depth Prepass

PBR 路径已新增第一个真实前向渲染前置 pass：

- 新增 `PBRDepthPrepass`，只接收 PBR opaque mesh。
- depth prepass 使用现有 `DepthMaterial` shader 写 depth，关闭 color write，结束后恢复 color write。
- `Renderer` 在 shadow pass 之后、legacy / PBR scene pass 之前执行 PBR depth prepass。
- `RendererFrameStats` 新增 `pbrDepthPrepassDrawCalls`，用于验证 PBR depth prepass 实际绘制数量。
- `--verify-pbr` 输出 renderer stats 时会包含 `pbrDepthPrepassDrawCalls`。
- `mGlobalMaterial` override 路径继续跳过 PBR 专用 pass，保持旧的全局材质调试行为。

本轮验证显示 `pbrDepthPrepassDrawCalls=25` 且 `pbrDrawCalls=25`，说明 5x5 PBR preview grid 同时经过 depth prepass 和 PBR scene pass。后续如果要做 PBR shadow atlas、deferred G-buffer 或 clustered lighting，这个 depth prepass 可以继续扩展为深度资源生产点。

### 2026-05-20 Renderer Frame Stats Debug UI

Renderer 的 PBR 路径验证信息已接入 Debug UI：

- `DebugControllerPanel` 新增 `Renderer Frame Stats` 区块，直接读取 `Renderer::getLastFrameStats()`。
- UI 会显示 `PBR Path Active`、shadow caster 数、legacy scene draw call 数、PBR depth prepass draw call 数和 PBR scene draw call 数。
- 这让普通运行时也能确认当前场景是否实际经过 PBR 专用 depth / scene pass，而不是只能依赖 `--verify-pbr` stdout。

这一步的目的不是替代 `--verify-pbr`，而是补上人工调试入口：命令行验证负责可复现证据，Debug UI 负责运行时观察和切换实验配置时的即时反馈。

### 2026-05-20 Renderer Frame Pass Registry

Renderer 内部每帧 pass 顺序已从 `Renderer::render()` 主函数中拆出：

- 新增 `RendererFrameStats` 独立头文件，renderer stats 不再定义在 `renderer.h` 主类文件内。
- 新增 `RendererFrameContext`，集中传递当前 frame 的 scene、camera、light、render queue、shader library、environment targets、各 pass 实例和 stats。
- 新增 `RendererFramePassRegistry`，用稳定 `RendererFramePassKey` 描述 renderer 内部 pass。
- 默认 pass plan 为 `BeginFrame -> ShadowMaps -> PBRDepthPrepass -> LegacyOpaqueScene -> PBROpaqueScene -> LegacyTransparentScene -> PBRTransparentScene`。
- 全局材质 override 仍保留旧行为，但现在通过独立 pass plan 执行：`BeginFrame -> ShadowMaps -> GlobalMaterialScene`。
- `Renderer::render()` 现在只创建 frame context、选择 pass plan 并执行 registry pass，不再直接写死每个阶段的绘制细节。
- `RendererFrameStats` 新增 `rendererPassCount`，Debug UI 和 `--verify-pbr` 都会输出本帧执行的 renderer pass 数。

这一步继续降低 renderer 主函数耦合，但没有把 pass plan 暴露给用户配置。后续如果要接 `PBRShadowAtlas`、`IBLDebug`、G-buffer 或 clustered lighting，可以优先新增 `RendererFramePassKey` 和 pass 执行函数，再决定是否需要 profile-driven renderer pass plan。

### 2026-05-20 Shadow Render Pass Split

Shadow 渲染边界已从 `ShadowRenderer` 内部拆出：

- 新增 `DirectionalShadowRenderPass`，负责 directional CSM shadow map 的 layer 生成、FBO 绑定和 mesh depth draw。
- 新增 `PointShadowRenderPass`，负责 point light cubemap array shadow 的 face/layer 绘制。
- 新增 `ShadowMeshDraw`，集中处理 shadow pass 共享的 postprocess-pass 判断和 mesh / instanced mesh draw。
- 新增 `ShadowRenderStats`，记录 directional layer/draw call、point light/face/draw call。
- `ShadowRenderer` 现在只作为 facade 调度 directional / point pass，并返回合并后的 shadow stats。
- `RendererFrameStats`、Debug UI 和 `--verify-pbr` 输出已接入 shadow stats，后续可以直接观察 shadow pass 是否覆盖当前 PBR 场景。

这一步为后续 `PBRShadowAtlas` 做准备：directional shadow、point shadow 和 mesh depth draw 已经不再绑定在一个大 `ShadowRenderer.cpp` 里，后续可以逐步替换某个 shadow pass 的资源布局，而不必重写整个 renderer frame flow。

### 2026-05-20 Light Resource Binder

Forward lighting uniform 绑定已从 `MaterialBinder` 中拆出：

- 新增 `LightResourceBinder`，集中写入 spot light、directional light、point light array、`POINT_LIGHT_NUM` 和 ambient color uniforms。
- `MaterialBinder` 不再持有灯光 uniform 绑定细节，Phong / PBR / shadow material 都通过 `LightResourceBinder::bindForwardLights(...)` 复用同一入口。
- `MaterialBinder` 对 `PointLightShadow` 的直接依赖已移除，point light 数量常量由 `LightResourceBinder` 负责。
- VS 工程已加入 `LightResourceBinder.cpp/.h`。

这一步为 PBR lighting path 后续演进做准备：将来如果要把 forward light uniforms 替换为 UBO、SSBO、clustered light list 或 PBR 专用 light binder，改动点可以集中在 `LightResourceBinder`，而不是在每个材质绑定函数里重复修改。

### 2026-05-20 PBR Material Binder

PBR 材质绑定已从通用 `MaterialBinder` 中拆出：

- 新增 `PBRMaterialBinder`，集中处理 PBR surface uniforms、PBR texture slots、IBL uniforms、IBL texture binding、forward lighting 和 CSM shadow resource binding。
- `MaterialBinder` 的 `PBRMaterial` 分支现在只负责转发到 `PBRMaterialBinder::bind(...)`，不再直接维护 PBR shader uniform 细节。
- `PBRMaterialBinder` 仍复用 `LightResourceBinder` 和 `ShadowResourceBinder`，因此灯光、shadow、IBL 和材质参数的职责边界更清晰。
- VS 工程已加入 `PBRMaterialBinder.cpp/.h`。

这一步为后续 PBR 专用 shader binding path 做准备。后续如果要让 PBR 使用独立 light buffer、IBL debug view、shadow atlas 或不同 BRDF 参数布局，可以优先修改 `PBRMaterialBinder`，而不是继续扩大通用 `MaterialBinder` 的 switch 分支。

### 2026-05-20 Renderer Mesh Draw Helper

底层 mesh draw 入口已从各 render pass 中收敛出来：

- 新增 `MeshDraw::drawIndexed(...)`，统一处理空 mesh / 空 geometry 检查、VAO 绑定、普通 indexed draw 和 instanced indexed draw。
- `SceneRenderPass`、`PBRSceneRenderPass`、`PBRDepthPrepass` 删除各自重复的 `drawMesh(...)` 实现。
- `ShadowMeshDraw` 继续保留 shadow 专用的 postprocess-pass 判断，但实际 draw 动作转发到 `MeshDraw`。
- `DirectionalShadowRenderPass` 和 `PointShadowRenderPass` 现在只在 draw 成功后增加 draw call 统计。
- `IBLPrecomputePass` 的 cubemap capture 与 BRDF LUT draw 也复用 `MeshDraw`。

这一步的目标是让后续 PBR shadow atlas、IBL debug pass、G-buffer 或 clustered depth pass 不再复制 VAO / instancing 绘制细节。pass 只负责绑定资源、shader 和 per-object uniform；mesh draw 行为由 renderer 层统一入口负责。

### 2026-05-20 Material Binding Context

材质绑定参数组已从长参数列表收敛为上下文对象：

- 新增 `MaterialBindingContext`，集中承载当前 frame 的 camera、directional light、spot light、point light list、ambient light 和 environment targets。
- `SceneRenderPass` 与 `PBRSceneRenderPass` 不再逐项接收并转发 camera / lights / environment。
- `MaterialBinder::bind(...)` 与 `PBRMaterialBinder::bind(...)` 的外部接口改为接收 `MaterialBindingContext`。
- `RendererFramePassRegistry` 在执行 scene pass 时由 `RendererFrameContext` 构造 binding context，再传给具体 pass。

这一步为后续 PBR 专用资源布局做准备：如果要把 PBR light buffer、IBL debug resources、shadow atlas 或 material debug flags 接入绑定阶段，可以扩展 `MaterialBindingContext` 或派生新的 PBR binding context，而不是继续扩大每个 pass / binder 的参数列表。

### 2026-05-20 Depth Prepass Binder

PBR depth prepass 的 shader binding 已从 pass 主流程中拆出：

- 新增 `DepthPrepassBinder`，集中写入 depth shader 的 `viewMatrix`、`projectionMatrix`、`near`、`far` 和 per-object `modelMatrix`。
- `PBRDepthPrepass::render(...)` 现在接收 `MaterialBindingContext`，与 scene / material binding 使用同一套 frame 输入。
- `RendererFramePassRegistry` 的 PBR depth prepass 和 scene pass 都通过 `createMaterialBindingContext(...)` 构造绑定上下文。

这一步把 depth-only binding 与 pass orchestration 分开。后续如果要增加 PBR G-buffer depth pass、shadow atlas depth variant 或 depth debug view，可以优先复用 / 扩展 `DepthPrepassBinder`，而不是继续把 camera 和 shader uniform 写入散落在具体 pass 中。

### 2026-05-20 PBR IBL Resource Binder

PBR IBL resource binding 已从 `PBRMaterialBinder` 中拆出：

- 新增 `PBRIBLResourceBinder`，集中处理 `useIBL` 判断、IBL strength uniforms、`iblMaxReflectionLod` 和 irradiance / prefilter / BRDF LUT texture binding。
- `PBRMaterialBinder` 不再直接依赖 `EnvironmentRenderTargets` 的 IBL 资源细节，只在材质绑定过程中调用 `PBRIBLResourceBinder::bind(...)`。
- `PBRIBLResourceBinder::canUseIBL(...)` 作为独立入口保留，后续 Debug UI、IBL debug pass 或 verification stats 可以复用同一套判断条件。

这一步为 IBL debug view 和 PBR environment resource layout 替换做准备。后续如果要增加 irradiance / prefilter 可视化，或把 IBL 资源改为 bindless / descriptor-like 布局，改动点可以集中在 `PBRIBLResourceBinder`。

### 2026-05-21 PBR Surface Resource Binder

PBR surface resource binding 已从 `PBRMaterialBinder` 中拆出：

- 新增 `PBRSurfaceResourceBinder`，集中绑定 PBR surface vec3 uniforms、surface float uniforms 和 6 组可选 texture slots。
- `PBRMaterialBinder` 不再直接遍历 `PBRMaterial` 的 texture slot / surface uniform schema，只负责组合 object matrices、lights、shadow、surface 和 IBL binder。
- PBR surface schema 仍来自 `PBRMaterial`，Debug UI、profile 和 shader binding 继续共享同一组 slot 描述。

这一步为后续 BRDF 参数布局调整和材质贴图槽扩展做准备。新增 clearcoat、anisotropy、transmission 等 PBR 参数时，主要改动点应集中在 `PBRMaterial` schema 与 `PBRSurfaceResourceBinder`。

### 2026-05-21 PBR Shadow Resource Binder

PBR shadow resource binding 已从 `PBRMaterialBinder` 中拆出：

- 新增 `PBRShadowResourceBinder`，作为 PBR 材质侧 shadow resource binding 的唯一入口。
- 当前实现继续复用已有 CSM shadow resources，但固定 PBR shadow texture unit `8` 不再散落在 `PBRMaterialBinder` 中。
- 当当前 frame 没有可用 directional shadow 时，binder 会写入 `csmLayerCount = 0`，让 PBR shader 走无 shadow 分支。
- `PBRMaterialBinder` 现在只组合 common uniforms、light binder、PBR shadow binder、surface binder 和 IBL binder。

这一步为 `PBRShadowAtlas` 做准备。后续如果替换 shadow atlas、增加 PBR 专用 cascade 布局或新增 shadow debug view，优先修改 `PBRShadowResourceBinder`，而不是改动 PBR 材质主绑定流程。

### 2026-05-21 PBR Object Uniform Binder

PBR object-level uniform binding 已从 `PBRMaterialBinder` 中拆出：

- 新增 `PBRObjectUniformBinder`，集中写入 `opacity`、`time`、`speed`、`cameraPosition`、MVP matrices 和 `normalMatrix`。
- `PBRMaterialBinder` 现在只负责组合 `PBRObjectUniformBinder`、`LightResourceBinder`、`PBRShadowResourceBinder`、`PBRSurfaceResourceBinder` 和 `PBRIBLResourceBinder`。
- 这让 PBR forward binding 的各类资源边界基本成型：object uniforms、lights、shadow、surface、IBL 均有独立入口。

这一步为后续 G-buffer、PBR debug view 和 shader layout 变体做准备。不同 PBR pass 可以复用 object uniform binder，也可以替换为 G-buffer 专用 object binder，而不用修改主材质绑定编排。

### 2026-05-21 PBR Scene Pass Direct Binder

PBR scene pass 已绕过通用材质 switch：

- `PBRSceneRenderPass` 在确认 mesh 使用 `PBRMaterial` 后，直接调用 `PBRMaterialBinder::bind(...)`。
- 默认 PBR forward path 不再经过 `MaterialBinder::bind(...)` 的历史材质 switch。
- `MaterialBinder` 仍保留 PBR 分支，用于 global material override 或 legacy scene pass 的兼容路径。

这一步进一步明确 PBR forward pass 的 ownership：PBR pass 负责 PBR material binding，legacy scene pass 负责通用 / 历史材质绑定。后续添加 G-buffer 或 PBR debug pass 时，可以直接复用 PBR 专用 binder，而不是通过通用材质 switch 间接调用。

### 2026-05-21 Renderer Frame Pass Plan Builder

Renderer 内部 pass plan 已从“直接选择固定 vector”推进到“按稳定 pass key 解析 order string”：

- `RendererFramePassDefinition` 新增 `keyName`，每个 renderer pass 现在都有稳定配置 key 和调试名。
- `RendererFramePassRegistry` 新增 `defaultPassOrder()` 与 `globalMaterialOverridePassOrder()`，当前默认顺序保持为 `BeginFrame,ShadowMaps,PBRDepthPrepass,LegacyOpaqueScene,PBROpaqueScene,LegacyTransparentScene,PBRTransparentScene`。
- `RendererFramePassRegistry::buildPassPlan(...)` 会解析逗号分隔 pass order、忽略未知 token、去重重复 pass；如果没有有效 pass，则回退默认 renderer pass plan。
- `Renderer::render(...)` 现在通过 order string 构建 pass plan，再执行每个 pass；默认路径和 global material override 路径的行为保持不变。

这一步的目的不是把 renderer pass 立刻暴露成用户配置，而是先把内部扩展机制打通。后续新增 `PBRShadowAtlas`、`GBuffer`、`IBLDebug` 或 clustered lighting pass 时，可以按 key 注册并插入 order，而不是继续修改 `Renderer::render()` 主流程。

### 2026-05-21 Renderer Frame Pass Profile

Renderer 内部 pass plan 已接入 profile 化配置：

- 新增 `RendererFramePassProfile`，保存默认 renderer pass order 和 global material override pass order。
- 新增 `RendererFramePassProfileStorage`，通过 `PropertySchema` / `ProfileConfigIO` 读写 `config/renderer_frame_pass.local.ini`。
- 新增 `config/renderer_frame_pass.example.ini`，记录当前默认 renderer pass key 顺序。
- `Renderer` 现在持有 frame pass profile，`Renderer::render(...)` 会从 profile 读取 pass order，再交给 `RendererFramePassRegistry::buildPassPlan(...)` 构建执行计划。
- `RuntimeProfileLoader` 启动时会加载 renderer frame pass profile；没有 local 配置时保持内建默认顺序。
- `DebugControllerPanel` 新增 Renderer Frame Pass Plan 面板，可编辑、保存、重载和恢复内建默认 renderer pass order。
- `--verify-pbr` 会强制重置 renderer pass profile 为内建默认值，避免本地实验配置影响 PBR 验证证据。

这一步把上一轮的 renderer pass builder 从“内部工具函数”推进为“可实验的渲染路径配置”。后续新增 `PBRShadowAtlas`、`GBuffer`、`IBLDebug` 或 clustered lighting pass 后，可以先通过 local profile 组合验证不同 pass 顺序，再决定是否固化为默认路径。

### 2026-05-21 IBL Debug Renderer Pass

Renderer 已新增第一个可选 PBR / IBL 诊断 pass：

- 新增 `IBLDebugPass`，在当前 scene color target 中绘制 IBL debug fullscreen view。
- 新增 `shaders/diagnostics/ibl_debug.*`，支持显示 environment cubemap、irradiance cubemap、prefilter cubemap 和 BRDF LUT。
- `ShaderLibrary` 新增 IBL debug shader 管理入口。
- `RendererFramePassRegistry` 新增可选 pass key `IBLDebug`。默认 pass order 不包含它，因此默认渲染行为不变；本地 profile 可以把 `IBLDebug` 加到 `defaultPassOrder` 末尾以显示 IBL debug view。
- `RendererFramePassProfile` 新增 `iblDebugMode`、`iblDebugMipLevel`、`iblDebugIntensity`，Debug UI 和 `config/renderer_frame_pass.example.ini` 均可配置。
- `RendererFrameStats` 新增 `iblDebugDrawCalls`，Debug UI 可直接观察 IBL debug pass 是否实际执行。
- 新增 `--verify-pbr-ibl-debug` 验证入口，使用 PBR verification scene 但临时插入 `IBLDebug` pass，导出 `out/pbr_ibl_debug_verification.ppm`。

这一步不是最终 IBL 可视化工具，而是先把“PBR 环境资源可诊断”接入 renderer pass 系统。后续如果 BRDF、prefilter mip、irradiance 或 environment 资源出现问题，可以通过 pass profile 直接切换可视化对象，而不是靠猜测 shader binding 是否正确。

### 2026-05-21 PBR GBuffer Renderer Pass

Renderer 已新增可选 PBR G-buffer pass：

- 新增 `PBRGBufferRenderTargets`，集中管理 G-buffer FBO、position/roughness、normal/metallic、albedo/AO 三张 `RGBA16F` color attachment 和 depth attachment。
- 新增 `PBRGBufferPass`，复用 `PBRObjectUniformBinder` 与 `PBRSurfaceResourceBinder`，只处理 PBR opaque mesh 的 geometry/material surface 写入。
- 新增 `shaders/pbr/pbr_gbuffer.*`，把 world position、normal、albedo、metallic、roughness、AO 写入 G-buffer attachments。
- `RendererFramePassRegistry` 新增可选 pass key `PBRGBuffer`；默认 pass order 不包含它，因此默认 forward PBR 行为不变。
- `RuntimePBRVerification` 新增 `--verify-pbr-gbuffer`，验证模式会临时把 pass order 调整为 `BeginFrame,ShadowMaps,PBRDepthPrepass,PBRGBuffer,LegacyOpaqueScene,PBROpaqueScene,LegacyTransparentScene,PBRTransparentScene`。
- `RendererFrameStats` 和 Debug UI 新增 G-buffer ready、size、draw calls，可直接确认 pass 是否真的执行。

这一步不是要立刻切换到 deferred PBR，而是先把 PBR 几何缓冲生产点接入现有 renderer pass 系统。后续 deferred lighting、G-buffer debug view、clustered lighting 或 SSR/TAA 都可以消费 `PBRGBufferRenderTargets`，而不是重新从 forward pass 里拆数据。

### 2026-05-21 PBR GBuffer Debug Consumer

Renderer 已新增第一个 G-buffer consumer pass：

- 新增 `PBRGBufferDebugPass`，从 `PBRGBufferRenderTargets` 读取 position/roughness、normal/metallic、albedo/AO 和 depth texture，并绘制 fullscreen debug view。
- 新增 `shaders/diagnostics/pbr_gbuffer_debug.*`，支持 `Albedo`、`Normal`、`Roughness`、`Metallic`、`AO`、`Depth`、`World Position` 多种 G-buffer debug mode。
- `RendererFramePassRegistry` 新增可选 pass key `PBRGBufferDebug`；它不生产 G-buffer，因此需要和 `PBRGBuffer` 同时插入 pass order。
- `RendererFramePassProfile` 新增 `pbrGBufferDebugMode` 与 `pbrGBufferDebugIntensity`，Debug UI 和 renderer pass profile ini 共用同一套字段。
- `RuntimePBRVerification` 新增 `--verify-pbr-gbuffer-debug`，验证模式会插入 `PBRGBuffer` 与 `PBRGBufferDebug`，并导出 `out/pbr_gbuffer_debug_verification.ppm`。
- PBR verification scene 的 material grid 从视锥下方移动到相机视野中心，避免只统计 `pbrDrawCalls=25` 但最终 capture 仍主要是 legacy/Phong 场景的弱验证。
- `shaders/pbr/pbr.vert` 与 `shaders/pbr/pbr_gbuffer.vert` 固定 PBR attribute layout，使 PBR forward pass 与 G-buffer pass 共享同一套 VAO attribute convention。

这一步把 G-buffer 从“只写不读”的准备状态推进到“可诊断、可验证”的状态。后续如果要做 deferred lighting pass，可以直接复用同一个 `PBRGBufferRenderTargets` 输入；如果 G-buffer 某个 channel 异常，也可以先用 `PBRGBufferDebug` 定位，而不是猜测 pass 是否真的写入了 attachment。

### 2026-05-21 PBR Deferred Lighting Consumer

Renderer 已新增最小 deferred PBR lighting consumer：

- 新增 `PBRDeferredLightingPass`，从 `PBRGBufferRenderTargets` 读取 G-buffer attachments，并使用当前 camera、ambient、directional light、point lights 和 IBL resources 做 fullscreen lighting。
- 新增 `shaders/pbr/pbr_deferred_lighting.*`，复用 forward PBR 的 GGX / Smith / Schlick BRDF 计算方式，当前实现先覆盖最小 opaque PBR lighting，不处理透明、emissive map 和 shadow。
- `RendererFramePassRegistry` 新增可选 pass key `PBRDeferredLighting`；该 pass 依赖 `PBRGBuffer`，验证模式使用 `BeginFrame,ShadowMaps,PBRDepthPrepass,PBRGBuffer,PBRDeferredLighting`，不经过 forward PBR scene pass。
- `RendererFramePassProfile` 新增 `pbrDeferredLightingIntensity`、`pbrDeferredIblDiffuseStrength`、`pbrDeferredIblSpecularStrength`，Debug UI 和 renderer pass profile ini 可调整 deferred lighting 强度。
- `RendererFrameStats` 新增 `pbrDeferredLightingDrawCalls`，Debug UI 和 verification stats 可确认 deferred lighting pass 是否实际执行。
- `RuntimePBRVerification` 新增 `--verify-pbr-deferred`，导出 `out/pbr_deferred_verification.ppm`，用于证明 G-buffer 可以进入实际 lighting consumer，而不只是进入 debug view。

这一步不是最终 deferred PBR renderer，而是把关键的 producer-consumer 闭环打通：PBR opaque mesh 可以进入 G-buffer，再由单独 lighting pass 读取并输出最终 HDR scene color。后续要补的主要是 shadow atlas / shadow sampling、透明 forward fallback、material feature parity，以及把 light data 从逐 uniform 绑定演进为 UBO / SSBO / clustered light list。

### 2026-05-21 Shared PBR Lighting Shader Include

Forward PBR 与 deferred PBR 已共享同一套 BRDF / IBL shader 函数：

- 新增 `shaders/pbr/pbr_lighting.glsl`，集中保存 GGX distribution、Smith geometry、Schlick Fresnel、direct PBR lighting 和 IBL ambient 计算。
- `shaders/pbr/pbr.frag` 删除重复 BRDF / IBL 函数，改为在声明 PBR / IBL uniforms 后 `#include "pbr_lighting.glsl"`。
- `shaders/pbr/pbr_deferred_lighting.frag` 删除同一套重复函数，也改为 include 共享实现。
- VS 工程已加入 `pbr_lighting.glsl`，保证物理文件与工程 filter 可见结构一致。

这一步降低 forward / deferred PBR 之间的 shader 分叉风险。后续修改 BRDF、IBL energy compensation、Fresnel、multi-scattering 或 tone-space 前置处理时，优先改共享 include，而不是分别改 forward shader 和 deferred shader。

### 2026-05-21 Shared PBR CSM Shadow Include

Deferred PBR lighting 已接入现有 CSM shadow sampling：

- 新增 `shaders/pbr/pbr_csm_shadow.glsl`，集中保存 CSM layer selection、bias 计算和 3x3 PCF shadow sampling。
- `shaders/pbr/pbr.frag` 删除原本内嵌的 CSM helper，forward PBR 改为 include 共享 CSM shadow 实现。
- `shaders/pbr/pbr_deferred_lighting.frag` include 同一份 CSM shadow 实现，并用 `calculateCsmShadow(...)` 调制 directional light direct lighting。
- `PBRDeferredLightingPass` 现在写入 `viewMatrix`，并复用 `PBRShadowResourceBinder` 绑定已有 directional CSM shadow map、cascade layers 和 light matrices。
- `PBRDeferredLightingPass` 返回 shadow binding stats；`RendererFrameStats`、Debug UI 和 `--verify-pbr-deferred` 会输出 deferred CSM shadow 是否绑定以及 layer 数。

这一步仍不是最终 shadow atlas 方案，但 deferred PBR 不再只是无阴影 lighting consumer。当前策略是先复用现有 CSM shadow 资源，保持 renderer pass 架构继续前进；后续如果要做 PBR shadow atlas，可以把资源布局替换到新的 pass / binder 中，而不需要再改 deferred lighting 的主流程边界。

### 2026-05-21 PBR Deferred Light Buffer

Deferred PBR lighting 的 light data 已从逐 uniform 写入推进到 SSBO：

- 新增 `PBRDeferredLightBuffer`，把 directional light、ambient light 和最多 16 个 point lights 打包到 `GL_SHADER_STORAGE_BUFFER`。
- `shaders/pbr/pbr_deferred_lighting.frag` 删除 `DirectionalLight` / `PointLight` uniform 数组，改为通过 `layout(std430, binding = 3)` 读取 deferred light buffer。
- `PBRDeferredLightingPass` 不再逐字段写 point light uniforms，而是每帧绑定一次 light buffer；当前仍保留 camera、IBL、shadow 等非 light-list uniforms。
- `RendererFrameStats`、Debug UI 和 `--verify-pbr-deferred` 会输出 deferred light buffer 是否绑定以及 point light count / capacity。

这一步不是 clustered lighting 的最终实现，但它把 deferred path 从“少量 uniform light list”推进到“buffer-backed light data”。后续可以在同一边界上继续扩展为 SSBO light list、tile/cluster index list 或 GPU culling，而不需要再改 fullscreen deferred lighting pass 的基础数据入口。

### 2026-05-21 PBR Shadow Atlas Resource Layout

Renderer 已新增 PBR shadow atlas 的资源布局层：

- 新增 `PBRShadowAtlasRenderTargets`，集中管理后续 PBR shadow atlas pass 会使用的 FBO、directional CSM depth texture array 和 point shadow depth texture array。
- 当前 directional atlas 按现有 `DirectionalLightCSMShadow` 的 cascade layer 数创建 depth array，point atlas 按当前有效 point light 数创建 `pointLightCount * 6` 个 face layers。
- `RendererFrameContext` 和 `Renderer` 已持有 `PBRShadowAtlasRenderTargets`，`ShadowMaps` pass 会 prepare atlas resources 并写入 `RendererFrameStats`。
- Debug UI 与 `--verify-pbr*` 输出新增 `pbrShadowAtlasReady`、directional layer count 和 point face count。

这一步暂时不替换现有 shadow 渲染和采样资源，目的是先把 PBR shadow atlas 的所有权、生命周期和可观测状态接进 renderer。后续可以把 directional / point shadow pass 逐步改为写入这组 atlas targets，再切换 PBR forward / deferred shader sampling。

### 2026-05-21 PBR Shadow Atlas Render Pass

PBR shadow atlas 已从“只创建资源”推进到“实际写入”：

- 新增 `PBRShadowAtlasRenderPass`，复用现有 shadow shaders 和 `ShadowMeshDraw`，把当前 shadow casters 额外渲染到 `PBRShadowAtlasRenderTargets`。
- directional atlas 会按当前 CSM layer 写入 directional depth texture array；point atlas 会按 point light cubemap faces 写入 point depth texture array。
- `ShadowMaps` pass 目前仍先生成旧 shadow maps，再额外写入 PBR atlas，因此现有 PBR forward / deferred shadow sampling 行为不变。
- `RendererFrameStats`、Debug UI 和 `--verify-pbr*` 会输出 atlas directional / point draw calls 与 point faces rendered。

这一步让 shadow atlas 具备可验证的生产链路，但还没有切换 shader sampling。下一步可以把 `PBRShadowResourceBinder` 从旧 CSM texture 迁移到 atlas resources，或者先把 atlas pass 改成可选 profile pass 以控制额外 shadow 渲染成本。

### 2026-05-21 PBR Shadow Atlas Sampling Binding

PBR directional CSM shadow sampling 已从“只写 atlas”推进到“优先采样 atlas”：

- `MaterialBindingContext` 新增 `pbrShadowAtlasTargets`，让 PBR material / deferred pass 的资源绑定层可以看到 renderer 持有的 PBR atlas targets。
- `PBRShadowResourceBinder::bindDetailed(...)` 现在会优先检查 `PBRShadowAtlasRenderTargets` 的 directional depth texture array；atlas ready 时绑定 atlas texture，atlas 不可用时 fallback 到旧 `ShadowResourceBinder::bindCSMShadowResources(...)`。
- atlas 采样仍复用 `pbr_csm_shadow.glsl` 的 CSM layer selection、light matrices、bias 和 PCF 逻辑；这一步只替换 directional depth texture 的来源，不改变 shader 侧 shadow math。
- `PBRDeferredLightingPass` 新增 atlas shadow binding stats，`RendererFrameStats`、Debug UI 和 `--verify-pbr-deferred` 会输出 `pbrDeferredCsmShadowAtlasBound`。
- Forward PBR 与 deferred PBR 共享 `PBRShadowResourceBinder`，因此默认 forward PBR pass 在 `ShadowMaps` pass 已写入 atlas 后也会优先绑定 atlas directional CSM texture。

这一步完成了 PBR directional shadow atlas 的 producer-consumer 闭环。当前仍未处理的是 point shadow atlas sampling：point atlas 已写入，但 PBR shading 还没有从 atlas 中读取 point light shadow faces。

### 2026-05-21 PBR Shadow Atlas Pass Key

PBR shadow atlas 写入已从 `ShadowMaps` pass 的内部副作用拆成独立 renderer pass：

- `RendererFramePassKey` 新增 `PBRShadowAtlas`，默认 pass order 变为 `BeginFrame,ShadowMaps,PBRShadowAtlas,PBRDepthPrepass,LegacyOpaqueScene,PBROpaqueScene,LegacyTransparentScene,PBRTransparentScene`。
- `ShadowMaps` pass 现在只负责 legacy directional / point shadow maps；`PBRShadowAtlas` pass 单独负责写入 PBR directional CSM depth array 和 point shadow depth array。
- `config/renderer_frame_pass.example.ini` 和 Debug UI 的 pass key 列表已包含 `PBRShadowAtlas`。本地 profile 可以删除该 key 来跳过额外 atlas 渲染成本，PBR shadow binder 会自动 fallback 到 legacy CSM resources。
- `PBRShadowAtlasRenderTargets::resetFrameStats()` 会在每帧 render 开始清空 atlas ready 状态，避免禁用 `PBRShadowAtlas` pass 后继续采样上一帧 atlas。
- `RuntimePBRVerification` 新增 `--verify-pbr-no-atlas` 与 `--verify-pbr-deferred-no-atlas`，用于验证禁用 atlas pass 后 `pbrShadowAtlasReady=no` 且 deferred PBR 走 legacy CSM fallback。

这一步把 PBR shadow atlas 从“固定跟随 ShadowMaps 执行”推进为“profile 可组合的 PBR pass”。后续可以继续把 point shadow atlas sampling 或 shadow debug view 按同样方式接入，而不用扩大 `ShadowMaps` pass 的职责。

### 2026-05-21 PBR Point Shadow Atlas Sampling

PBR point light shadow atlas 已从“只写入”推进到“forward / deferred PBR shader 可采样”：

- 新增 `shaders/pbr/pbr_point_shadow.glsl`，集中保存 point shadow atlas 的 cube-face selection、2D array layer mapping 和 3x3 PCF sampling。
- `PBRShadowResourceBinder` 现在会在 atlas ready 且 point atlas ready 时绑定 `pbrPointShadowMapSampler`，并写入每个 point light 对应的 layer base、near/far、bias、PCF radius 和 enabled flag。
- `shaders/pbr/pbr.frag` 与 `shaders/pbr/pbr_deferred_lighting.frag` include 同一份 point shadow helper，并用 `calculatePbrPointShadow(...)` 调制 point light direct lighting。
- `PBRDeferredLightingPassStats`、`RendererFrameStats`、Debug UI 与 verification 输出新增 `pbrDeferredPointShadowAtlasBound` 和 `pbrDeferredPointShadowAtlasLights`，用于确认 deferred PBR 是否实际采样 point atlas。
- `--verify-pbr-deferred-no-atlas` 继续验证禁用 `PBRShadowAtlas` pass 时 point shadow atlas 不会绑定，shader 会回到无 point atlas shadow 的 fallback 行为。

这一步完成了 PBR shadow atlas 的主要 producer-consumer 闭环：directional CSM 与 point light depth array 都已经由 PBR atlas pass 生产，并被 forward / deferred PBR shader 采样。后续重点转向透明 forward fallback、material feature parity，以及 clustered/tiled light list。

### 2026-05-21 PBR Deferred Transparent Forward Fallback

Deferred PBR 已新增透明 forward fallback 的验证路径：

- 新增 `--verify-pbr-deferred-transparent`，在 PBR verification scene 中追加一个透明 PBR probe，并把 renderer pass order 设置为 `PBRDeferredLighting` 后继续执行 `LegacyTransparentScene,PBRTransparentScene`。
- `RuntimePBRVerification::addVerificationSceneProbes(...)` 负责按验证配置追加透明 PBR probe；普通 `--verify-pbr`、`--verify-pbr-deferred` 不会被该 probe 污染。
- `RendererFrameStats` 新增 `legacyTransparentDrawCalls` 与 `pbrTransparentDrawCalls`，Debug UI 和 verification 输出可以直接确认透明 fallback 是否实际执行。
- 透明 probe 使用 PBRMaterial、blend enabled、depth write disabled、opacity 0.45，仍走 `PBRSceneRenderPass` / `PBRMaterialBinder`，因此会复用 PBR IBL、directional / point shadow atlas binding。

这一步没有引入新的透明渲染算法，目标是先把 deferred opaque + forward transparent 的组合路径变成可验证状态。后续如果要支持 OIT、weighted blended transparency 或独立 transparent lighting 策略，可以在这个 fallback 边界上继续替换。

### 2026-05-21 PBR Deferred Emissive Material Parity

Deferred PBR 已补齐第一块 material feature parity：emissive。

- `PBRGBufferRenderTargets` 新增 emissive color attachment，G-buffer 从 position/normal/albedo 三张 color attachment 扩展为四张 color attachment。
- `shaders/pbr/pbr_gbuffer.frag` 写入 `pbrEmissiveColor * pbrEmissiveIntensity` 与 emissive map contribution；`PBRSurfaceResourceBinder` 已经统一绑定 emissive uniforms / texture slots，因此无需为 G-buffer 增加独立材质绑定分支。
- `shaders/pbr/pbr_deferred_lighting.frag` 新增 `emissiveTexture` 输入，并在 lighting 输出中叠加 emissive。空像素判断改为允许 emissive-only material，不再因为 albedo 为 0 而丢弃 emissive probe。
- `PBRGBufferDebugPass` 和 `shaders/diagnostics/pbr_gbuffer_debug.frag` 新增 debug mode `7 = Emissive`，便于直接检查 G-buffer emissive attachment。
- 新增 `--verify-pbr-deferred-emissive`，在 verification scene 中追加一个 opaque emissive PBR probe，用于验证 emissive attachment 能进入 deferred lighting 输出。

这一步让 forward PBR 已有的 emissive color / map / intensity 数据进入 deferred path。后续 material parity 还需要继续覆盖 opacity / alpha mask、clearcoat 或更多贴图通道，但 emissive 已经有独立 attachment、debug view 和 runtime verification。

### 2026-05-21 PBR Deferred Material IBL Params

Deferred PBR 已把 per-material IBL 参数从 forward path 补到 G-buffer / lighting path：

- `PBRGBufferRenderTargets` 新增 material params attachment，G-buffer 从四张 color attachment 扩展为五张 color attachment。
- `PBRGBufferPass` 会把 `PBRMaterial::mUseIBL`、`mIblDiffuseStrength`、`mIblSpecularStrength` 写入 `gMaterialParams`，避免 deferred lighting 只能使用 renderer profile 的全局 IBL 强度。
- `shaders/pbr/pbr_lighting.glsl` 新增 `calculateIblAmbientWithStrength(...)`，forward PBR 继续使用材质 uniform，deferred PBR 则可以按像素读取 G-buffer 中的 IBL 强度。
- `PBRDeferredLightingPass` 绑定 material params texture，deferred shader 在 environment ready 且 material 允许 IBL 时按像素使用 diffuse/specular strength。
- `PBRGBufferDebugPass` 新增 debug mode `8 = IBL Params`，用于直接检查 diffuse strength、specular strength 和 useIBL 标记。
- 新增 `--verify-pbr-deferred-material-ibl`，在 verification scene 中追加一个 opaque custom IBL PBR probe，并输出 `pbrCustomIblMeshes`。

这一步让 deferred PBR 不再把所有材质的 IBL 响应压平成同一个全局强度。后续如果继续补 material parity，应优先考虑 alpha mask / opacity policy、normal-map tangent 数据一致性，以及把更多 material flags 统一编码到 G-buffer material params 或 material id buffer。

### 2026-05-21 PBR Alpha Mask First Stage

PBR 材质已新增 alpha mask / cutoff 语义，并接入 forward 与 deferred G-buffer 主视图：

- `PBRMaterial` 与 `PBRMaterialProfile` 新增 `useAlphaMask` 和 `alphaCutoff`，可通过 Inspector / profile config 控制。
- `PBRSurfaceResourceBinder` 统一绑定 `useAlphaMask` 与 `alphaCutoff`，forward PBR 和 G-buffer pass 共享同一套材质参数来源。
- `shaders/pbr/pbr.frag` 与 `shaders/pbr/pbr_gbuffer.frag` 会读取 albedo map alpha，并在 alpha 小于 cutoff 时 `discard`。
- `PBRDepthPrepass` 会跳过 alpha-masked PBR mesh，避免 cutout 材质在主视图 G-buffer 前被整片预写深度。
- 新增 `--verify-pbr-deferred-alpha-mask`，使用 `Texture/window.png` 作为带 alpha 的 albedo map，验证 alpha mask probe 能进入 G-buffer 而不进入 PBR depth prepass。

这一步先保证主视图的 forward / deferred PBR 不把 cutout 材质当整片实心面渲染。当前尚未完成的是 alpha-aware shadow：legacy shadow map 和 PBR shadow atlas 仍使用 depth-only shader，后续需要给 shadow pass 增加 alpha mask material binding 或专用 PBR alpha shadow shader，才能得到正确 cutout shadow。

### 2026-05-21 PBR Alpha-Aware Shadow

PBR alpha mask 已从主视图扩展到 shadow map / PBR shadow atlas：

- 新增 `PBRAlphaShadowBinder`，集中判断 alpha-masked PBR mesh，并为 shadow shader 绑定 albedo map alpha 与 cutoff。
- 新增 `pbr_alpha_shadow` 与 `pbr_alpha_point_shadow` shader，分别用于 directional / point shadow 的 alpha discard。
- `DirectionalShadowRenderPass` 与 `PointShadowRenderPass` 会继续用原 depth-only shader 绘制普通 mesh；遇到 alpha-masked PBR mesh 时切换到 alpha-aware shadow shader。
- `PBRShadowAtlasRenderPass` 同样接入 alpha-aware directional / point atlas 写入，保证 atlas shadow 与 legacy shadow map 的 cutout 语义一致。
- `RendererFrameStats`、Debug UI 和 `--verify-pbr*` 输出新增 legacy shadow 与 PBR atlas 的 alpha-masked shadow draw call 统计。

这一步补上了 alpha mask first stage 的最大缺口：cutout 材质不再只在主视图镂空，而是也能用 alpha discard 写入 shadow depth。后续仍可优化为更少 shader switch 或更明确的 cutout queue，但当前行为已经具备可验证的 producer-consumer 链路。

### 2026-05-21 PBR Verification Script

PBR 验证链路已从“手动逐个命令运行”推进到“一键回归脚本”：

- 新增 `tools/verify_pbr.ps1`，默认会通过 VS DevCmd + MSBuild 构建 `Debug|x64`，再顺序运行所有 `--verify-pbr*` 模式。
- 脚本会把每个模式的 stdout / stderr 写到 `out/pbr_verify_<mode>.log`，并生成 `out/pbr_verification_summary.txt`。
- summary 会解析 PPM capture，输出尺寸、文件大小、非黑像素比例和 RGB 均值，用于快速判断 capture 是否非空、是否明显偏离预期。
- 脚本把进程退出码和 PPM 有效性作为失败条件；`Failed to open logfile.` 这类非致命 stderr 会被保留在日志里，但不会误判为验证失败。
- 支持 `-SkipBuild` 和 `-Modes`，可以只跑局部模式，例如 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -Modes deferred,deferred-alpha-mask`。

本轮已执行默认全量验证，`Debug|x64` 构建通过，11 个 PBR 验证模式全部通过：

- `forward`
- `forward-no-atlas`
- `ibl-debug`
- `gbuffer`
- `gbuffer-debug`
- `deferred`
- `deferred-no-atlas`
- `deferred-transparent`
- `deferred-emissive`
- `deferred-material-ibl`
- `deferred-alpha-mask`

这一步不改变渲染行为，但显著降低后续 PBR 重构的回归成本。下一步继续做 clustered / tiled light list 或 PBR asset import 前，应先保持这个脚本作为基本 gate，避免继续只靠人工观察窗口判断。

### 2026-05-21 Assimp PBR Material Import Mapping

模型导入链路已新增 PBR 材质映射入口，目标是让外部资产可以直接进入 PBR path，而不是只能靠手写 preview grid：

- 新增 `AssimpMaterialImporter`，把 Assimp material 到项目 material 的转换从 `AssimpLoader::processMesh(...)` 中拆出。
- `AssimpLoader::load(path, renderer)` 默认行为保持不变，仍生成 legacy `PhongMaterial`，避免破坏旧实验。
- 新增 `AssimpLoader::load(path, renderer, AssimpMaterialImportOptions)` 和 `AssimpLoader::loadPBR(path, renderer)`，可显式选择 `PBRMetallicRoughness` 导入模式。
- PBR 导入模式会映射 base color / diffuse color、metallic factor、roughness factor、opacity、emissive color / intensity，以及 base color、metallic、roughness、AO、normal、emissive 贴图。
- `PBRMaterial` 新增 metallic / roughness / AO 贴图通道字段，forward PBR 和 G-buffer shader 现在会按 channel 采样材质贴图；当 Assimp 把 metallic 和 roughness 指向同一张贴图时，导入器会按 glTF metallic-roughness 常见布局使用 `B=metallic`、`G=roughness`。
- 新增 `--verify-pbr-import`，验证场景会用 `AssimpLoader::loadPBR("fbx/test/test.fbx", ...)` 导入一个 PBR asset probe，并输出 `pbrImportedMeshes`。
- `tools/verify_pbr.ps1` 已把 `import` 纳入默认回归模式。

本轮已执行默认全量脚本，`Debug|x64` 构建通过，12 个 PBR 验证模式全部通过；新增 `import` 模式输出 `pbrImportedMeshes=1`、`pbrDrawCalls=26`、有效 capture `out/pbr_import_verification.ppm`。

这一步把 PBR 从“程序生成材质球可验证”推进到“Assimp 外部资产可显式导入为 PBRMaterial”。后续还需要继续补真实 glTF / FBX PBR 资产的视觉审阅，以及更完整的 material feature parity，例如 alpha mode、combined occlusion-roughness-metallic texture 的更精细策略、clearcoat / transmission 等扩展。

### 2026-05-21 PBR Deferred Tiled Light Grid First Stage

Deferred PBR point light shading 已从“每个 fragment 遍历全局 point light list”推进到“可选 tile-indexed light list”：

- 新增 `PBRDeferredTiledLightGrid`，在 CPU 侧根据当前 camera、viewport、point light attenuation radius 生成 screen-space tile light list。
- `PBRDeferredLightingPass` 现在会在 deferred lighting 前绑定 tiled grid SSBO，并继续保留全局 point light loop fallback。
- `shaders/pbr/pbr_deferred_lighting.frag` 新增 tile buffer / tile index buffer 两个 SSBO：
  - tile buffer 使用 binding `4`，保存每个 tile 的 offset / count。
  - index buffer 使用 binding `5`，保存 tile 对应的 point light index 列表。
- `RendererFramePassProfile` 新增 `pbrDeferredTiledLightsEnabled` 与 `pbrDeferredTileSize`，可通过 Debug UI / profile 控制 tiled path。
- `RendererFrameStats`、Debug UI 和 runtime verification 输出新增 tiled light grid stats，用于确认 deferred shader 是否实际使用 tile-indexed light list。

这一步是 tiled / clustered lighting 的第一阶段，不是最终性能版本。当前 CPU bounds 仍偏保守，验证场景中两个 point lights 会覆盖全部 `80x45` tiles，因此 `pbrDeferredTiledLightGridIndices=7200`。这个结果说明数据链路已经跑通，但后续还需要继续收紧 culling bounds，或进一步演进到 GPU compute / clustered culling。

本轮已执行 `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1`，`Debug|x64` 构建通过，12 个 PBR verification mode 全部通过；deferred 系列输出确认 `pbrDeferredTiledLightGridBound=yes`、`pbrDeferredTiledLightGridSize=80x45`、`pbrDeferredTiledLightGridTileSize=16`、`pbrDeferredTiledLightGridMaxTileLights=2`。

### 2026-05-21 PBR Deferred Tiled Light Culling Verification

Tiled light grid 已新增专用验证模式，用于证明 tiled grid 不只是绑定成功，而是真的能减少 deferred shader 的 point light index 遍历入口：

- 新增 `--verify-pbr-deferred-tiled-lights`，该模式使用 sparse / high-attenuation point light rig，让点光只覆盖部分 screen tiles。
- `tools/verify_pbr.ps1` 默认验证模式从 12 个扩展为 13 个，并把 `deferred-tiled-lights` 纳入默认回归。
- 验证脚本会对该模式做额外断言：
  - 必须输出 `pbrDeferredTiledLightGridBound=yes`。
  - 必须能解析 tile grid size、point light count 和 light index count。
  - `pbrDeferredTiledLightGridIndices` 必须大于 `0`。
  - `pbrDeferredTiledLightGridIndices` 必须小于 `tileColumns * tileRows * pointLightCount`，否则说明 tiled list 没有减少全局 point light loop。
- `PBRDeferredLightBuffer` 修正 point light intensity 打包，`deferredPointLightColorIntensity.a` 现在使用 runtime `PointLight::getIntensity()`，与 tiled radius 估算和 shader lighting 语义保持一致。

本轮 full verification 已通过，新增模式输出 `pbrDeferredTiledLightGridSize=80x45`、`pbrDeferredLightBufferPointLights=2/16`、`pbrDeferredTiledLightGridIndices=3311`。对比全局遍历上限 `80 * 45 * 2 = 7200`，该模式已经能验证 tiled culling 实际减少了 deferred point light index 数。

### 2026-05-21 PBR Deferred Tiled Light Heatmap Debug Pass

Tiled light grid 已新增可视诊断 pass，用于直接观察每个 screen tile 的 point light occupancy，而不是只依赖 verification stats：

- 新增 `PBRDeferredTiledLightDebugPass`，作为独立 renderer frame pass 接入 `RendererFramePassRegistry`，pass key 为 `PBRDeferredTiledLightDebug`。
- 新增 `shaders/diagnostics/pbr_deferred_tiled_light_debug.vert` 与 `pbr_deferred_tiled_light_debug.frag`，fullscreen debug shader 从 tiled light tile buffer SSBO binding `4` 读取每个 tile 的 light count，并输出 heatmap。
- `RendererFramePassProfile` 新增 `pbrDeferredTiledLightDebugMaxLights` 与 `pbrDeferredTiledLightDebugIntensity`，Debug UI / profile 可调整热力图归一化上限和显示强度。
- `RendererFrameStats`、Debug UI 和 runtime verification 输出新增 `pbrDeferredTiledLightDebugDrawCalls`，用于证明 debug pass 确实执行。
- 新增 `--verify-pbr-deferred-tiled-heatmap`，验证场景会走 `PBRDepthPrepass,PBRGBuffer,PBRDeferredTiledLightDebug`，用 heatmap capture 验证 tile list 可被独立 debug consumer 使用。
- `tools/verify_pbr.ps1` 默认 PBR 回归从 13 个模式扩展为 14 个模式，并新增 `-DiscardCaptures`，可以在低磁盘空间下解析 PPM 后立即删除 capture，只保留 summary / log。

本轮 full verification 已通过，新增 heatmap 模式输出 `pbrDeferredTiledLightDebugDrawCalls=1`、`pbrDeferredTiledLightGridSize=80x45`、`pbrDeferredTiledLightGridIndices=3311`、非黑比例 `100%`、RGB 均值约 `185.09 / 173.14 / 87.33`。这说明 tiled light grid 不仅能服务 deferred lighting，也能被独立诊断 pass 消费，后续可用它继续收紧 CPU bounds 或迁移 clustered / GPU culling。

### 2026-05-21 PBR Deferred Tiled Light Bounds Cleanup

Tiled light grid 的 screen-space bounds 估算已从“单侧采样 + 64px 最小半径”调整为更明确的投影采样策略：

- `PBRDeferredTiledLightGrid::calculateLightBounds(...)` 现在同时采样 `+right`、`-right`、`+up`、`-up` 四个 light radius offset，使用这些投影点相对 light center 的最大屏幕距离估算 tile 覆盖半径。
- 只有当所有 radius sample 都无法投影时，才回退到保守的 `64px` fallback；正常情况下最小半径降为 `2px`，避免小光源被无条件扩大为 64px 覆盖。
- 当前 sparse tiled verification 仍输出 `pbrDeferredTiledLightGridIndices=3311`，说明该验证场景的覆盖范围主要由真实投影半径决定，而不是旧 64px 下限造成；本轮改动主要是移除隐藏魔数对普通小光源的过度扩张风险。
- 新增 `tools/msbuild_no_link_debug.targets` 与 `tools/verify_pbr.ps1 -NoLinkDebugInfo`，用于在低磁盘空间下导入 `/DEBUG:NONE`，避免验证构建生成几十 MB 的 linker PDB。
- `tools/verify_pbr.ps1 -Modes` 现在支持逗号分隔输入，例如 `-Modes deferred-tiled-lights,deferred-tiled-heatmap`。

本轮验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`：构建通过，tiled culling 验证通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：14 个 PBR verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures -Modes deferred-tiled-lights,deferred-tiled-heatmap`：逗号分隔 `-Modes` 解析验证通过。

### 2026-05-21 PBR Deferred Tiled Light Occupancy Stats

Tiled light grid 诊断已从“index count + max per tile”扩展为“tile occupancy”：

- `PBRDeferredTiledLightGridStats` 新增 `occupiedTileCount` 与 `emptyTileCount`。
- `RendererFrameStats`、Debug UI 和 runtime verification 输出新增 `pbrDeferredTiledLightGridOccupiedTiles=<occupied>/<total>` 与 `pbrDeferredTiledLightGridEmptyTiles=<empty>`。
- `tools/verify_pbr.ps1` 的 tiled culling 断言新增 occupancy 校验：occupied 必须大于 `0`，且必须小于 tile 总数；reported tile count 必须与 `columns * rows` 一致。

本轮验证结果给出了更明确的 bounds 质量基线：

- 普通 deferred 场景：`pbrDeferredTiledLightGridOccupiedTiles=3600/3600`，说明默认两个点光仍覆盖全屏 tiles。
- sparse tiled 场景：`pbrDeferredTiledLightGridIndices=3311`、`pbrDeferredTiledLightGridOccupiedTiles=3119/3600`、`pbrDeferredTiledLightGridEmptyTiles=481`，说明专用 culling 场景不仅减少了 index loop，也确实存在空 tiles。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`：构建通过，focused tiled occupancy 断言通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：14 个 PBR verification mode 全部通过。

### 2026-05-21 PBR Deferred Tiled Light Circle Tile Clip

Tiled light grid 已从“screen-space circle 的 AABB 覆盖所有候选 tiles”推进到“AABB 内再做 tile rectangle 与 light circle 的交叠测试”：

- `ScreenBounds` 保留 light 的屏幕中心和投影半径。
- `tileIntersectsCircularBounds(...)` 会在写入 tile light list 前，计算 tile rect 到 light center 的最近点，并用 `radius + 1px` 的保守圆半径判断是否相交。
- 当 light center 无法投影、系统退回全屏保守 bounds 时，不启用 circle clip，避免近裁剪 / 背后光源场景漏光。

本轮 focused verification 中 sparse tiled 场景从上一轮的：

- `pbrDeferredTiledLightGridIndices=3311`
- `pbrDeferredTiledLightGridOccupiedTiles=3119/3600`
- `pbrDeferredTiledLightGridEmptyTiles=481`

下降为：

- `pbrDeferredTiledLightGridIndices=2890`
- `pbrDeferredTiledLightGridOccupiedTiles=2846/3600`
- `pbrDeferredTiledLightGridEmptyTiles=754`

这说明 circle tile clip 去掉了 AABB 四角的部分无效 tile entries，同时保持 `--verify-pbr-deferred-tiled-lights` 与 heatmap consumer 都通过。`powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures` 已再次通过 14 个 PBR verification mode。

### 2026-05-21 PBR Deferred Tiled Light Flat Index Builder

Tiled light grid 的 CPU builder 已从 `std::vector<std::vector<int>> tileLightLists` 改为 flat entry + prefix offset 构建：

- 第一阶段收集 `{ tileIndex, lightIndex }` entries，并同步统计每个 tile 的 light count。
- 第二阶段根据 per-tile count 生成 `tileOffsetCount` 的 prefix offset / count。
- 第三阶段用 tile write cursor 将 entries scatter 到连续 `lightIndices` buffer。
- GPU 侧 SSBO layout 不变：tile buffer 仍是 binding `4`，index buffer 仍是 binding `5`。

这一步不改变 tiled lighting 结果，目标是减少每帧 `tileCount` 个小 vector 的容器开销，让 CPU builder 的数据布局更接近后续 GPU compute / clustered culling 需要的 flat buffer 模型。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`：构建通过，focused tiled 输出保持 `pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridOccupiedTiles=2846/3600`、`pbrDeferredTiledLightGridEmptyTiles=754`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：14 个 PBR verification mode 全部通过。

### 2026-05-21 PBR Deferred Tiled Light Scratch Buffer Reuse

Tiled light grid 的 CPU builder 已进一步把每帧局部临时容器改为 `PBRDeferredTiledLightGrid` 成员 scratch buffers：

- `mTileLightCounts` 复用每个 tile 的 light count 存储。
- `mTileLightEntries` 复用 flat `{ tileIndex, lightIndex }` entry buffer。
- `mTileOffsetCount` 复用上传到 tile SSBO 的 offset / count 数据。
- `mLightIndices` 复用上传到 index SSBO 的连续 light index 数据。
- `mTileWriteOffsets` 复用 scatter 阶段的写入 cursor。

这一步仍不改变 GPU SSBO layout，也不改变 deferred shader / heatmap shader 的读取方式。目标是避免 `bind()` 每帧反复创建多组临时 `std::vector`，让 CPU builder 逐步靠近“长期持有 buffer、每帧 clear / assign / reserve 复用容量”的形态，为后续 clustered / GPU culling 继续减少 CPU 侧结构性开销。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`：构建通过，focused tiled 输出保持 `pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridOccupiedTiles=2846/3600`、`pbrDeferredTiledLightGridEmptyTiles=754`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：14 个 PBR verification mode 全部通过；`deferred-tiled-lights` 与 `deferred-tiled-heatmap` 均保持 `2890` indices、`2846/3600` occupied、`754` empty。

### 2026-05-21 PBR Deferred Tiled Light Culling Efficiency Stats

Tiled light grid 的诊断已从“实际 index 数和 occupancy”扩展为“相对全局 point-light loop 的节省量”：

- `PBRDeferredTiledLightGridStats` 新增 point light count、full index count 和 culled index count。
- `RendererFrameStats`、Debug UI 和 runtime verification 输出新增：
  - `pbrDeferredTiledLightGridPointLights`
  - `pbrDeferredTiledLightGridFullIndices`
  - `pbrDeferredTiledLightGridCulledIndices`
- Debug UI 现在会显示 skipped index count 和 percentage。
- `tools/verify_pbr.ps1` 的 tiled culling 断言现在会检查：
  - `fullIndices == tileColumns * tileRows * pointLightCount`
  - `culledIndices == fullIndices - actualIndices`
  - 专用 tiled culling 模式必须裁掉至少一个 index。

这一步不改变渲染路径本身，目标是建立后续 clustered / GPU culling 的收益基线。现在 verification 不只知道 tiled grid “比全局遍历少”，还会验证具体少了多少。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`：构建通过，focused tiled 输出 `pbrDeferredTiledLightGridFullIndices=7200`、`pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridCulledIndices=4310`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：14 个 PBR verification mode 全部通过；`deferred-tiled-lights` 与 `deferred-tiled-heatmap` 均确认 `7200 -> 2890`，裁掉 `4310` 个 tiled point-light index 入口。

### 2026-05-21 PBR Deferred Tiled Light Tile Size Verification

Tiled light grid 的 tile size 配置现在有专用 verification 覆盖：

- `RuntimePBRVerificationConfig` 新增 `pbrDeferredTileSizeOverride`，verification 可以显式覆盖 `RendererFramePassProfile::pbrDeferredTileSize`。
- 新增命令行模式 `--verify-pbr-deferred-tiled-lights-32`，复用 sparse tiled light probe，但把 tile size 从默认 `16px` 改为 `32px`。
- `tools/verify_pbr.ps1` 默认 PBR 回归从 14 个模式扩展为 15 个模式，新增 `deferred-tiled-lights-32`。
- `tools/verify_pbr.ps1` 对该模式新增 tile size 断言，确保 runtime stats 中的 `pbrDeferredTiledLightGridTileSize` 必须等于 `32`。

这一步验证的是 profile-driven tile size 不是 UI 上的虚设配置，而是真的影响 tiled grid 维度和 full index baseline。后续调试不同 tile size、评估 CPU tiled / clustered culling 策略时，可以直接用该模式做回归。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights-32`：构建通过，输出 `pbrDeferredTiledLightGridSize=40x23`、`pbrDeferredTiledLightGridTileSize=32`、`pbrDeferredTiledLightGridFullIndices=1840`、`pbrDeferredTiledLightGridIndices=762`、`pbrDeferredTiledLightGridCulledIndices=1078`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：15 个 PBR verification mode 全部通过。

### 2026-05-21 PBR Deferred Tiled Light Cutoff Profile

Tiled light grid 的 point light influence cutoff 已从硬编码常量提升为 renderer frame pass profile 参数：

- `RendererFramePassProfile` 新增 `pbrDeferredTiledLightCutoff`，默认值保持 `0.01`。
- `config/renderer_frame_pass.example.ini` 新增 `pbrDeferredTiledLightCutoff=0.01`，本地实验可以直接调整 cutoff。
- `PBRDeferredTiledLightGrid::bind(...)` 现在接收 cutoff，并用它估算 point light screen-space influence radius。
- `PBRDeferredLightingPass` 与 `PBRDeferredTiledLightDebugPass` 都从 profile 传入 cutoff，因此 deferred lighting 和 heatmap debug 使用同一套 light bounds。
- Debug UI 和 runtime verification 输出新增 `pbrDeferredTiledLightGridCutoff`。
- `RuntimePBRVerificationConfig` 新增 `pbrDeferredTiledLightCutoffOverride`。
- 新增 `--verify-pbr-deferred-tiled-lights-cutoff-005`，把 sparse tiled verification 的 cutoff 从 `0.01` 提高到 `0.05`。
- `tools/verify_pbr.ps1` 默认 PBR 回归从 15 个模式扩展为 16 个模式，并对 cutoff override 模式断言 `pbrDeferredTiledLightGridCutoff=0.05`。

这一步的意义是把 tiled light bounds 的关键质量参数纳入 profile / UI / ini / verification，而不是藏在 C++ 魔数里。后续比较 CPU tiled、clustered、GPU culling 时，可以明确说明 light influence cutoff 的取值。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights-cutoff-005`：构建通过，输出 `pbrDeferredTiledLightGridCutoff=0.050000`、`pbrDeferredTiledLightGridFullIndices=7200`、`pbrDeferredTiledLightGridIndices=714`、`pbrDeferredTiledLightGridCulledIndices=6486`、`pbrDeferredTiledLightGridOccupiedTiles=714/3600`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：16 个 PBR verification mode 全部通过。

### 2026-05-21 PBR Deferred Untiled Fallback Verification

`pbrDeferredTiledLightsEnabled` 现在有专用 verification 覆盖，确保 profile 关闭 tiled path 时 deferred lighting 会回退到全局 point-light loop：

- `RuntimePBRVerificationConfig` 新增 `disablePbrDeferredTiledLights`。
- 新增命令行模式 `--verify-pbr-deferred-untiled-lights`，复用 sparse tiled light probe，但强制设置 `RendererFramePassProfile::pbrDeferredTiledLightsEnabled=false`。
- `PBRDeferredLightingPass` 现在把本帧是否启用 tiled point-light path 写入 stats。
- `RendererFrameStats`、Debug UI 和 runtime verification 输出新增 `pbrDeferredTiledLightsEnabled`。
- `tools/verify_pbr.ps1` 默认 PBR 回归从 16 个模式扩展为 17 个模式，新增 `deferred-untiled-lights`。
- 脚本会断言 untiled fallback 模式下 deferred lighting 仍绘制、deferred light buffer 仍绑定并有 point lights、`pbrDeferredTiledLightsEnabled=no`、`pbrDeferredTiledLightGridBound=no`。

这一步让 tiled / untiled 两条 deferred point-light shading 路径都可回归。后续对比 tiled、clustered 或 GPU culling 时，可以明确验证 fallback 仍可用。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-untiled-lights`：构建通过，输出 `pbrDeferredLightingDrawCalls=1`、`pbrDeferredLightBufferBound=yes`、`pbrDeferredLightBufferPointLights=2/16`、`pbrDeferredTiledLightsEnabled=no`、`pbrDeferredTiledLightGridBound=no`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：17 个 PBR verification mode 全部通过。

### 2026-05-21 PBR Deferred Tiled Light Grid Config Object

Tiled light grid 的 `bind(...)` 接口已从松散参数收敛为配置对象：

- 新增 `PBRDeferredTiledLightGridConfig`，当前包含 `tileSize` 与 `lightCutoff`。
- `PBRDeferredTiledLightGrid::bind(...)` 现在接收 `const PBRDeferredTiledLightGridConfig&`，避免继续在调用点扩散多个 loose parameters。
- `PBRDeferredLightingPass` 与 `PBRDeferredTiledLightDebugPass` 都从 `RendererFramePassProfile` 构造同一类 grid config，保证 deferred lighting consumer 和 heatmap debug consumer 使用相同参数形态。
- 当前行为保持不变，验证输出仍保持 sparse tiled 场景 `7200 -> 2890`；这一步主要是 API 形态重构，为后续 clustered / GPU culling 继续加入 z slices、strategy、max lights per tile、depth-aware culling 等参数预留扩展点。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`：构建通过，focused tiled 输出 `pbrDeferredTiledLightGridFullIndices=7200`、`pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridCulledIndices=4310`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures -Modes deferred-tiled-heatmap`：heatmap consumer 验证通过，输出 `pbrDeferredTiledLightDebugDrawCalls=1`、`pbrDeferredTiledLightGridBound=yes`、`pbrDeferredTiledLightGridIndices=2890`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：17 个 PBR verification mode 全部通过。

### 2026-05-21 PBR Texture Set Verification Probe

PBR verification 已新增 texture-set probe，用于验证真实贴图链路，而不是只依赖纯参数材质球或简单 import probe：

- 新增 `--verify-pbr-texture-set` 命令行模式。
- 复用仓库已有 `fbx/bag` 贴图集，创建一个 generated plane probe，并绑定：
  - `diffuse.jpg` -> albedo map
  - `specular.jpg` -> metallic map
  - `roughness.jpg` -> roughness map
  - `ao.jpg` -> AO map
  - `normal.png` -> normal map
- `RuntimePBRVerification` 新增 `pbrTexturedMeshes` scene stat，用于证明 textured PBR probe 确实进入 verification scene。
- `tools/verify_pbr.ps1` 默认 PBR 回归从 17 个模式扩展为 18 个模式，并对 `texture-set` 模式断言 `pbrTexturedMeshes > 0` 且 `pbrDrawCalls >= 26`。

这一步还不是完整真实资产视觉基准，因为 `fbx/bag` 当前缺少对应 mesh；但它已经把 PBR 贴图采样链路纳入自动化验证，覆盖 albedo / metallic / roughness / AO / normal map 组合。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes texture-set`：构建通过，输出 `pbrTexturedMeshes=1`、`pbrMeshes=26`、`pbrDrawCalls=26`、capture 非黑比例 `99.9951%`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 18 个 verification mode 全部通过。

### 2026-05-21 PBR Deferred Texture Set Verification

Texture-set probe 已从 forward PBR 验证扩展到 deferred PBR 路径：

- 新增 `--verify-pbr-deferred-texture-set` 命令行模式。
- 该模式复用 `PBR Texture Set Probe`，但 renderer pass order 走 `PBRDepthPrepass,PBRGBuffer,PBRDeferredLighting`。
- `tools/verify_pbr.ps1` 新增 `deferred-texture-set` 默认模式，并复用 textured probe 断言，同时额外断言 deferred lighting pass 必须绘制。
- textured probe 断言现在同时支持 forward draw path 和 deferred G-buffer draw path，避免只用 `pbrDrawCalls` 判断导致 deferred 模式误判。

这一步把 albedo / metallic / roughness / AO / normal map 贴图集从 forward shader 验证推进到 G-buffer producer + deferred lighting consumer 验证。后续改 PBR G-buffer packing、surface binder 或 deferred material decode 时，该模式可以直接暴露贴图链路回归。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-texture-set`：构建通过，输出 `pbrTexturedMeshes=1`、`pbrGBufferDrawCalls=26`、`pbrDeferredLightingDrawCalls=1`、capture 非黑比例 `29.2184%`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 19 个 verification mode 全部通过。

### 2026-05-21 PBR Verification Argument Split

`main.cpp` 中不断增长的 PBR verification 参数解析已拆出到 application 层：

- 新增 `RuntimePBRVerificationArgs`，集中解析 `--verify-pbr-*` 参数，并生成 `RuntimeApplicationShellConfig`。
- `main.cpp` 现在只负责初始化 logger、创建 `RuntimeApplicationShell` 和进入 `RuntimeBootstrapper`，不再直接维护 19 个 PBR verification flag。
- 新模块继续覆盖 verification defaults、capture path、deferred pass 开关、tile size / cutoff overrides、texture-set / deferred texture-set probe 开关。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已加入新 `.cpp/.h`，确保 Visual Studio 工程和命令行 MSBuild 使用同一套文件。

这一步不改变渲染行为，目标是把 PBR verification 的模式矩阵从程序入口剥离。后续新增真实资产基准、clustered lighting 或更多 material parity 验证时，只需要扩展 `RuntimePBRVerificationArgs` 和 `tools/verify_pbr.ps1`，不会继续把 `main.cpp` 拉回脚本式总控。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-texture-set`：构建通过，确认 `RuntimePBRVerificationArgs.cpp` 已参与编译；输出保持 `pbrTexturedMeshes=1`、`pbrGBufferDrawCalls=26`、`pbrDeferredLightingDrawCalls=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 19 个 verification mode 全部通过。

### 2026-05-21 PBR Verification Mode Descriptor Table

`RuntimePBRVerificationArgs` 内部已从 19 个 bool flag 和长串 mode-specific `if` 继续收敛为 descriptor table：

- 新增 `PbrVerificationModeDescriptor`，每个 `--verify-pbr-*` mode 在一行中声明 argument、capture path、pass/probe option bit、tile size override 和 tiled light cutoff override。
- `makeShellConfigFromArguments(...)` 现在只遍历 mode descriptor，命中任意 PBR verification mode 后应用公共默认配置，再按 descriptor 合并具体 mode 行为。
- `--verify-pbr` 和 `--verify-pbr-no-atlas` 复用公共默认 capture path；deferred、G-buffer、texture-set、tiled light、heatmap 等特殊模式只声明自己的差异。
- 这一步继续压缩入口层耦合，为后续新增 `--verify-pbr-clustered`、真实资产基准或更多 material parity 验证预留更稳定的扩展点。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights-cutoff-005`：构建通过，输出 `pbrDeferredTiledLightGridCutoff=0.050000`、`pbrDeferredTiledLightGridIndices=714`、`pbrDeferredTiledLightGridCulledIndices=6486`，确认 descriptor 中 cutoff override / tiled probe / deferred pass 开关生效。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 19 个 verification mode 全部通过。

### 2026-05-21 PBR Deferred Light Culling Config Boundary

开始为 clustered / GPU light culling 建立上层配置边界：

- 新增 `PBRDeferredLightCullingConfig` 与 `PBRDeferredLightCullingMode`，当前实际后端为 `CpuTiled`，并预留 `GpuClustered` 枚举值。
- `PBRDeferredTiledLightGrid::bind(...)` 不再接收 tiled-grid 专用 config，而是接收更上层的 light culling config；当前只有 `CpuTiled` mode 会实际构建 SSBO tile list。
- `PBRDeferredLightingPass` 与 `PBRDeferredTiledLightDebugPass` 通过 `makePbrDeferredLightCullingConfig(...)` 从 `RendererFramePassProfile` 创建 culling config，调用点不再直接散落 tile size / cutoff config 构造。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已加入新的 `.cpp/.h`，保持 Visual Studio 工程分类同步。

这一步不改变现有 shader 行为和 verification 输出，目标是先把“使用哪种 deferred point-light culling backend”从 tiled grid 实现中抽出。后续接入 clustered / GPU culling 时，可以先扩展 `PBRDeferredLightCullingMode` 和 config，再替换 deferred lighting pass 的 backend 分派。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`：构建通过，确认 `PBRDeferredLightCullingConfig.cpp` 已参与编译；输出保持 `pbrDeferredTiledLightGridFullIndices=7200`、`pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridCulledIndices=4310`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 19 个 verification mode 全部通过。

### 2026-05-21 PBR Deferred Clustered Culling Layout Prep

在不启用 GPU clustered backend、不写 compute shader 的前提下，先把 `GpuClustered` 需要的 profile / layout / stats 数据边界补齐：

- `RendererFramePassProfile` 新增 `pbrDeferredClusteredDepthSlices` 和 `pbrDeferredClusteredMaxLightsPerCluster`，默认分别为 `24` 和 `64`，并接入 Debug UI / profile config schema。
- `config/renderer_frame_pass.example.ini` 新增上述 clustered 参数，后续本地实验可以先调 layout 参数，再接 backend 实现。
- `PBRDeferredLightCullingConfig` 新增 `clusterDepthSlices` 和 `maxLightsPerCluster`，`makePbrDeferredLightCullingConfig(...)` 会从 profile 读取并 clamp 到安全范围。
- 新增 `PBRDeferredClusteredLightGridLayout` 和 `PBRDeferredClusteredLightGridStats`，明确 clustered backend 后续至少需要 cluster columns / rows / depth slices / cluster count / max light index count / binding points / point-light stats。
- 新增 `makePbrDeferredClusteredLightGridLayout(...)` 与 `usesPbrDeferredGpuClusteredLightGrid(...)` helper，为后续 `GpuClustered` backend 分派和 verification stats 输出预留入口。

这一步仍保持默认运行使用 `CpuTiled`，因此不改变当前 deferred lighting shader 行为。后续实现 GPU clustered culling 时，应优先复用这组 layout/stats 结构，而不是直接在 pass 内部临时写 buffer 布局。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights`：构建通过，确认 clustered profile/layout 字段已参与编译；当前 CPU tiled path 输出保持 `pbrDeferredTiledLightGridFullIndices=7200`、`pbrDeferredTiledLightGridIndices=2890`、`pbrDeferredTiledLightGridCulledIndices=4310`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 19 个 verification mode 全部通过。

### 2026-05-21 PBR Deferred Clustered Layout Verification Mode

新增非 compute 的 clustered layout verification mode，用于先验证 `GpuClustered` 数据边界和统计输出：

- `RuntimePBRVerificationConfig` 新增 `enablePbrClusteredLayoutProbe`，`RuntimePBRVerificationArgs` 新增 `--verify-pbr-deferred-clustered-layout`。
- 新 verification mode 启用 PBR G-buffer + deferred lighting，打开 `RendererFramePassProfile::pbrDeferredClusteredLayoutStatsEnabled`，并关闭 CPU tiled lights。
- `PBRDeferredLightingPass` 在 clustered layout stats 开启时构造 `GpuClustered` culling config，计算 cluster columns / rows / depth slices / cluster count / max index capacity，但不绑定 GPU clustered buffers，不改变 shader 的当前 lighting 路径。
- `RendererFrameStats`、`RendererFramePassRegistry`、runtime verification 输出和 Debug UI 增加 clustered layout stats 字段。
- `tools/verify_pbr.ps1` 新增 `deferred-clustered-layout` mode，并断言 clustered layout enabled、buffer bound 为 no、CPU tiled lights disabled、cluster count 等于 dimensions 乘积、max index capacity 等于 cluster count 乘 max lights per cluster。

这一步是 GPU clustered culling 的 verification scaffold。它不声称 clustered culling 已实现，只保证后续 backend 实现前，layout 参数和统计输出已经可自动化验证。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-clustered-layout`：构建通过，输出 `pbrDeferredClusteredLightGridEnabled=yes`、`pbrDeferredClusteredLightGridBound=no`、`pbrDeferredClusteredLightGridSize=80x45x24`、`pbrDeferredClusteredLightGridClusters=86400`、`pbrDeferredClusteredLightGridMaxLightsPerCluster=64`、`pbrDeferredClusteredLightGridMaxIndices=5529600`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 20 个 verification mode 全部通过。

### 2026-05-21 PBR Deferred Clustered Grid CPU Backend

在 clustered layout-only scaffold 之后，新增 CPU-filled clustered grid backend，用于先打通真实 clustered SSBO binding 和 shader consumer，再进入 compute shader assignment：

- 新增 `PBRDeferredClusteredLightGrid`，按 screen tile + linear depth slice 构建 clustered offset/count buffer 和 light index buffer。
- clustered buffer 使用 SSBO binding `6`，clustered index buffer 使用 SSBO binding `7`；结构与 tiled grid 保持同类 flat `{offset,count}` + index list 形式。
- `PBRDeferredLightingPass` 新增 `pbrDeferredClusteredLightsEnabled` profile 开关，clustered grid bound 成功后 shader 使用 clustered point-light loop；如果 clustered 未启用或绑定失败，仍可回退 tiled/global path。
- `pbr_deferred_lighting.frag` 新增 clustered consumer 分支，根据 `gl_FragCoord` 和 `viewMatrix * worldPosition` 计算 cluster index 并遍历 clustered light indices。
- `RuntimePBRVerificationArgs` 新增 `--verify-pbr-deferred-clustered-grid`，该模式启用 PBR G-buffer + deferred lighting + clustered grid，关闭 CPU tiled grid。
- `tools/verify_pbr.ps1` 默认 PBR 回归新增 `deferred-clustered-grid` mode，并断言 clustered buffers 必须 bound、CPU tiled grid 不绑定、cluster dimensions / capacity 一致、clustered index count 大于 0 且小于全局 cluster-light loop。

这一步仍不是最终 GPU compute clustered culling。它的作用是先验证 clustered buffer layout、binding point、shader consumer 和 stats 输出，后续可以把 CPU assignment 替换为 compute shader，而不需要同时调试 shader consumer 和 buffer layout。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-clustered-grid`：构建通过，输出 `pbrDeferredClusteredLightGridBound=yes`、`pbrDeferredClusteredLightGridSize=80x45x24`、`pbrDeferredClusteredLightGridIndices=14400`、`pbrDeferredClusteredLightGridCulledIndices=158400`，CPU tiled grid 为 `bound=no`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 21 个 verification mode 全部通过。

### 2026-05-21 PBR Deferred Clustered Compute Assignment

在 CPU-filled clustered grid backend 之后，新增最小 compute shader assignment 闭环：

- `Shader` 新增 compute shader 构造函数，支持单独编译 / link `GL_COMPUTE_SHADER`。
- `ShaderLibrary` 新增 `getPbrDeferredClusteredLightGridComputeShader()`，加载 `shaders/pbr/pbr_deferred_clustered_light_grid.comp`。
- 新 compute shader 每个 invocation 负责一个 cluster，遍历 deferred point light buffer，按 screen tile circle overlap 和 linear depth slice overlap 写入 clustered `{offset,count}` buffer 与 fixed-capacity light index buffer。
- `PBRDeferredClusteredLightGrid::bindCompute(...)` 分配 clustered SSBO，dispatch compute shader，并通过 cluster buffer readback 聚合 `lightIndexCount`，用于 verification stats。
- `PBRDeferredLightingPass` 的 clustered path 现在调用 compute backend；dispatch 后恢复 deferred lighting shader，并继续使用 clustered shader consumer。
- runtime stats / Debug UI / verification 输出新增 `pbrDeferredClusteredLightGridCompute=yes/no`。
- `tools/verify_pbr.ps1` 对 `deferred-clustered-grid` 新增 compute dispatch 断言，要求 `pbrDeferredClusteredLightGridCompute=yes`。

这一步已经把 clustered light assignment 从 CPU 构建推进到 GPU compute dispatch。当前仍保留 GPU readback 用于验证统计；后续优化方向是减少或配置化 readback，并增加 clustered occupancy debug pass / GPU timing。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-clustered-grid`：构建通过，输出 `pbrDeferredClusteredLightGridBound=yes`、`pbrDeferredClusteredLightGridCompute=yes`、`pbrDeferredClusteredLightGridIndices=14400`、`pbrDeferredClusteredLightGridCulledIndices=158400`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 21 个 verification mode 全部通过。

### 2026-05-21 PBR Deferred Clustered Readback Gate

clustered compute assignment 已经能在 GPU 上生成 clustered light grid，但上一轮实现仍在每次 dispatch 后执行 `glGetBufferSubData(...)` 读取 cluster count。该读回适合 verification，但不适合作为默认渲染路径，因为它会引入 CPU/GPU 同步点。

本轮将 readback 改为显式 profile/verification gate：

- `RendererFramePassProfile` 新增 `pbrDeferredClusteredStatsReadbackEnabled`，默认 `false`，并接入 profile config schema 与 `config/renderer_frame_pass.example.ini`。
- `PBRDeferredLightCullingConfig` 新增 `clusteredStatsReadbackEnabled`，clustered backend 不再隐式同步读回 GPU buffer。
- `PBRDeferredClusteredLightGrid::bindCompute(...)` 默认只 dispatch compute shader 并设置 SSBO memory barrier；只有 readback gate 打开时才执行 `glGetBufferSubData(...)` 并填充 actual light index / culled index stats。
- clustered stats 新增 `statsReadbackEnabled` 与 `lightIndexStatsAvailable`，避免 UI / verification 把 no-readback 模式下的 `0` 误解成真实 culling 数。
- `RuntimePBRVerificationArgs` 新增 `--verify-pbr-deferred-clustered-grid-no-readback`，用于验证默认 no-readback clustered compute path；原 `--verify-pbr-deferred-clustered-grid` 仍显式打开 readback，用于验证 actual culling counts。
- `tools/verify_pbr.ps1` 默认 PBR 回归新增 `deferred-clustered-grid-no-readback` mode，总数从 21 个增加到 22 个。

这一步把 clustered compute 从“验证型实现”推进到更接近真实 renderer 的路径：默认渲染不再依赖同步读回，只有 debug / verification 需要实际 index count 时才打开 readback。后续仍需要补 GPU timing、occupancy debug pass，以及更真实多光源压力场景。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-clustered-grid,deferred-clustered-grid-no-readback`：构建通过；readback mode 输出 `pbrDeferredClusteredLightGridStatsReadback=yes`、`pbrDeferredClusteredLightGridLightIndexStats=yes`、`pbrDeferredClusteredLightGridIndices=14400`、`pbrDeferredClusteredLightGridCulledIndices=158400`；no-readback mode 输出 `pbrDeferredClusteredLightGridStatsReadback=no`、`pbrDeferredClusteredLightGridLightIndexStats=no`，且 clustered compute / buffer bound 仍为 `yes`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 22 个 verification mode 全部通过。

### 2026-05-21 PBR Deferred Clustered Occupancy Debug Pass

在 clustered compute assignment 和 readback gate 之后，新增 clustered occupancy debug visualization，用于直接观察 clustered grid 的屏幕空间热力图：

- 新增 `PBRDeferredClusteredLightDebugPass`，结构对齐已有 `PBRDeferredTiledLightDebugPass`。
- 新增 pass key `PBRDeferredClusteredLightDebug`，可通过 renderer pass plan 插入在 `PBRGBuffer` 之后。
- 新增 `pbr_deferred_clustered_light_debug.frag`，直接读取 clustered offset/count SSBO binding `6`，按 tile 聚合 depth slices 的最大 light count 并输出 heatmap。
- debug pass 会调用 clustered compute backend 生成 SSBO，但强制 `clusteredStatsReadbackEnabled=false`，因此 visualization 本身不依赖 CPU readback。
- `RendererFramePassProfile` 新增 `pbrDeferredClusteredLightDebugDepthSlice`、`pbrDeferredClusteredLightDebugMaxLights`、`pbrDeferredClusteredLightDebugIntensity`，支持选择单个 depth slice 或默认 `-1` 聚合所有 depth slices 的最大 occupancy。
- `RuntimePBRVerificationArgs` 新增 `--verify-pbr-deferred-clustered-heatmap`，`tools/verify_pbr.ps1` 默认 PBR 回归新增 `deferred-clustered-heatmap`，总数从 22 个增加到 23 个。

这一步补上了 clustered path 的可视化调试入口。它不能替代 GPU timing 或真实压力场景，但能避免后续 clustered culling 变更只靠最终画面判断是否正确。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-clustered-heatmap`：构建通过，输出 `pbrDeferredClusteredLightDebugDrawCalls=1`、`pbrDeferredClusteredLightGridBound=yes`、`pbrDeferredClusteredLightGridCompute=yes`、`pbrDeferredClusteredLightGridStatsReadback=no`、capture 非黑比例 `100%`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 23 个 verification mode 全部通过。

### 2026-05-21 PBR Deferred Light Pressure Verification

在 clustered heatmap 之后，补充更真实的多点光源压力验证，避免后续只用默认 2 盏点光或老 Phong 场景判断 PBR tiled / clustered culling 是否有效：

- `PBRLightRigProfile::maxPointLights` 从 `2` 提升到 `8`，但普通 verification 默认仍保持原有少量点光，不强制所有模式变成压力场景。
- `RuntimePBRVerificationConfig` 新增 `enablePbrLightPressureProbe`，用于只在明确 pressure mode 下启用 8 盏点光。
- `RuntimePBRVerification.cpp` 新增 `applyPressurePointLightRig(...)`，注入 8 个位置、颜色和强度不同的 point light，并统一使用较紧的 attenuation `K2=96.0`，让 tiled / clustered culling 有更明显的压力。
- `RuntimePBRVerificationArgs` 新增 `--verify-pbr-deferred-tiled-lights-pressure` 和 `--verify-pbr-deferred-clustered-grid-pressure`。
- `tools/verify_pbr.ps1` 默认 PBR 回归新增 `deferred-tiled-lights-pressure` 与 `deferred-clustered-grid-pressure`，并通过 `ExpectPointLightPressure=8` 检查 renderer stats 中至少报告 8 个 point lights。

这一步的重点是把“PBR 多光源 culling 是否真的工作”从主观观察推进到可重复验证。它不替代 GPU timing；它只证明 tiled / clustered 在 8 点光压力场景下仍能生成合理 light list 并完成渲染。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-tiled-lights-pressure,deferred-clustered-grid-pressure`：构建通过；tiled pressure 输出 `pointShadowLights=8`、`pbrDeferredLightBufferPointLights=8/16`、`pbrDeferredTiledLightGridPointLights=8`、`pbrDeferredTiledLightGridFullIndices=28800`、`pbrDeferredTiledLightGridIndices=8034`、`pbrDeferredTiledLightGridCulledIndices=20766`、`pbrDeferredTiledLightGridOccupiedTiles=3568/3600`；clustered pressure 输出 `pbrDeferredClusteredLightGridPointLights=8`、`pbrDeferredClusteredLightGridIndices=8037`、`pbrDeferredClusteredLightGridCulledIndices=683163`、`pbrDeferredClusteredLightGridCompute=yes`、`pbrDeferredClusteredLightGridStatsReadback=yes`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 25 个 verification mode 全部通过。

### 2026-05-21 Renderer Pass GPU Timing Probe

在 pressure scene 之后，补充 renderer pass 级 GPU timing probe，用于让后续 tiled / clustered / deferred lighting 对比不再只依赖 index count 和 draw call：

- `RendererFramePassProfile` 新增 `rendererGpuTimingEnabled`，默认 `false`，并接入 profile config 和 Debug UI；该开关明确用于 profiling / verification，不作为普通渲染默认行为。
- `RendererFramePassRegistry::executePass(...)` 在 timing 开启时用 `GL_TIME_ELAPSED` 包住每个 renderer pass，并在同帧读回 query result 写入 `RendererFrameStats`。
- `RendererFrameStats` 新增 renderer GPU timing 总量、timed pass count，以及 BeginFrame / ShadowMaps / PBRShadowAtlas / PBRDepthPrepass / PBRGBuffer / PBRDeferredLighting / debug pass / PBR scene pass 的纳秒统计。
- `RuntimePBRVerificationArgs` 新增 `--verify-pbr-deferred-clustered-grid-timing`，启用 clustered grid + readback + renderer GPU timing。
- `tools/verify_pbr.ps1` 默认回归新增 `deferred-clustered-grid-timing` mode，并断言 `rendererGpuTimingEnabled=yes`、`rendererGpuTimingAvailable=yes`、timed pass 数量足够、frame / G-buffer / deferred lighting GPU 时间为正数。

这个实现会同步读回 timer query，因此它是显式 profiling 工具，不是最终低开销 telemetry。后续如果要长期在 UI 中显示稳定 timing，应再改成跨帧 query ring buffer，避免同帧阻塞。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-clustered-grid-timing`：构建通过，输出 `rendererGpuTimingEnabled=yes`、`rendererGpuTimingAvailable=yes`、`rendererGpuTimedPasses=6`、`rendererGpuFrameNs=7024410`、`rendererGpuPbrGBufferNs=720440`、`rendererGpuPbrDeferredLightingNs=3019100`，并保持 `pbrDeferredClusteredLightGridCompute=yes`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 26 个 verification mode 全部通过。

### 2026-05-21 Renderer GPU Timing Deferred Readback

上一轮 renderer GPU timing probe 已经可以输出 pass 级 GPU 时间，但实现仍在同帧对每个 `GL_TIME_ELAPSED` query 调用 `GL_QUERY_RESULT`，这会强制 CPU 等 GPU 完成当前 pass。为了让 profiling 更接近真实运行，本轮将它改为跨帧延迟读回：

- 新增 `RendererGpuTimerQueryPool`，由 `Renderer` 持有，负责 query 对象复用、当前帧 query 收集、旧帧 query 可用性检查和结果读回。
- `Renderer::render(...)` 在每帧开始调用 `beginFrame(...)`，只尝试读取已经 `GL_QUERY_RESULT_AVAILABLE` 的旧帧 query；本帧结束后把当前帧 query 放入 pending frame 队列。
- `RendererFramePassRegistry` 不再直接 `glGetQueryObjectui64v(...GL_QUERY_RESULT...)`，只负责在 pass 前后调用 query pool 的 `beginPass(...)` / `endPass(...)`。
- `RendererFrameStats` 新增 `rendererGpuTimingDeferredReadback` 和 `rendererGpuTimingPendingQueries`，用于区分当前 timing probe 是延迟读回还是同帧阻塞读回。
- `--verify-pbr-deferred-clustered-grid-timing` 的 capture frame 延后到第 5 帧，确保 verification 报告读取的是旧帧完成的 query，而不是当前帧同步等待。
- `tools/verify_pbr.ps1` 对 timing mode 新增 `rendererGpuTimingDeferredReadback=yes` 与 `rendererGpuTimingPendingQueries > 0` 断言。

这一步仍然是 profiling / verification 工具，不默认启用；但相比上一轮，它不再为了拿到当前帧 timing 而主动阻塞当前帧 GPU work。后续可在这个基础上做 tiled / clustered profile 对比报告。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -DiscardCaptures -Modes deferred-clustered-grid-timing`：构建通过，输出 `rendererGpuTimingEnabled=yes`、`rendererGpuTimingAvailable=yes`、`rendererGpuTimingDeferredReadback=yes`、`rendererGpuTimedPasses=6`、`rendererGpuTimingPendingQueries=12`、`rendererGpuPbrGBufferNs=218600`、`rendererGpuPbrDeferredLightingNs=2172830`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 26 个 verification mode 全部通过。

### 2026-05-21 PBR Light Culling Pressure Timing Comparison

在 renderer GPU timing 已经改成跨帧 readback 后，本轮补上 tiled / clustered 在同一压力场景下的对比入口，避免继续用不同 verification scene 或单独 clustered timing 推断性能方向：

- `RuntimePBRVerificationArgs` 新增 `--verify-pbr-deferred-tiled-lights-pressure-timing` 与 `--verify-pbr-deferred-clustered-grid-pressure-timing`，两者都启用 8 点光 pressure rig 和 renderer GPU timing。
- `tools/verify_pbr.ps1` 默认回归新增 `deferred-tiled-lights-pressure-timing` 与 `deferred-clustered-grid-pressure-timing`，并复用 `ExpectPointLightPressure=8`、tiled / clustered grid 断言和 `ExpectGpuTiming` 断言。
- 新增 `tools/profile_pbr_light_culling.ps1`，它会运行两个 pressure timing mode，解析 renderer stats，并生成 `docs/pbr_light_culling_timing_report.md`。
- 当前报告记录 tiled pressure path 为 `80x45` tile grid、8 点光、`8034 / 28800` live indices；clustered pressure path 为 `80x45x24` cluster grid、8 点光、`8037 / 691200` live indices。
- 基于已提交代码 `bf85071` 重新采样后，tiled deferred lighting GPU time 约 `1.0270 ms`，clustered deferred lighting GPU time 约 `2.9362 ms`；这只能作为当前机器单次 pressure rig 基线，不能作为最终性能结论。

这一步的意义是把“clustered 是否值得继续优化”拆成可复现问题。当前证据说明：tiled 仍应保留为稳定 fallback；clustered 已经有 compute assignment 和 3D grid 基础，但下一步需要优先做 overflow/fallback、多帧平均 timing，以及真实资产下的视觉/性能 baseline，而不是直接把 clustered 设为默认。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\profile_pbr_light_culling.ps1 -NoLinkDebugInfo`：构建通过，两个 pressure timing mode 均通过，生成 `docs/pbr_light_culling_timing_report.md`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 28 个 verification mode 全部通过。

### 2026-05-21 PBR Light Culling Multi-Sample Timing

上一轮 timing comparison 解决了“没有 tiled / clustered 对比报告”的问题，但仍只是一轮采样。GPU timing 在桌面环境下可能受到驱动调度、缓存、窗口系统和前序 pass 波动影响，因此本轮把 profiling 脚本扩展为多样本统计：

- `tools/profile_pbr_light_culling.ps1` 新增 `-Samples N`，默认仍为 `1`，保持单次 baseline 的兼容行为。
- 多样本模式下第一轮按参数决定是否构建，后续样本自动 `-SkipBuild`，避免把重复构建时间混进 profiling 流程。
- 报告结果表从单值改为 `avg / min / max ms`，分别覆盖 G-buffer、deferred lighting 和 frame GPU time。
- 新增 `docs/pbr_light_culling_timing_samples.csv`，逐样本保存 path、grid、index count、G-buffer time、deferred lighting time、frame time 和 pending query 数量，便于后续画图或比较多次运行。
- `docs/pbr_light_culling_timing_report.md` 新增样本表与 CSV 路径，最后保留最后一轮 renderer stats 用于定位具体 verification 输出。

这一步不改变渲染结果，也不尝试把 clustered 设为默认。它的作用是让后续性能讨论至少基于多样本统计，而不是单次 timing；下一步可以把 `Samples` 提高到 5 或 10，并增加 CSV 对比脚本或真实资产场景。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\profile_pbr_light_culling.ps1 -SkipBuild -NoLinkDebugInfo -Samples 3`：两个 pressure timing mode 连续三轮通过，报告中输出 tiled / clustered 的 avg/min/max GPU 时间，并生成 CSV 样本表。当前样本集的 deferred lighting 平均时间为 tiled `1.0301 ms`、clustered `3.4366 ms`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归 28 个 verification mode 全部通过。

### 2026-05-21 Engine Roadmap And PBR Freeze Boundary

本轮将项目目标从“继续扩张 PBR renderer”重新收束到“以 PBR 作为第一个 renderer pipeline，继续建设游戏引擎”：

- 新增 `docs/engine_roadmap.md`，明确后续路线为 Engine Core、Scene / Entity、Asset / Serialization、Editor Foundation、Runtime Gameplay、Renderer Module，而不是继续把 PBR 作为项目中心。
- 新增 `docs/pbr_final_design.md`，定义 PBR 阶段冻结边界：保留 forward/deferred PBR、G-buffer、shadow atlas、tiled/clustered probe、GPU timing、pressure profiling 和 verification；暂停 full glTF material parity、OIT、production clustered overflow、高级 GI 等 renderer-only 扩张。
- PBR 默认策略调整为保守：forward PBR 用于 preview / asset probe；deferred PBR 用于 pipeline validation；tiled deferred 作为稳定 fallback；clustered deferred 保持 profiling / experimental 状态。
- 后续新增渲染工作需要先回答是否能让 engine boundary 更清楚；如果不能，就应延后到 renderer product 阶段。

接下来只做 PBR 最小收尾：提交 real-asset golden baseline workflow，并确认 `verify_pbr.ps1` 与 `verify_pbr_golden.ps1` 都通过。完成后停止 PBR 扩张，转入 engine core / scene / asset / editor 方向讨论。

### 2026-05-21 PBR Final Gate Verification

本轮完成 PBR 冻结前的最小必要验证设计：

- 新增 `tools/verify_pbr_golden.ps1`，将真实 Assimp import、PBR texture set、deferred texture set 三个模式组织成 golden baseline workflow。
- 新增 `docs/pbr_golden_baselines.json`，记录当前 baseline 的分辨率、PPM 字节数、non-black 百分比、mean RGB、SHA256 和关键 scene / renderer stats pattern。
- 新增 `docs/pbr_golden_verification_report.md`，记录最近一次 golden 验证结果。
- 更新 `docs/pbr_final_design.md`，把 golden gate 和完整 28-mode PBR 回归作为 PBR freeze 的最终证据。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr_golden.ps1 -SkipBuild -NoLinkDebugInfo -DiscardCaptures`：通过；`import`、`texture-set`、`deferred-texture-set` 三个 golden mode 均匹配 baseline。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：通过；默认 28 个 PBR verification mode 全部通过。

结论：PBR 作为 engine foundation renderer pipeline 已具备冻结条件。后续除非是 bug fix、验证修复或 engine boundary 需要，否则不应继续扩张 PBR 功能。

### 2026-05-21 PBR Showcase Sphere Scene

根据“实现一个 PBR 渲染球场景，展示已经完成的内容”的目标，本轮新增一个独立 verification showcase mode，而不是改默认工程启动场景：

- 新增 `--verify-pbr-showcase-spheres`，`tools/verify_pbr.ps1` 中对应 `-Modes showcase-spheres`。
- showcase scene 使用现有本地资源，不下载外部纹理：`Texture/solar system` 行星/太阳贴图、`Texture/normal/normal_map.png`、`fbx/bag` 的 diffuse / specular / roughness / AO / normal 贴图。
- 场景上排新增 6 个大 PBR sphere：Earth albedo IBL、Mars rough dielectric、texture-set + normal/roughness/AO、gold metallic low roughness、glossy normal-map、emissive sun/bloom。
- 场景下排保留材质球矩阵，展示 metallic / roughness 参数变化。
- showcase mode 使用 deferred PBR、G-buffer、deferred lighting、tiled point-light culling、8 点光 pressure rig、PBR shadow atlas、procedural IBL 和 verification capture。
- 新增 `docs/pbr_showcase_scene.md` 记录运行方式、场景内容、验证契约和资源策略。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes showcase-spheres`：构建通过；输出 `pbrShowcaseSpheres=6`、`pbrGBufferDrawCalls=26`、`pbrDeferredLightingDrawCalls=1`、`pointShadowLights=8`、`pbrDeferredTiledLightGridBound=yes`、`pbrDeferredTiledLightGridIndices=7167/28800`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：通过；默认 PBR 回归从 28 个 mode 增加到 29 个 mode，新增 `showcase-spheres`。
- 已生成本地截图 `out/pbr_showcase_spheres_verification.ppm`，并临时转换为 `out/pbr_showcase_spheres_verification.png` 做视觉检查。

### 2026-05-25 Engine Transformation Project Book Draft

在确认 PBR 已经不应继续作为项目中心扩张后，本轮将“后续引擎化改造”从路线判断推进为可审核的项目书和接口草案：

- 新增 `docs/engine_transformation_project_plan.md`，作为后续引擎化改造项目方案。文档明确当前基线、项目目标、非目标、总体架构、Phase 0 到 Phase 6 的阶段计划、验收门槛、风险控制和需要用户审核的问题。
- 新增 `docs/engine_interface_design.md`，作为后续接口设计草案。文档定义 Engine Core、Scene / Entity、Asset、Property Schema、Renderer、Editor、Input、Diagnostics / Verification 等接口边界，并明确哪些只是过渡 adapter、哪些是长期 engine-facing API。
- 新增 `docs/aigc_engine_workflow_requirements.md`，作为本项目后续 AIGC 协作范式要求。文档明确文档先行、当前状态优先、小步验证、`worked.md` 记录、任务分级、标准工作循环、Git 约束、verification 要求和审核清单。
- 新增 `docs/engine_project_book.md`，作为本轮文档包总入口，集中说明项目结论摘要、审核文档清单、必须审核的十个决策、Phase 1 第一轮工作和当前本地状态提醒。
- 新增 `docs/engine_project_approval_checklist.md`，把用户审核入口集中为决策表，明确哪些方向必须确认、哪些内容可后续微调，以及进入 Phase 1 前需要处理的本地状态。
- 新增 `docs/engine_phase1_execution_plan.md`，把审核通过后的第一轮 Engine Core 实现拆成可执行步骤：只新增 `engine/IEngineModule.h`、`engine/EngineContext.h`、`engine/Engine.h/.cpp` 并注册工程，不接管 renderer、不迁移 scene、不改变启动行为。

本轮只做文档设计，不修改运行时代码。下一步应先由用户审核项目书文档包，再决定是否进入 Phase 1 的 Engine Core 最小接口实现。

### 2026-05-25 UE5 Inspired Framework Direction And Phase 1 Skeleton

根据最新目标，前一轮“等待审核的 Entity / Renderer boundary 项目书”被修正为“UE5 启发式引擎框架，自主推进”：

- `docs/engine_project_book.md` 改为 UE5 启发式项目书，明确主线为 `Engine -> World -> Level -> Actor -> Component -> Subsystem`。
- `docs/engine_interface_design.md` 改为 UE5 启发式接口设计，原 `EntityId / RenderWorldSnapshot` 主线降级为未来内部实现细节。
- `docs/engine_transformation_project_plan.md` 改为 UE5 启发式阶段方案，Phase 1 直接落 framework skeleton。
- `docs/aigc_engine_workflow_requirements.md` 改为自主推进规则，不再以人工审核阻塞当前阶段。
- `docs/engine_project_approval_checklist.md` 改为自主执行检查清单。
- `docs/engine_phase1_execution_plan.md` 改为 UE5 启发式 Phase 1 执行计划，并在验证通过后记录为 `Implemented and verified`。

本轮新增第一批非侵入式 engine framework 代码：

- `engine/Transform.h`
- `engine/EngineObject.h/.cpp`
- `engine/EngineContext.h`
- `engine/EngineSubsystem.h`
- `engine/ActorComponent.h/.cpp`
- `engine/SceneComponent.h/.cpp`
- `engine/Actor.h/.cpp`
- `engine/Level.h/.cpp`
- `engine/World.h/.cpp`
- `engine/Engine.h/.cpp`
- `engine/RendererSubsystem.h/.cpp`
- `engine/ActorAdapters.h/.cpp`

工程接入：

- 更新 `text2.vcxproj`，注册新增 `engine` 源文件和头文件。
- 更新 `text2.vcxproj.filters`，新增 `cppfile\GLengine` 与 `include\GLengine` 过滤器。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes showcase-spheres -DiscardCaptures`：构建通过；`--verify-pbr-showcase-spheres` 通过；输出 `pbrShowcaseSpheres=6`、`pbrDeferredLightingDrawCalls=1`、`pbrDeferredTiledLightGridBound=yes`。
- 新增 `RendererSubsystem` 后重新执行同一命令：构建通过；`RendererSubsystem.cpp` 参与编译；`--verify-pbr-showcase-spheres` 继续通过。
- 新增 `ActorAdapters` 后重新执行同一命令：构建通过；`ActorAdapters.cpp` 参与编译；`--verify-pbr-showcase-spheres` 继续通过。

下一步建议：

- 新增 World-driven scene setup，让一个最小场景片段由 `World / Level / Actor` 生成，同时保留旧 scene path。

### 2026-05-25 Legacy Scene To World Bridge

本轮继续 UE5 启发式引擎化方向，完成旧 `Scene / Object` 树到新 `World / Level / Actor` 框架的第一版非侵入式桥接：

- `ActorAdapters` 新增 `LegacyObjectComponent` 和 `LegacyObjectActor`，用于承载旧 `GLframework::Object` 指针。
- 新增 `engine/LegacySceneWorldBuilder.h/.cpp`，提供 `importScene(...)`、`importObjectTree(...)` 和 `makeTransform(...)`。
- `LegacySceneWorldBuilder` 会把旧 `Scene / Object / Mesh / InstancedMesh / Light` 递归导入到 `Level`：
  - `Mesh / InstancedMesh` 生成 `MeshActor`。
  - `Light` 生成 `LightActor`。
  - 普通 `Object / Scene` 生成 `LegacyObjectActor`。
  - 旧对象的 position / rotation / scale 写入新 `SceneComponent::relativeTransform`。
  - 旧对象父子关系映射为 `SceneComponent::attachTo(...)`。
- 更新 `text2.vcxproj` 和 `text2.vcxproj.filters`，将新增 bridge 文件纳入 `cppfile\GLengine` / `include\GLengine`。

这一步仍然不修改 `main.cpp`，不接管旧 `SceneSetup`，不改变 renderer 执行路径。它的意义是先让旧场景数据能被新框架表示，后续再逐步把 editor hierarchy、scene setup 和 runtime composition root 切到 `World / Level / Actor`。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes showcase-spheres -DiscardCaptures`：通过；`LegacySceneWorldBuilder.cpp` 已参与 `Debug|x64` 构建；PBR showcase 仍输出 `pbrShowcaseSpheres=6`、`pbrDeferredLightingDrawCalls=1`、`pbrDeferredTiledLightGridBound=yes`。
- 独立 smoke：使用临时 C++ 程序构造旧 `Scene -> Object -> Object`，调用 `LegacySceneWorldBuilder::importScene(...)`，验证生成 3 个 Actor、2 条组件 attach、transform 导入正确，退出码为 0。

下一步建议：

- 把一个受控的 scene setup probe 改为先生成 `World / Level / Actor`，再由 adapter 回填旧 renderer 所需的 `Scene`，验证新框架可以开始驱动场景装配。

### 2026-05-25 World Driven Scene Export Probe

本轮完成了上一节提出的受控 scene setup probe：新框架先生成场景片段，再回填旧 renderer 所需的 `GLframework::Scene`。

新增与修改：

- 新增 `engine/WorldLegacySceneExporter.h/.cpp`，用于从 `World / Level / Actor` 导出旧 `Scene` 可渲染对象。
- `WorldLegacySceneExporter` 遍历 `MeshComponent`、`LightComponent`、`LegacyObjectComponent`，把组件持有的旧对象加入目标 `Scene`。
- 导出时会把 `SceneComponent::relativeTransform` 写回旧 `Object` 的 position / rotation / scale。
- 新 `SceneComponent` 父子关系会被映射成旧 `Object::addChild(...)`，因此旧 renderer 仍可通过旧对象树递归渲染。
- 新增 `--verify-engine-world-scene-probe`，验证模式内部创建一个 `GLengine::World`，在 `Level` 中生成 root `LegacyObjectActor` 和 child `MeshActor`，再通过 `WorldLegacySceneExporter` 回填到 `sceneOffScreen`。
- `tools/verify_pbr.ps1` 新增 `engine-world-scene-probe` mode，并将其纳入默认 PBR verification 列表。
- `RuntimePBRVerification` 新增 `engineWorldProbeMeshes` 场景统计和 `Engine world scene probe stats` bridge 统计输出。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-probe -DiscardCaptures`：构建通过；`WorldLegacySceneExporter.cpp`、`RuntimePBRVerification.cpp`、`RuntimePBRVerificationArgs.cpp` 参与编译；`engineWorldProbeMeshes=1`；`pbrGBufferDrawCalls=26`；`pbrDeferredLightingDrawCalls=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 PBR 回归通过；当前默认 mode 数量从 29 增加到 30，新增 `engine-world-scene-probe`。

结论：

- 当前已经证明新 `World / Level / Actor` 不只是能“包住旧对象”，还可以作为受控 scene setup 的来源，并通过 adapter 回填给旧 renderer。
- 默认启动行为仍未改变；`main.cpp` 未接管新 engine；renderer 主流程未被改写。

下一步建议：

- 把这个 verification-only probe 提取成 `tools/sceneSetup` 下的可复用 World-driven scene setup helper，让 runtime 后续可以按配置选择旧 scene setup 或新 World setup。

### 2026-05-25 World Driven Scene Setup Helper

本轮将上一节的 verification-only probe 抽成 `tools/sceneSetup` 下的可复用 helper，继续推进从“验证专用桥接”到“可被 runtime 组合根复用的场景生成入口”。

新增与修改：

- 新增 `tools/sceneSetup/WorldDrivenSceneSetup.h/.cpp`。
- `WorldDrivenSceneSetup` 目前提供：
  - `addEngineWorldSceneProbe(renderer, scene)`：创建一个临时 `GLengine::World`，在 persistent `Level` 中生成 root `LegacyObjectActor` 与 child `MeshActor`，再通过 `WorldLegacySceneExporter` 导出到旧 `GLframework::Scene`。
  - `formatEngineWorldSceneProbeStats(...)`：统一输出 bridge stats，保持 `tools/verify_pbr.ps1` 可继续解析。
- `RuntimePBRVerification.cpp` 不再直接 include `ActorAdapters / Level / World / WorldLegacySceneExporter`，而是调用 `GL_SCENE::addEngineWorldSceneProbe(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 helper 源文件和头文件。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-probe -DiscardCaptures`：构建通过；`WorldDrivenSceneSetup.cpp` 与 `RuntimePBRVerification.cpp` 参与编译；`engineWorldProbeMeshes=1`；`pbrGBufferDrawCalls=26`；`pbrDeferredLightingDrawCalls=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 30 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有既有 LF/CRLF warning。

结论：

- 当前新框架已经具备双向 adapter：旧 `Scene` 可导入新 `World`，新 `World` 也可导出旧 `Scene`。
- World-driven scene setup 不再只存在于 `RuntimePBRVerification` 匿名函数里，已经成为 `tools/sceneSetup` 下可复用模块。
- 默认启动行为仍未改变；`main.cpp` 未接管新 engine；旧 `prepareDefaultScene(...)` 仍是主路径。

下一步建议：

- 在 `tools/sceneSetup` 增加一个可配置的 scene setup 入口，让 runtime composition root 可以选择旧 `prepareDefaultScene(...)` 或新 World-driven setup helper，并先只在验证模式下启用。

### 2026-05-25 Scene Setup Pipeline Entry

本轮继续上一节的下一步，新增非侵入式 scene setup pipeline 入口。目标不是替换旧场景，而是先把 runtime composition root 从“直接调用旧 `prepareDefaultScene(...)`”改为“通过一个可配置 pipeline 调用旧路径”，为后续切换 World-driven setup 留出明确入口。

新增与修改：

- 新增 `tools/sceneSetup/SceneSetupPipeline.h/.cpp`。
- `SceneSetupPipelineConfig` 当前包含：
  - `useLegacyDefaultScene`：默认 `true`，保持现有旧场景准备路径。
  - `addWorldDrivenProbe`：默认 `false`，可在后续验证模式中把 World-driven probe 追加到目标 scene。
- `SceneSetupPipelineResult` 当前记录旧路径是否执行、World probe 是否请求，以及 `WorldDrivenSceneProbeStats`。
- `RuntimeScenePreparer::prepare(...)` 不再直接调用 `GL_SCENE::prepareDefaultScene(...)`，而是调用 `GL_SCENE::prepareScene(sceneSetupContext, config.sceneSetupPipeline)`。
- `RuntimeScenePrepareConfig` 新增 `sceneSetupPipeline` 字段，默认配置等价于旧行为。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 pipeline 源文件和头文件。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes showcase-spheres,engine-world-scene-probe -DiscardCaptures`：构建通过；`SceneSetupPipeline.cpp` 参与编译；`showcase-spheres` 和 `engine-world-scene-probe` 均通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 30 个 PBR verification mode 全部通过。

结论：

- Runtime composition root 已经有第一层 scene setup pipeline seam。
- 默认行为仍保持旧 `prepareDefaultScene(...)`，没有改 `main.cpp`，没有接管 renderer，也没有删除旧 scene path。
- 后续可以在验证配置中启用 `addWorldDrivenProbe` 或逐步新增真正的 World-driven scene preset，而不需要再次改动 `RuntimeScenePreparer` 的调用结构。

下一步建议：

- 把 `SceneSetupPipelineConfig` 接入 runtime verification args/profile，让某个验证模式通过 pipeline 配置追加 World-driven probe，而不是在 `RuntimePBRVerification::addVerificationSceneProbes(...)` 后置追加。

### 2026-05-25 Scene Setup Pipeline Verification Integration

本轮完成上一节的下一步：`engine-world-scene-probe` 不再由 `RuntimePBRVerification::addVerificationSceneProbes(...)` 后置追加，而是通过 `SceneSetupPipelineConfig` 在 scene prepare 阶段注入。

新增与修改：

- `RuntimeApplicationShell::makeScenePrepareConfig()` 会把 `pbrVerification.enableEngineWorldSceneProbe` 映射为 `sceneSetupPipeline.addWorldDrivenProbe`。
- `RuntimeScenePreparer::prepare(...)` 在 `GL_SCENE::prepareScene(...)` 返回后，统一输出 `Engine world scene probe stats`。
- `RuntimePBRVerification::addVerificationSceneProbes(...)` 移除了 World-driven probe 的后置添加逻辑，只保留 PBR 透明、emissive、material IBL、alpha mask、import、texture set、showcase spheres 等 PBR 专用 probe。
- `RuntimePBRVerification` 仍保留 `enableEngineWorldSceneProbe` 的 profile 说明和 `engineWorldProbeMeshes` 场景统计，用于验证 pipeline 注入后的结果。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-probe -DiscardCaptures`：构建通过；`RuntimeApplicationShell.cpp`、`RuntimePBRVerification.cpp`、`RuntimeScenePreparer.cpp` 参与编译；`engineWorldProbeMeshes=1`；`pbrGBufferDrawCalls=26`；`pbrDeferredLightingDrawCalls=1`。
- `out/pbr_verify_engine-world-scene-probe.log` 中 `Engine world scene probe stats` 只输出 1 次，确认没有 pipeline 与 verification 后置重复添加。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 30 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- World-driven scene setup 已正式进入 runtime scene prepare 阶段，而不是 verification 后置阶段。
- `RuntimeScenePreparer` 已成为 legacy scene path 与 World-driven setup 的实际组合点。
- 当前默认运行仍保持旧 scene path，只有 `--verify-engine-world-scene-probe` 会通过 pipeline 追加 World-driven probe。

下一步建议：

- 在 `SceneSetupPipeline` 中新增一个真正的 World-driven minimal scene preset，不再只是追加单个 probe；该 preset 仍可导出到旧 `Scene`，用于验证后续 runtime 可以逐步从 legacy default scene 切到 World-first scene composition。

### 2026-05-25 World Driven Minimal Scene Preset

本轮完成上一节的下一步：`SceneSetupPipeline` 现在不只支持在旧默认场景上追加一个 probe，也支持准备一个真正由 `World / Level / Actor / Component` 生成的最小 PBR 场景。

新增与修改：

- `SceneSetup.h/.cpp` 新增三个可组合入口：
  - `prepareSceneInfrastructure(...)`：创建 renderer、screen scene、world scene、frame targets、bloom、skybox、screen pass、IBL precompute 和 lights。
  - `prepareLegacyDefaultSceneContent(...)`：只添加旧房间内容。
  - `prepareConfiguredPBRPreview(...)`：只添加 profile 驱动的 PBR preview。
- `prepareDefaultScene(...)` 仍保留旧调用顺序，默认启动行为不变。
- `WorldDrivenSceneSetup.h/.cpp` 新增 `addEngineWorldMinimalScene(...)` 与 `formatEngineWorldMinimalSceneStats(...)`。
- World-driven minimal scene 当前创建 1 个 root `LegacyObjectActor` 和 4 个 child `MeshActor`，包括 matte、metallic、gloss 和 emissive 四个 PBR sphere。
- `SceneSetupPipelineConfig` 新增 `useWorldDrivenMinimalScene`；当 legacy default scene 关闭时，pipeline 会先准备基础设施，再导出 World-driven minimal scene。
- `RuntimePBRVerificationArgs.cpp` 新增 `--verify-engine-world-minimal-scene`。
- `RuntimeApplicationShell::makeScenePrepareConfig()` 会在该模式下设置 `useWorldDrivenMinimalScene=true` 并关闭 `useLegacyDefaultScene`。
- `RuntimePBRVerification.cpp` 新增 `enableEngineWorldMinimalScene`、`engineWorldMinimalMeshes` 场景统计、minimal scene 专用 camera / light rig，并让该模式不再准备默认 5x5 PBR preview grid。
- `tools/verify_pbr.ps1` 新增 `engine-world-minimal-scene` mode，并断言：
  - `engineWorldMinimalMeshes >= 4`
  - `pbrPreviewMeshes == 0`
  - bridge stats 至少 5 个 Actor / 5 个 SceneComponent / 5 个导出对象 / 4 个导出 mesh / 1 个 root / 4 条 attachment
  - `pbrGBufferDrawCalls >= 4`
  - deferred lighting pass 正常绘制

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-probe,engine-world-minimal-scene -DiscardCaptures`：构建通过；`SceneSetup.cpp`、`SceneSetupPipeline.cpp`、`WorldDrivenSceneSetup.cpp`、`RuntimeApplicationShell.cpp`、`RuntimePBRVerification.cpp`、`RuntimePBRVerificationArgs.cpp`、`RuntimeScenePreparer.cpp` 参与编译；两个重点 mode 均通过。
- `out/pbr_verify_engine-world-minimal-scene.log` 中 `Engine world minimal scene stats` 输出为 `actors=5`、`sceneComponents=5`、`exportedObjects=5`、`exportedMeshes=4`、`sceneRootObjects=1`、`objectAttachments=4`。
- minimal scene 的 `PBR verification scene stats` 输出 `objects=6`、`meshes=4`、`pbrMeshes=4`、`pbrPreviewMeshes=0`、`engineWorldMinimalMeshes=4`、`iblReady=yes`。
- minimal scene 的 renderer stats 输出 `pbrDepthPrepassDrawCalls=4`、`pbrGBufferDrawCalls=4`、`pbrDeferredLightingDrawCalls=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- `SceneSetupPipeline` 已经具备从 legacy default scene 切到 World-first scene composition 的第一个可验证开关。
- 当前仍未修改 `main.cpp`，默认运行仍保持旧 scene path。
- 这个 minimal scene 仍通过 exporter 回填旧 `GLframework::Scene`，说明 renderer 尚未被新 `World` 直接驱动。

下一步建议：

- 让 `AppRuntimeContext` 或新的 runtime composition root 持有一个持久 `GLengine::World`，把 World-driven scene 从“局部临时构造后导出”升级为“runtime 状态的一部分”，为后续 Editor hierarchy 切到 `World / Level / Actor` 做准备。

### 2026-05-25 Runtime Persistent World Ownership

本轮完成上一节的下一步：World-driven scene 不再只存在于 `WorldDrivenSceneSetup` 的局部变量中，runtime context 现在可以持有准备阶段生成的 `GLengine::World`。

新增与修改：

- `AppRuntimeContext` 新增 `std::shared_ptr<GLengine::World> engineWorld`。
- `SetupContext` 新增 `engineWorld` 引用，`RuntimeScenePreparer::makeSceneSetupContext(...)` 会把 `context.engineWorld` 传入 scene setup pipeline。
- `SceneSetupPipeline.cpp` 在 `useWorldDrivenMinimalScene` 或 `addWorldDrivenProbe` 启用时创建/重置 `context.engineWorld`，再把该 World 传给 World-driven scene setup helper。
- `WorldDrivenSceneSetup.h/.cpp` 为 `addEngineWorldSceneProbe(...)` 和 `addEngineWorldMinimalScene(...)` 新增接收 `GLengine::World&` 的重载；旧的无 World 参数函数保留为兼容包装。
- `RuntimePBRVerification.cpp` 的 scene stats 新增 `runtimeWorldActors`，用于证明 Actor 保留在 runtime context 的 `engineWorld` 中，而不仅仅被导出到旧 `Scene`。
- `tools/verify_pbr.ps1` 对 `engine-world-scene-probe` 和 `engine-world-minimal-scene` 新增 `runtimeWorldActors` 断言。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-probe,engine-world-minimal-scene -DiscardCaptures`：构建通过；`AppRuntimeContext.h` 变更触发 runtime 相关编译；两个重点 mode 均通过。
- `engine-world-scene-probe` scene stats 输出 `runtimeWorldActors=2`，同时保持 `engineWorldProbeMeshes=1`、`pbrGBufferDrawCalls=26`、`pbrDeferredLightingDrawCalls=1`。
- `engine-world-minimal-scene` scene stats 输出 `runtimeWorldActors=5`、`engineWorldMinimalMeshes=4`、`pbrPreviewMeshes=0`，renderer stats 输出 `pbrGBufferDrawCalls=4`、`pbrDeferredLightingDrawCalls=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- World-driven scene 已经从“临时构造后立即导出”升级为“runtime context 持有 World，再导出给旧 renderer”。
- 当前旧默认 verification mode 中 `runtimeWorldActors=0`，说明默认旧 scene path 仍未被强行迁移。
- renderer 仍通过旧 `Scene` 消费对象；`engineWorld` 当前是 runtime/editor 后续迁移的状态源，不是 renderer 的直接输入源。

下一步建议：

- 将 `engineWorld` 暴露给 Editor hierarchy，先以只读 World / Level / Actor 树显示和选择为目标，再把 inspector 从旧 `Object` 逐步扩展到 Actor / Component。

### 2026-05-25 Editor Engine World Hierarchy Read Only

本轮完成上一节的下一步：runtime 持有的 `engineWorld` 已经暴露给 Editor hierarchy，当前以只读 World / Level / Actor 树显示和 Actor 选择为目标。

新增与修改：

- `EditorPanelContext` 新增 `engineWorld`，`RuntimeEditorPanelCoordinator::makeEditorPanelContext(...)` 会把 `AppRuntimeContext::engineWorld` 传给 Editor panels。
- `SelectionContext` 新增 `SelectionKind::Actor` 与 `selectedActor`，并新增 `getSelectedActor(...)` / `selectActor(...)`。
- `drawHierarchyPanel(...)` 新增 `Engine World` 分组，用于显示 runtime `World -> Persistent Level -> Actor -> Component` 树。
- Actor hierarchy 节点支持选中高亮；Component 节点当前点击后选择其 owning Actor，避免在没有 Component inspector 编辑模型前引入不完整的可编辑状态。
- `drawSelectionInspectorPanel(...)` 已接入 Actor inspector，当前只读展示 Actor name / type / object id / world / level / components，以及 root `SceneComponent` 的 relative transform 和 attachment 数量。
- 当前实现不改变旧 `Scenes / Lights / Cameras` hierarchy，也不改变旧 `Object / Light / Camera / Shadow` inspector 编辑路径。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimeEditorPanelCoordinator.cpp` 与 `EditorPanels.cpp` 参与编译；`runtimeWorldActors=5`、`engineWorldMinimalMeshes=4`、`pbrPreviewMeshes=0`、`pbrGBufferDrawCalls=4`、`pbrDeferredLightingDrawCalls=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- Editor 已经可以在 runtime 侧并行观察旧 `Scene / Object` tree 和新 `World / Level / Actor` tree。
- 当前 Actor inspector 是只读的，避免在 Component schema、undo/redo 和 property binding 未定型前过早引入可编辑状态。
- 默认旧 scene path 中 `engineWorld` 仍可能为空；World-driven verification modes 会填充 `engineWorld`，这是当前可验证入口。

下一步建议：

- 让默认旧 scene path 也生成一个 read-only `engineWorld` mirror，或优先推进 Actor / Component property inspector 的可编辑 schema。前者更利于 Editor 统一观察，后者更利于后续真正编辑 World-driven 场景。

### 2026-05-25 Legacy Scene Engine World Mirror

本轮继续完成上一节建议中的第一项：默认旧 scene path 现在也会生成只读 `engineWorld` mirror，普通 legacy scene 不再只能在旧 Object hierarchy 中观察。

新增与修改：

- `SceneSetupPipelineConfig` 新增 `mirrorLegacySceneToEngineWorld`，默认开启。
- `SceneSetupPipelineResult` 新增 `legacyWorldMirrorPrepared` 与 `legacyWorldMirrorStats`。
- `SceneSetupPipeline.cpp` 在纯 legacy default scene 路径下调用 `LegacySceneWorldBuilder::importScene(...)`，把已准备好的 `sceneOffScreen` 导入 `AppRuntimeContext::engineWorld`。
- mirror 逻辑明确排除 `useWorldDrivenMinimalScene` 和 `addWorldDrivenProbe`，避免 World-driven 模式把旧 scene mirror 再次导出回 renderer，造成重复对象。
- `RuntimeScenePreparer` 新增 `Engine world legacy mirror stats` 日志输出。
- `tools/verify_pbr.ps1` 在 `forward` mode 中新增 `ExpectLegacyWorldMirror` 断言，并在 summary 中输出 legacy mirror / engine probe / engine minimal stats。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene -DiscardCaptures`：构建通过；`SceneSetupPipeline.cpp` 与 `RuntimeScenePreparer.cpp` 参与编译。
- `forward` scene stats 输出 `runtimeWorldActors=33`；legacy mirror stats 输出 `visitedObjects=33`、`actors=33`、`meshActors=32`、`legacyObjectActors=1`、`componentAttachments=32`。
- `engine-world-scene-probe` 仍输出 `runtimeWorldActors=2` 和原本 probe bridge stats，确认没有把 legacy mirror 混入该 World-driven mode。
- `engine-world-minimal-scene` 仍输出 `runtimeWorldActors=5`、`engineWorldMinimalMeshes=4`、`pbrPreviewMeshes=0`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- 普通 legacy default scene 现在也有 runtime `engineWorld`，Editor 的 `Engine World` hierarchy 在默认路径中不再为空。
- 这个 mirror 是 read-only bridge，不改变 renderer 输入；renderer 仍消费旧 `GLframework::Scene`。
- PBR verification 后置添加的 probe 当前不会进入 legacy mirror，因为 mirror 发生在 scene prepare 阶段。这符合 runtime 默认观察目标，但后续如果要编辑 verification probe，需要把 probe 也迁入 scene setup pipeline。

下一步建议：

- 进入 Actor / Component property schema：先定义只读/可写字段描述、编辑提交路径和未来 undo/redo 边界，再让 Actor inspector 从手写只读 UI 迁移为 schema-driven UI。

### 2026-05-25 Actor Component Property Schema Inspector

本轮完成上一节的下一步中的第一段：Actor inspector 已从手写只读 ImGui 行迁移到已有 `PropertyBuilder / drawProperties(...)` schema 体系。

新增与修改：

- `PropertyDescriptor` 新增 `readOnly` 标记。
- `PropertyBuilder` 新增 `addReadOnlyFloat(...)`、`addReadOnlyBool(...)`、`addReadOnlyInt(...)`、`addReadOnlyVec3(...)` 和 `addReadOnlyString(...)`。
- `drawProperties(...)` 对只读 float / bool / int / vec3 / color3 / string 使用文本展示，对可写字段继续走原有 ImGui 控件。
- `EditorPanels.cpp` 新增 Actor schema builder：Actor name / type / object id / world / level / component count 现在由 schema 描述。
- `EditorPanels.cpp` 新增 Component schema builder：Component name / type / owner / active / tick、`SceneComponent` relative transform、parent / children，以及 Mesh / Light / Camera / Legacy Object adapter 绑定信息现在由 schema 描述。
- Actor inspector 当前仍保持只读，不对 `SceneComponent::setRelativeTransform(...)` 做写回。原因是当前 legacy mirror 是 read-only bridge，直接编辑 Actor transform 不会自动同步 renderer 中的旧 Object，会造成 UI 上“改了但画面没变”的假编辑。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；`EditorPanels.cpp`、`DebugControllerPanel.cpp` 和 profile/property schema 相关编译单元参与编译；`forward` 输出 `runtimeWorldActors=33`；`engine-world-minimal-scene` 输出 `runtimeWorldActors=5`、`engineWorldMinimalMeshes=4`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- Actor / Component inspector 已经开始从“为每种对象手写 UI”转向“字段描述驱动 UI”。
- 当前 schema 已能统一表达只读字段和未来可写字段，复用了之前 PBR profile / material inspector 已经在使用的 property infrastructure。
- 编辑功能仍未开启，这是刻意边界：需要先确定编辑提交后如何同步 Actor、adapter 持有的旧 Object、renderer scene，以及未来 undo/redo。

下一步建议：

- 增加 Actor / Component edit commit bridge：先只允许编辑 World-driven scene 中 `SceneComponent` relative transform，并在提交时同步 adapter 中的旧 `Object` transform；legacy mirror 继续只读。

### 2026-05-25 Actor Component Edit Commit Bridge

本轮完成上一节的下一步：Actor / Component inspector 不再只是只读 schema 展示，World-driven scene 中可以编辑具备旧 `Object` adapter 的 `SceneComponent` relative transform，并在提交时同步 renderer 仍在使用的旧对象 transform。

新增与修改：

- `AppRuntimeContext` 新增 `engineWorldEditable`，用于区分当前 runtime `engineWorld` 是 World-driven 可编辑源，还是 legacy scene 的只读 mirror。
- `SetupContext` 传递 `engineWorldEditable`，`SceneSetupPipeline` 在每轮 scene prepare 开始时默认设为 `false`。
- legacy mirror 路径继续把 `engineWorldEditable=false`，避免默认旧 scene path 变成“UI 上可改但 renderer 状态源不明确”的假编辑。
- `useWorldDrivenMinimalScene` 与 `addWorldDrivenProbe` 路径会把 `engineWorldEditable=true`，因为这些 Actor 是 scene setup 的实际 World 源，并且通过 adapter 导出到旧 renderer。
- `EditorPanelContext` 传递 `engineWorldEditable`，Actor / Component schema 可以根据当前 World 来源决定字段是否可写。
- `EditorPanels.cpp` 新增 legacy adapter object 查找和 transform 同步 helper：`MeshComponent`、`LightComponent`、`LegacyObjectComponent` 可同步到旧 `Object`；`CameraComponent` 当前没有旧 `Object`，因此仍保持只读。
- `SceneComponent` 的 `Relative Location`、`Relative Rotation`、`Relative Scale` 在满足 `engineWorldEditable && hasLegacyObjectAdapter` 时使用可写 `PropertyBuilder::addVec3(...)`，setter 会同时更新 `SceneComponent::setRelativeTransform(...)` 和 adapter 旧 `Object` 的 position / angle / scale。
- schema 中新增 `Editable World` 与 `Transform Editable` 只读状态字段，方便在 inspector 中判断当前是否真的允许提交编辑。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimeEditorPanelCoordinator.cpp`、`RuntimeScenePreparer.cpp`、`EditorPanels.cpp`、`SceneSetupPipeline.cpp` 参与编译。
- `forward` 输出 `runtimeWorldActors=33`，legacy mirror stats 输出 `visitedObjects=33`、`actors=33`、`meshActors=32`、`componentAttachments=32`，该路径保持 read-only mirror。
- `engine-world-minimal-scene` 输出 `runtimeWorldActors=5`、`engineWorldMinimalMeshes=4`、`pbrPreviewMeshes=0`，renderer stats 输出 `pbrGBufferDrawCalls=4`、`pbrDeferredLightingDrawCalls=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- Actor / Component inspector 已经具备第一条可写提交路径，但编辑范围被刻意限制在 World-driven scene 和有旧对象 adapter 的 SceneComponent。
- 默认 legacy scene mirror 继续只读，避免和旧 renderer object ownership 边界冲突。
- 当前验证覆盖构建和自动 PBR 回归，尚未做人工 UI 拖动/输入 smoke；如果后续要把它作为编辑器功能验收，需要增加交互式或脚本化 UI 验证。

下一步建议：

- 增加 Component 直接选择：让 hierarchy 中的 Component 节点不再退回选择 owning Actor，而是进入 `SelectionKind::Component`，并给 Component inspector 一个独立入口。之后再补 undo/redo 或 edit transaction 边界，避免可写字段继续扩散成无记录的即时修改。

### 2026-05-25 Component Direct Selection Inspector

本轮完成上一节的下一步：Engine World hierarchy 中的 Component 节点现在可以直接被选中，并进入专用 Component inspector，不再退回选择 owning Actor。

新增与修改：

- `SelectionKind` 新增 `Component`。
- `SelectionContext` 新增 `selectedComponent`，用于保存当前选中的 `GLengine::ActorComponent*`。
- `EditorPanels.h/.cpp` 新增 `getSelectedComponent(...)` 和 `selectComponent(...)`。
- `selectObject(...)`、`selectShadow(...)`、`selectCamera(...)`、`selectActor(...)` 会清空 `selectedComponent`，保证同一时间只有一种 selection target 生效。
- `renderActorHierarchyNode(...)` 中的 Component leaf 会根据 `SelectionKind::Component` 做 selected 高亮。
- 点击 Component leaf 后调用 `selectComponent(...)`，inspector 优先渲染 `renderComponentInspector(...)`。
- `renderComponentInspector(...)` 复用上一轮的 `buildComponentPropertySchema(...)`，因此 Component 独立选择后仍保留 `Editable World` / `Transform Editable` 判定和 World-driven transform 写回路径。
- Actor inspector 仍保留组件折叠展示，方便查看整个 Actor；Component 专用 inspector 则用于后续编辑器重定向和事务边界。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-minimal-scene -DiscardCaptures`：构建通过；`EditorPanels.cpp` 参与编译；`engine-world-minimal-scene` 通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- Editor selection 已从 Object / Light / Camera / Shadow / Actor 扩展到 Component。
- hierarchy 和 inspector 的责任边界更清晰：Actor 负责整体概览，Component 负责组件属性与后续可写编辑入口。
- 当前仍未引入 undo/redo，Component transform 写回仍是即时提交。

下一步建议：

- 增加最小 edit transaction 边界：把 `SceneComponent` transform setter 从直接写回升级为可记录的编辑操作，至少记录 target、field、before、after，为后续 undo/redo、dirty 标记和保存场景做准备。

### 2026-05-25 Minimal Edit Transaction Boundary

本轮完成上一节的下一步：`SceneComponent` transform 编辑不再只是无记录的即时写回，现在会进入一个最小 edit transaction log，记录 target、field、before、after。

新增与修改：

- `EditorPanels.h` 新增 `EditTransactionRecord` 与 `EditTransactionLog`。
- `EditTransactionRecord` 当前记录 `sequence`、`targetLabel`、`targetType`、`targetObjectId`、`targetAddress`、`field`、`beforeValue`、`afterValue`。
- `EditTransactionLog` 当前提供 `recordVec3(...)`、`getRecords()`、`getLatestRecord()` 和 `isDirty()`；本轮只做记录和 dirty 标记，不实现 undo/redo。
- `RuntimeApplicationShell` 新增持久 `mEditorEditTransactions`，生命周期跟随 runtime shell。
- `RuntimeEditorPanelCoordinator` 会把 `mEditorEditTransactions` 传入 `EditorPanelContext`。
- `EditorPanelContext` 新增 `editTransactions` 指针，供 inspector property setter 写入 transaction。
- `setSceneComponentLocation(...)`、`setSceneComponentRotation(...)`、`setSceneComponentScale(...)` 会先读取 before，再更新 `SceneComponent` 和 adapter 旧 `Object`，最后记录对应字段的 before / after。
- Actor / Component inspector 底部新增 `Edit Transactions` 摘要，显示 dirty、record count、最近一次编辑的 target.field 以及 before / after。
- 记录逻辑会跳过 before / after 完全相同的输入，避免无变化写入 transaction。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimeApplicationShell.cpp`、`RuntimeEditorPanelCoordinator.cpp`、`EditorPanels.cpp` 参与编译；`engine-world-minimal-scene` 通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- 当前编辑链路已经具备最小可审计边界：每次 transform 写回都能留下 target / field / before / after。
- 这仍不是完整 undo/redo 系统；下一步可以基于这些 records 增加 undo 应用路径，或先补 dirty marker / save scene boundary。
- 当前仍未做人工 UI 输入 smoke，自动验证覆盖构建和渲染回归。

下一步建议：

- 增加最小 undo apply：先只支持撤销最近一条 `SceneComponent` Vec3 transform transaction，并复用现有 transform sync helper 写回 `SceneComponent` 与旧 `Object`；暂不做 redo stack 和跨对象批量事务。

### 2026-05-25 Minimal SceneComponent Undo Apply

本轮继续完成上一节的下一步：edit transaction log 现在支持撤销最近一条 `SceneComponent` Vec3 transform transaction。

新增与修改：

- `EditTransactionRecord` 新增 `sceneComponent` 指针，用于最小 undo apply 直接定位目标组件。
- `EditTransactionLog` 新增 `popLatestRecord()`，撤销成功后弹出最近记录，并根据剩余记录更新 dirty 状态。
- `recordSceneComponentVec3Edit(...)` 记录 transaction 时会保存对应 `SceneComponent*`。
- `EditorPanels.cpp` 新增 `undoLatestSceneComponentVec3Edit(...)`，按 `field` 将 before 值写回 `Relative Location`、`Relative Rotation` 或 `Relative Scale`。
- undo 写回复用现有 `setSceneComponentLocation(...)` / `setSceneComponentRotation(...)` / `setSceneComponentScale(...)`，并传入 `nullptr` transaction log，避免 undo 自身再次生成新 transaction。
- `Edit Transactions` inspector 摘要在存在可撤销记录时显示 `Undo Latest Transform` 按钮。
- 当前只支持撤销最近一条 SceneComponent Vec3 transform；不支持 redo stack、不支持事务合并、不处理目标组件被销毁后的复杂生命周期。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimeApplicationShell.cpp`、`RuntimeEditorPanelCoordinator.cpp`、`EditorPanels.cpp` 参与编译；`engine-world-minimal-scene` 通过。

结论：

- Actor / Component transform 编辑链路现在从“可写”推进到“可记录并可撤销最近一步”。
- 这仍是最小 undo，不是完整编辑器事务系统；但它已经建立了后续 redo、dirty/save、事务合并的必要接口边界。

下一步建议：

- 增加 redo stack 或 dirty/save boundary 二选一。更符合当前引擎化目标的是先做 dirty/save boundary：把 `EditTransactionLog::isDirty()` 变成可观察状态，并在 inspector 提供 `Mark Saved / Clear Transactions`，为后续场景保存做准备。

### 2026-05-25 Dirty Save Transaction Boundary

本轮完成上一节的下一步：edit transaction log 现在具备明确的 dirty/save 观察与清理边界，inspector 可以手动标记已保存或清空 transaction history。

新增与修改：

- `EditTransactionLog` 新增 `markDirty()`、`markSaved()` 和 `clear()`。
- `Undo Latest Transform` 执行成功后会重新 `markDirty()`，因为 undo 本身也是一次 scene state 变更。
- `Edit Transactions` inspector 摘要新增 `Mark Saved` 按钮，用于只清除 dirty 标记并保留 transaction history。
- `Edit Transactions` inspector 摘要新增 `Clear Transactions` 按钮，用于清空历史记录并将 dirty 置为 `false`。
- `Mark Saved` 当前只是编辑器状态边界，不执行磁盘写入；这一步的目标是为后续 scene persistence / save scene 边界提供可观察状态。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimeApplicationShell.cpp`、`RuntimeEditorPanelCoordinator.cpp`、`RuntimePBRVerificationArgs.cpp`、`main.cpp`、`EditorPanels.cpp` 参与编译；`engine-world-minimal-scene` 通过，输出 `runtimeWorldActors=5`、`engineWorldMinimalMeshes=4`、`pbrGBufferDrawCalls=4`、`pbrDeferredLightingDrawCalls=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过；`forward` legacy mirror 仍输出 `runtimeWorldActors=33`，`engine-world-scene-probe` 仍输出 `runtimeWorldActors=2`，`engine-world-minimal-scene` 仍输出 `runtimeWorldActors=5`。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- Actor / Component transform 编辑链路现在具备最小“编辑、记录、撤销、dirty、mark saved、clear history”闭环。
- 这仍不是完整编辑器事务系统：没有 redo stack、事务合并、跨对象 transaction batch，也没有真正的场景文件保存。
- 当前更适合继续推进 scene persistence boundary，而不是继续在 undo/redo 上扩展复杂度。

下一步建议：

- 增加 scene persistence boundary：先保存 World-driven scene 中可编辑 `SceneComponent` transform snapshot，格式可以是小型 JSON/INI，不直接尝试完整 asset serialization。

### 2026-05-25 Scene Transform Snapshot Persistence Boundary

本轮完成上一节的下一步：World-driven scene 现在具备第一条最小 persistence boundary，可以把 runtime `World` 中的 `SceneComponent` transform 写成 snapshot 文件。

新增与修改：

- 新增 `tools/editor/SceneTransformSnapshot.h/.cpp`。
- `saveSceneTransformSnapshot(...)` 会遍历 `World -> Persistent Level -> Actor -> SceneComponent`，写出 actor/component 名称、类型、object id、parent object id，以及 relative location / rotation / scale。
- 默认 snapshot 路径为 `out/engine_world_transform_snapshot.ini`，verification 使用 `out/engine_world_transform_snapshot.verification.ini`。
- `Edit Transactions` inspector 在可编辑 World-driven scene 下新增 `Save Transform Snapshot` 按钮；保存成功后会调用 `markSaved()`，保留 transaction history 但清除 dirty 标记。
- legacy scene mirror 仍然只读，inspector 会提示 read-only World mirror 不支持 transform snapshot 保存。
- `RuntimePBRVerification::reportPreparedScene(...)` 在 `engineWorldEditable` 为 `true` 时自动写出 transform snapshot，并输出 `Engine world transform snapshot stats`。
- `verify_pbr.ps1` 对 `engine-world-scene-probe` 和 `engine-world-minimal-scene` 增加 snapshot 断言：要求 `saved=yes`、actor 数量大于 0、SceneComponent 数量大于 0，且 snapshot 文件真实存在。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新文件。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimePBRVerification.cpp`、`EditorPanels.cpp`、`SceneTransformSnapshot.cpp` 参与编译；`engine-world-minimal-scene` 通过，snapshot 输出 `saved=yes, actors=5, sceneComponents=5`。
- 已检查 `out/engine_world_transform_snapshot.verification.ini`，文件包含 `schema=engine.world.sceneComponentTransform.v1`、World/Level 信息、`actorCount=5`、`sceneComponentCount=5` 和每个 SceneComponent 的 relative transform。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过；`engine-world-scene-probe` snapshot 输出 `actors=2, sceneComponents=2`，`engine-world-minimal-scene` snapshot 输出 `actors=5, sceneComponents=5`。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- 当前项目已经从“编辑器内存状态可 dirty/save 标记”推进到“可把 World-driven transform 状态落盘为 snapshot”。
- 这仍不是完整场景保存系统：没有 load/apply snapshot，没有 asset GUID，没有 prefab/level package，也没有处理 actor/component 删除和新增。
- 这一步建立的是后续 save/load scene 的最小文件格式和验证边界。

下一步建议：

- 增加 snapshot load/apply boundary：先读取 snapshot 并按 object id 或 actor/component path 回放 `SceneComponent` transform，再同步 adapter 旧对象；同时保留 legacy mirror 只读。

### 2026-05-25 Scene Transform Snapshot Load Apply Boundary

本轮完成上一节的下一步：World-driven scene 的 transform snapshot 现在不仅能保存，也能读取并回放到 runtime `World`。

新增与修改：

- `SceneTransformSnapshot.h/.cpp` 新增 `SceneTransformSnapshotApplyResult` 与 `applySceneTransformSnapshot(...)`。
- Snapshot apply 会读取 `engine.world.sceneComponentTransform.v1` schema，解析 `actorCount`、`sceneComponentCount` 和每个 `SceneComponent` 的 relative location / rotation / scale。
- 匹配策略为 object id 优先，actor index / SceneComponent index fallback；这允许同一 runtime session 走稳定 object id，同时为后续稳定 path id 留出迁移空间。
- Apply 成功后会调用 `SceneComponent::setRelativeTransform(...)`，并同步 `MeshComponent`、`LightComponent`、`LegacyObjectComponent` 持有的旧 `Object` transform；`CameraComponent` 仍没有旧 `Object` adapter。
- `Edit Transactions` inspector 在可编辑 World-driven scene 下新增 `Apply Transform Snapshot` 按钮；apply 成功后调用 `markSaved()`，表示当前内存状态已回到 snapshot 文件状态。
- `RuntimePBRVerification::reportPreparedScene(...)` 保存 snapshot 后会临时扰动第一个 `SceneComponent`，随后调用 `applySceneTransformSnapshot(...)` 回放 snapshot。
- `verify_pbr.ps1` 对 World-driven modes 增加 apply 断言：要求 `applied=yes`、`appliedSceneComponents>0`、`changedSceneComponents>0`、`verificationPerturbed=yes`，证明 apply 实际恢复了被扰动的 transform。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimePBRVerification.cpp`、`EditorPanels.cpp`、`SceneTransformSnapshot.cpp` 参与编译；`engine-world-minimal-scene` 通过，snapshot apply 输出 `applied=yes, matchedSceneComponents=5, appliedSceneComponents=5, changedSceneComponents=1, verificationPerturbed=yes`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过；`engine-world-scene-probe` 和 `engine-world-minimal-scene` 均通过 snapshot save/apply 断言。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- 当前已经具备最小 scene transform persistence 闭环：edit -> dirty -> save snapshot -> apply snapshot -> sync legacy adapter。
- 这仍不是完整 scene serialization。当前 snapshot 仍依赖 transient object id 和 actor/component index fallback，没有稳定 GUID/path、没有 actor/component 新增删除回放、没有自动启动加载策略。

下一步建议：

- 增加 stable scene path id：为 Actor / SceneComponent snapshot 写入稳定层级路径或显式 persistent id，降低 object id 变化和 actor 顺序变化对 load/apply 的影响。

### 2026-05-25 Stable Scene Path Id For Transform Snapshot

本轮完成上一节的下一步：transform snapshot 现在写入并优先使用 stable scene path id，减少对 transient object id 和 actor/component 顺序的依赖。

新增与修改：

- `SceneTransformSnapshot.cpp` 新增 stable path 生成逻辑。
- Actor stable path 由 `Level` 名称、Actor 类型、Actor 名称和同名同类型 occurrence 组成。
- SceneComponent stable path 由所属 Actor stable path、Component 类型、Component 名称和同名同类型 occurrence 组成。
- Snapshot 文件新增 `actor.N.stablePath` 和 `actor.N.sceneComponent.M.stablePath`。
- `applySceneTransformSnapshot(...)` 的匹配优先级改为 `stablePath -> objectId -> actor/component index`。
- `SceneTransformSnapshotApplyResult` 新增 `matchedByStablePathCount`、`matchedByObjectIdCount` 和 `matchedByIndexCount`。
- `RuntimePBRVerification` 的 snapshot apply stats 会输出 `matchedByStablePath`、`matchedByObjectId` 和 `matchedByIndex`。
- `verify_pbr.ps1` 增加 `matchedByStablePath>0` 断言，确保 apply 实际走 stable path 匹配，而不是继续只依赖 object id。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-minimal-scene -DiscardCaptures`：构建通过；`engine-world-minimal-scene` 输出 `matchedByStablePath=5, matchedByObjectId=0, matchedByIndex=0`。
- 已检查 `out/engine_world_transform_snapshot.verification.ini`，snapshot 文件包含 `actor.N.stablePath` 与 `actor.N.sceneComponent.M.stablePath`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过；`engine-world-scene-probe` 输出 `matchedByStablePath=2`，`engine-world-minimal-scene` 输出 `matchedByStablePath=5`。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- Snapshot load/apply 现在已经具备比 object id 更稳定的第一层匹配键。
- 当前 stable path 仍是名字/类型/occurrence 派生的路径，不是长期资产 GUID；如果用户重命名 Actor/Component 或大量重排同名对象，仍可能触发 fallback。
- 这一步解决的是“不要主要依赖 transient object id 和顺序”的短期问题，后续仍需要真正的 persistent id。

下一步建议：

- 增加 persistent id 字段：为 `EngineObject` 或 editor metadata 引入可序列化 id，并把 snapshot stable path 从派生路径升级为真正可持久化标识。

### 2026-05-25 Persistent Id For Transform Snapshot

本轮完成上一节的下一步：`EngineObject` 现在具备最小 persistent id 字段，World-driven scene preset 会写入确定性 id，transform snapshot apply 优先按 persistent id 匹配。

新增与修改：

- `EngineObject` 新增 `getPersistentId()` / `setPersistentId(...)`，保存一个可序列化字符串 id；`objectId` 仍保留为 runtime transient id。
- `WorldDrivenSceneSetup.cpp` 为 `engine-world-scene-probe` 和 `engine-world-minimal-scene` 的 `World / Level / Actor / SceneComponent` 分配确定性 persistent id。
- Snapshot 文件新增 `world.persistentId`、`level.persistentId`、`actor.N.persistentId` 和 `actor.N.sceneComponent.M.persistentId`。
- `applySceneTransformSnapshot(...)` 的匹配优先级升级为 `persistentId -> stablePath -> objectId -> actor/component index`。
- `SceneTransformSnapshotApplyResult` 新增 `matchedByPersistentIdCount`，verification stats 输出 `matchedByPersistentId`。
- Actor / Component schema inspector 新增只读 `Persistent ID` 字段，便于在 editor 中确认当前对象的长期标识。
- `verify_pbr.ps1` 对 World-driven snapshot 增加 `sceneComponent.0.persistentId` 文件断言，并要求 `matchedByPersistentId>0`。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-minimal-scene -DiscardCaptures`：构建通过；`engine-world-minimal-scene` 输出 `matchedByPersistentId=5, matchedByStablePath=0, matchedByObjectId=0, matchedByIndex=0`。
- 已检查 `out\engine_world_transform_snapshot.verification.ini`，snapshot 文件包含 `world.persistentId`、`level.persistentId`、Actor persistent id 和 SceneComponent persistent id。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过；`engine-world-scene-probe` 输出 `matchedByPersistentId=2`，`engine-world-minimal-scene` 输出 `matchedByPersistentId=5`。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- Snapshot load/apply 现在已经有真正的可序列化 persistent id 匹配层，短期内不再依赖 transient object id，也不优先依赖名字/类型/occurrence 派生 stable path。
- 当前 persistent id 覆盖范围仍是显式构造的 World-driven scene；legacy mirror/default scene 以及未来 import/prefab/level package 还没有统一 id 分配策略。
- `stablePath` 和 `objectId` 继续保留为兼容 fallback，但正常 World-driven snapshot 已经优先走 persistent id。

下一步建议：

- 制定并落地 broader persistent id policy：为 legacy mirror、imported asset、手动创建 Actor/Component 和未来 scene package 定义 id 生成/保存规则，避免 persistent id 只在两个 verification preset 中有效。

### 2026-05-25 Legacy Mirror Persistent Id Coverage

本轮继续推进 broader persistent id policy 的第一块：普通 legacy scene mirror 现在也会为导入出来的 `World / Level / Actor / SceneComponent` 分配 deterministic persistent id。

新增与修改：

- `LegacySceneImportStats` 新增 `actorsWithPersistentIds` 和 `sceneComponentsWithPersistentIds`，用于验证 legacy mirror 的 id 覆盖情况。
- `LegacySceneWorldBuilder::importScene(...)` 会在缺省情况下为 mirror world 写入 `world:legacy-scene-mirror`，为 persistent level 写入 `level:legacy-scene-mirror:persistent-level`。
- `LegacySceneWorldBuilder` 递归导入旧 `Object` 树时，会根据旧对象在树中的 type/name/occurrence path 派生 deterministic actor persistent id。
- 每个导入 actor 的 root `SceneComponent` 会获得对应 component persistent id，并继续保留原有 transform / attachment bridge。
- `formatLegacyWorldMirrorStats(...)` 输出 `persistentActors` 和 `persistentSceneComponents`。
- `verify_pbr.ps1` 的 `ExpectLegacyWorldMirror` 断言现在要求每个导入 actor 和 SceneComponent 都具备 persistent id。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward -DiscardCaptures`：构建通过；`LegacySceneWorldBuilder.cpp` 与 `SceneSetupPipeline.cpp` 参与编译；`forward` 输出 `persistentActors=33, persistentSceneComponents=33`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过；普通 legacy mirror 路径持续输出 persistent id 覆盖统计，`showcase-spheres` 也输出 `persistentActors=28, persistentSceneComponents=28`。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- persistent id 现在不再只覆盖两个 World-driven verification preset；普通旧场景 mirror 也进入了可观测的长期标识体系。
- 当前 legacy mirror id 仍是从旧对象树 type/name/occurrence path 派生，不是 asset GUID；如果旧场景对象重命名或大规模重排，id 仍可能变化。
- 这一层适合作为过渡 bridge 的 deterministic id，不应被误认为最终资产数据库 id。

下一步建议：

- 继续拆 persistent id policy：把 id 来源拆成 `PresetAssigned`、`LegacyMirrorDerived`、`ImportedAssetDerived`、`EditorCreatedGenerated` 等来源，后续 scene package 保存时必须保存实际 id，而不是每次重新派生。

### 2026-05-25 Persistent Id Policy Module

本轮把前两轮的 persistent id 生成规则集中到 `engine/PersistentIdPolicy`，让 id 来源不再散落在各个 scene setup / bridge 文件中。

新增与修改：

- 新增 `engine/PersistentIdPolicy.h/.cpp`。
- 新增 `PersistentIdSource`，当前定义 `PresetAssigned`、`LegacyMirrorDerived`、`ImportedAssetDerived` 和 `EditorCreatedGenerated`。
- 新增 `persistentIdSourceToken(...)`、`sanitizePersistentIdSegment(...)`、`makePersistentId(...)` 和 `makeIndexedPersistentIdPathSegment(...)`。
- persistent id 统一升级为 `objectKind:source:scope:path` 形式；例如 World-driven preset snapshot 当前写入 `world:preset:engine-world-minimal-scene` 和 `component:preset:engine-world-minimal-scene:matte`。
- `WorldDrivenSceneSetup.cpp` 改为通过 `PersistentIdPolicy` 生成 `preset` 来源 id，不再硬编码完整 persistent id 字符串。
- `LegacySceneWorldBuilder.cpp` 改为通过 `PersistentIdPolicy` 生成 `legacy-mirror` 来源 id，不再在本地维护 sanitize/path 规则。
- `formatLegacyWorldMirrorStats(...)` 新增 `persistentIdSource=legacy-mirror`，用于验证来源语义。
- `verify_pbr.ps1` 增加 source 级断言：legacy mirror stats 必须报告 `persistentIdSource=legacy-mirror`；World-driven snapshot 必须写出 `world:preset:` 和 `component:preset:` 前缀。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册 `PersistentIdPolicy` 新文件。

验证结果：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；`PersistentIdPolicy.cpp`、`LegacySceneWorldBuilder.cpp`、`SceneSetupPipeline.cpp`、`WorldDrivenSceneSetup.cpp` 参与编译；`forward` 输出 `persistentIdSource=legacy-mirror`；`engine-world-minimal-scene` snapshot apply 仍输出 `matchedByPersistentId=5`。
- 已检查 `out\engine_world_transform_snapshot.verification.ini`，World-driven snapshot 写入 `world:preset:engine-world-minimal-scene`、`level:preset:engine-world-minimal-scene:persistent-level` 和 `component:preset:...` id。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过；legacy mirror stats 均包含 `persistentIdSource=legacy-mirror`，World-driven snapshot apply 仍优先走 persistent id。
- `git diff --check`：无 whitespace error，仅有 LF/CRLF warning。

结论：

- persistent id 现在有集中策略模块，当前已覆盖 preset-assigned 和 legacy-mirror-derived 两类来源。
- 旧 snapshot 中的旧格式 persistent id 如果和新格式不一致，仍可通过 `stablePath` fallback 应用；新 snapshot 会写入带 source token 的 id。
- `ImportedAssetDerived` 和 `EditorCreatedGenerated` 目前只是策略枚举，尚未接入具体导入器或编辑器创建流程。

下一步建议：

- 继续接 `ImportedAssetDerived`：从 Assimp import / PBR import probe 中提取 asset path + node/material/mesh path，给导入生成的 Actor/Component 或未来 asset wrapper 分配可追踪 persistent id。

### 2026-05-25 Imported Asset Persistent Id Coverage

本轮继续推进 broader persistent id policy 的第二块：PBR import probe 现在不只把 Assimp 导入资产追加到旧 `Scene` 中渲染，也会把同一导入对象树导入 runtime `engineWorld`，并使用 `ImportedAssetDerived` 来源生成 persistent id。

新增与修改：

- `LegacySceneWorldBuilder` 新增 `LegacySceneImportOptions`，允许调用方指定 `persistentIdSource` 与 `persistentIdScope`。
- `LegacySceneWorldBuilder::importScene(...)` 与 `importObjectTree(...)` 增加 options overload；默认 legacy mirror 行为保持 `LegacyMirrorDerived / legacy-scene-mirror`。
- `RuntimePBRVerification` 的 `--verify-pbr-import` 在 `AssimpLoader::loadPBR("fbx/test/test.fbx", ...)` 成功后，会把 `"PBR Imported Asset Probe"` 对象树导入 `context.engineWorld->getPersistentLevel()`。
- 导入资产 probe 使用 `PersistentIdSource::ImportedAssetDerived` 和 `fbx-test-test-fbx` scope，输出 `PBR imported asset engine world import stats`。
- `verify_pbr.ps1` 的 `import` mode 新增 `ExpectImportedAssetProbe`，断言旧 render scene 中有 imported PBR mesh、runtime `engineWorld` 有 Actor、导入资产 stats 报告 `persistentIdSource=imported-asset`，并要求每个导入 Actor / SceneComponent 都具备 persistent id。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes import -DiscardCaptures`：构建通过；`RuntimePBRVerification.cpp` 与 `LegacySceneWorldBuilder.cpp` 参与编译；`import` 输出 `pbrImportedMeshes=1`、`runtimeWorldActors=37`、`persistentActors=4`、`persistentSceneComponents=4`、`persistentIdSource=imported-asset`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 31 个 PBR verification mode 全部通过；`import` 汇总输出 `importedAssetWorldImport`，其中 `actors=4`、`meshActors=1`、`persistentActors=4`、`persistentSceneComponents=4`、`persistentIdSource=imported-asset`。

结论：

- `ImportedAssetDerived` 已从枚举占位进入真实验证路径，当前覆盖 Assimp PBR import probe 到 runtime `engineWorld` 的 bridge。
- 当前 scope 仍使用验证资产路径派生的固定字符串，尚未形成完整 asset database GUID；这足够作为 bridge 验证，不应被误认为最终资产系统。

下一步建议：

- 继续接 `EditorCreatedGenerated`：为编辑器中新建 Actor/Component 的入口分配 persistent id，并把生成规则纳入 transaction / snapshot 验证。

### 2026-05-25 Editor Created Persistent Id Coverage

本轮继续推进 broader persistent id policy 的第三块：编辑器现在有最小“创建空 Actor”入口，并通过同一条逻辑在 verification 中验证 `EditorCreatedGenerated` persistent id。

新增与修改：

- 新增 `tools/editor/EditorWorldActions.h/.cpp`，集中提供 `createEditorEmptyActor(...)`、`countEditorCreatedActors(...)` 和 `countEditorCreatedSceneComponents(...)`。
- `createEditorEmptyActor(...)` 会在可编辑 `World` 的 persistent level 中创建普通 `Actor` 和 root `SceneComponent`，并分配 `actor:editor-created:runtime-editor:actor-N` 与 `component:editor-created:runtime-editor:actor-N:root`。
- `EditorPanels` 的 `Edit Transactions` 区域新增 `Create Empty Actor` 按钮；创建成功后写入 lifecycle transaction，标记 dirty，但暂不实现 create undo。
- `RuntimePBRVerification` 新增 `enableEngineWorldEditorCreate` probe，非交互调用同一 `EditorWorldActions` 创建逻辑，并输出 `Engine world editor create stats`。
- `verify_pbr.ps1` 新增 `engine-world-editor-create` mode，基于 World-driven minimal scene 运行，断言 runtime World 中新增 Actor、`editor-created` Actor / SceneComponent persistent id、snapshot 文件中写入 editor-created id，并保持 snapshot apply 走 persistent id。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册 `EditorWorldActions` 新文件。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-editor-create -DiscardCaptures`：构建通过；`EditorWorldActions.cpp` 参与编译；scene stats 输出 `runtimeWorldActors=6`；editor create stats 输出 `created=yes`、`editorCreatedActors=1`、`editorCreatedSceneComponents=1`、`actorPersistentId=actor:editor-created:runtime-editor:actor-0`、`componentPersistentId=component:editor-created:runtime-editor:actor-0:root`；snapshot 输出 `actors=6`、`sceneComponents=6`、`matchedByPersistentId=6`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 32 个 verification mode 已通过；新增 `engine-world-editor-create` mode 在全量默认验证中继续输出 `runtimeWorldActors=6`、`editorCreatedActors=1`、`editorCreatedSceneComponents=1`、`matchedByPersistentId=6`。

结论：

- persistent id 当前已覆盖 `preset`、`legacy-mirror`、`imported-asset` 和 `editor-created` 四类来源的最小验证路径。
- editor-created Actor 当前是无 renderer adapter 的空 Actor，因此不会改变旧 renderer 场景内容；这是符合目标的第一步，因为本轮验证的是编辑器对象生命周期与长期标识，不是渲染资产创建。
- create transaction 目前只记录 lifecycle 与 dirty 状态，不支持撤销创建；后续需要把 create/delete 纳入 undo/redo 栈。

下一步建议：

- 开始 scene package / save boundary：把 `World / Level / Actor / Component` 的 persistent id、transform 和最小层级结构保存为独立 scene package，再从 package 恢复到 runtime `World`。

### 2026-05-25 Scene Package Save/Load Boundary

本轮按上一节建议进入 scene package / save boundary。目标不是继续扩展 PBR，而是给 engine `World` 建立第一条对象结构级保存/加载边界。

新增与修改：

- 新增 `engine/ScenePackage.h/.cpp`，提供 `saveScenePackage(...)`、`loadScenePackage(...)` 和 `defaultScenePackagePath()`。
- scene package 格式为 `engine.world.scenePackage.v1`，当前保存 `World`、persistent level、Actor、SceneComponent 的 name、persistent id、root component、parent component、relative transform。
- `loadScenePackage(...)` 会创建新的 `World`，按 package 恢复普通 `Actor` + `SceneComponent` 层级，并回填 persistent id、root component、parent attach 和 transform。
- `RuntimePBRVerification` 新增 scene package roundtrip probe，保存 `out/engine_world_scene_package.verification.ini` 后立即加载，并输出 `Engine world scene package stats`。
- 新增 CLI mode `--verify-engine-world-scene-package`，基于 World-driven minimal scene 验证 package save/load。
- `tools/verify_pbr.ps1` 新增 `engine-world-scene-package` 默认 mode，断言 package saved/loaded、Actor / SceneComponent 数量 roundtrip、persistent id 保留、root component 恢复、文件 schema 和 preset id 写入。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册 `ScenePackage` 新文件。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-package -DiscardCaptures`：构建通过；`ScenePackage.cpp` 参与编译；scene package 输出 `saved=yes`、`loaded=yes`、`actors=5`、`sceneComponents=5`、`loadedActors=5`、`loadedSceneComponents=5`、`loadedActorsWithPersistentIds=5`、`loadedSceneComponentsWithPersistentIds=5`、`restoredRootComponents=5`、`restoredSceneComponentParents=4`。
- 已检查 `out\engine_world_scene_package.verification.ini`，文件包含 `schema=engine.world.scenePackage.v1`、`world:preset:engine-world-minimal-scene`、5 个 Actor persistent id、5 个 SceneComponent persistent id、root component 和 parent persistent id。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 已通过；新增 `engine-world-scene-package` mode 在全量默认验证中继续输出 `saved=yes`、`loaded=yes`、`loadedActors=5`、`loadedSceneComponents=5`、`loadedActorsWithPersistentIds=5`、`loadedSceneComponentsWithPersistentIds=5`、`restoredRootComponents=5`、`restoredSceneComponentParents=4`。

结论：

- 当前已经从 transform-only snapshot 推进到对象结构级 scene package roundtrip。
- 这仍是最小 package，不是完整资产序列化：加载时会恢复普通 `Actor` / `SceneComponent`，不会恢复 `MeshActor`、材质、geometry、light、camera 或旧 renderer adapter。
- 这一步足够作为后续 editor save/load、asset reference、prefab/level package 的基础边界。

下一步建议：

- 给 scene package 增加 asset reference / component type registry：先不直接序列化 mesh 和 material 实体，只保存 renderer asset handle 或 adapter descriptor，让加载后的 World 能重新接回 renderer。

### 2026-05-25 Scene Package Component Type / Adapter Metadata

本轮推进上一节的下一步：scene package 现在不只保存对象结构，还会保存 Actor / SceneComponent 类型信息和 renderer adapter 描述。

新增与修改：

- `ScenePackage` 保存时不再把所有 Actor 写成 `Actor`，会写出 `MeshActor`、`LegacyObjectActor` 等 actor type metadata。
- `ScenePackage` 保存时不再把所有组件写成 `SceneComponent`，会写出 `MeshComponent`、`LegacyObjectComponent`、`LightComponent`、`CameraComponent` 等 component type metadata。
- 新增 adapter descriptor 字段：`adapter.kind`、`adapter.assetReference`、`adapter.objectName`、`adapter.objectType`、`adapter.materialType`。
- 对当前 World-driven minimal scene，mesh adapter 会写出 `runtime-generated:mesh:*` reference，并记录 `adapter.materialType=PBRMaterial`；root legacy object 会写出 `runtime-generated:legacy-object:*` reference。
- `loadScenePackage(...)` 现在会按 component type 创建对应的 typed component 类；当前 renderer 资源仍未解析，因此 typed component 的 adapter pointer 为空，但 package 已保留 resolver 所需的 asset reference。
- `ScenePackageSaveResult` / `ScenePackageLoadResult` 新增 `typedSceneComponentCount`、`adapterDescriptorCount`、`loadedTypedSceneComponents`、`loadedAdapterReferences`、`unresolvedAdapterReferences` 等统计。
- `verify_pbr.ps1` 扩展 `engine-world-scene-package` 断言：要求 package 写入 actor/component type metadata、mesh adapter descriptor、mesh asset reference 和 PBR material metadata，并要求 load 阶段恢复 typed components、读取 adapter references。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-package -DiscardCaptures`：构建通过；`ScenePackage.cpp` 参与编译；scene package stats 输出 `typedSceneComponents=5`、`adapterDescriptors=5`、`loadedTypedSceneComponents=5`、`loadedAdapterReferences=5`、`unresolvedAdapterReferences=5`。
- 已检查 `out\engine_world_scene_package.verification.ini`，文件包含 `actor.N.type=MeshActor`、`actor.0.type=LegacyObjectActor`、`sceneComponent.N.type=MeshComponent`、`sceneComponent.0.type=LegacyObjectComponent`、`adapter.kind=mesh`、`adapter.assetReference=runtime-generated:mesh:*`、`adapter.materialType=PBRMaterial`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过；新增 adapter metadata 断言在全量默认验证中通过。

结论：

- Scene package 已从“只恢复普通 Actor / SceneComponent”推进到“保存并恢复 typed SceneComponent 类，同时保留 renderer adapter reference”。
- 当前 `unresolvedAdapterReferences=5` 是预期结果：还没有 asset resolver / renderer reconstruction，因此加载后的 typed component 暂时没有真实 mesh/material/object 指针。
- 下一步应增加 asset resolver：根据 `adapter.assetReference` 和 `adapter.materialType` 重建或绑定 renderer object，使从 package 加载出的 World 可以重新导出到旧 renderer。

### 2026-05-27 Scene Package Asset Resolver / Renderer Reconstruction

本轮完成上一节的 resolver 缺口：scene package 现在不只是保存 typed component 和 adapter reference，也能在加载阶段通过外部 resolver 回填真实 renderer object，并证明 package-loaded World 可以重新导出到旧 renderer scene。

新增与修改：

- `ScenePackage.h` 新增 `ScenePackageAdapterDescriptor`、`ScenePackageAssetResolver` 和 `ScenePackageLoadOptions`。
- `ScenePackageAssetResolver` 当前提供 `resolveMesh(...)`、`resolveLight(...)`、`resolveLegacyObject(...)` 三个扩展点；默认实现返回空对象，保持无 resolver 加载行为可用。
- `loadScenePackage(path, options)` 会读取 adapter descriptor，并在有 resolver 时为 `MeshComponent`、`LightComponent`、`LegacyObjectComponent` 回填对应 renderer adapter。
- `ScenePackageLoadResult` 新增 `resolvedAdapterReferences`，现在会区分 `loadedAdapterReferences`、`resolvedAdapterReferences` 和 `unresolvedAdapterReferences`。
- `RuntimePBRVerification` 新增 runtime scene package resolver：针对当前 World-driven minimal scene 的 `runtime-generated:mesh:*` reference 重建 PBR sphere mesh，针对 `runtime-generated:legacy-object:*` reference 重建 legacy root object。
- `RuntimePBRVerification` 在 scene package roundtrip 后创建临时 `GLframework::Scene`，把 package-loaded World 通过 `WorldLegacySceneExporter` 重新导出，并输出 loaded World export stats。
- `verify_pbr.ps1` 扩展 `engine-world-scene-package` 断言：要求 `resolvedAdapterReferences>=5`、`unresolvedAdapterReferences=0`，并在后续 light fixture 接入后断言 loaded World 可导出 `6` 个对象、`4` 个 mesh、`1` 个 light、`1` 个 legacy object 和 `5` 条 attachment。
- `docs/engine_phase1_execution_plan.md`、`docs/engine_project_book.md`、`docs/engine_interface_design.md`、`docs/engine_transformation_project_plan.md` 已更新当前进度与下一步方向。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-package -DiscardCaptures`：构建通过；scene package stats 输出 `resolvedAdapterReferences=6`、`unresolvedAdapterReferences=0`、`exportedLoadedWorldObjects=6`、`exportedLoadedWorldMeshes=4`、`exportedLoadedWorldLights=1`、`exportedLoadedWorldLegacyObjects=1`、`exportedLoadedWorldAttachments=5`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过；`engine-world-scene-package` mode 在全量验证中继续输出同样的 resolver 和 loaded World export 统计。

结论：

- scene package 已从“能保存/恢复对象结构”推进到“能恢复 typed component 并接回 renderer adapter”。
- 当前 resolver 是 verification bridge，不是最终资产系统：它根据 `runtime-generated:*` reference 重新生成 mesh/object，足够证明架构边界，但不能替代长期 AssetRegistry。
- 下一步应该进入稳定资产句柄 / AssetRegistry：给 mesh、material、texture、imported asset 建立稳定 handle，使 scene package 依赖 handle 解析真实资源，而不是依赖 runtime-generated 临时引用。

### 2026-05-27 AssetRegistry Stable Handle Slice

本轮根据 subagents 审查结果，推进 AssetRegistry 的最小实现切片。目标仍然不是做完整资产系统，而是把 scene package 的 renderer adapter reference 从“临时 runtime reference”推进到“稳定 asset handle + 兼容 fallback”。

新增与修改：

- 新增 `engine/AssetRegistry.h/.cpp`，定义 `AssetKind`、`AssetHandle`、`AssetDescriptor`、`AssetRegistry` 和稳定 handle 工具函数。
- 当前 handle 格式为 `asset:<kind>:<source>:<path>`，例如 `asset:mesh:runtime-generated:engine-world-minimal-matte-sphere`。
- `ScenePackageAdapterDescriptor` 新增 `assetHandle` 字段。
- `saveScenePackage(...)` 现在同时写出：
  - `adapter.assetHandle=asset:mesh:runtime-generated:*`、`asset:legacy-object:runtime-generated:*` 或 `asset:light:runtime-generated:*`
  - `adapter.assetReference=runtime-generated:*` 作为兼容 fallback
- `loadScenePackage(...)` 会读取 asset handle，并新增 `assetHandles`、`loadedAssetHandles`、`resolvedAssetHandles`、`unresolvedAssetHandles` 统计。
- runtime scene package resolver 现在优先识别 `adapter.assetHandle`，同时兼容旧 `adapter.assetReference`。
- runtime scene package resolver 已覆盖 mesh、legacy-object 和 light 三类 adapter；light 通过 package-only `LightActor` fixture 验证，不影响原 PBR 渲染场景输出。
- `loadScenePackage(...)` 现在会按保存的 actor type 恢复 `MeshActor`、`LightActor`、`LegacyObjectActor` 等动态类型，并且 typed component 统计只在实际构造出的 component 类型匹配时计数。
- `verify_pbr.ps1` 扩展 `engine-world-scene-package` 断言：要求 package 文件写出 mesh / legacy-object / light asset handle，保留 mesh / light fallback reference，并要求 stable handle 全部 resolved。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册 `AssetRegistry` 新文件。
- 根据 resolver audit 修正 camera 边界：`CameraComponent` 暂时只保存 component type 和 adapter kind，不写可解析 asset handle/reference，避免在没有 camera resolver/ownership 策略前制造必然 unresolved 的资源引用。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-package -DiscardCaptures`：构建通过；`AssetRegistry.cpp`、`ScenePackage.cpp` 参与编译；scene package stats 输出 `typedActors=6`、`loadedTypedActors=6`、`assetHandles=6`、`loadedAssetHandles=6`、`resolvedAssetHandles=6`、`unresolvedAssetHandles=0`，并保持 `resolvedAdapterReferences=6`、`unresolvedAdapterReferences=0`。
- 已检查 `out\engine_world_scene_package.verification.ini`：文件同时包含 `adapter.assetHandle=asset:mesh:runtime-generated:*` / `asset:legacy-object:runtime-generated:*` / `asset:light:runtime-generated:*` 和 `adapter.assetReference=runtime-generated:*` fallback。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过；`engine-world-scene-package` mode 在全量验证中继续输出 stable handle resolved 统计。

结论：

- AssetRegistry 当前完成的是“稳定引用身份”边界，不是 live object owner，也不复用原 renderer 对象，避免污染原 scene parent/transform。
- scene package 现在已经具备从 `assetHandle` 到 resolver 的第一条链路，并已覆盖 mesh、light、legacy object；后续可以逐步把 imported mesh、material、texture 接入同一 handle 格式。
- 下一步建议优先把 imported mesh/material/texture 纳入 stable handle；package 层如继续强化，应补 duplicate persistent id、self-parent/cycle attach、跨 actor parent reference 等 graph validation。

### 2026-05-27 Scene Package Negative Probes

本轮继续完成 resolver audit 中剩余的 package 健壮性缺口。目标不是扩大渲染特性，而是让 scene package loader 对未知类型和损坏输入有可观察、可验证的行为。

新增与修改：

- `ScenePackageLoadResult` 新增 `unknownActorTypes`、`unknownSceneComponentTypes`、`invalidSceneComponentTransforms` 诊断计数。
- `loadScenePackage(...)` 现在会在遇到未知 actor type 时 fallback 到普通 `Actor`，同时记录 `unknownActorTypes`；遇到未知 component type 时 fallback 到普通 `SceneComponent`，同时记录 `unknownSceneComponentTypes`。
- `loadScenePackage(...)` 现在会检测存在但无法完整解析的 transform 字段，并记录 `invalidSceneComponentTransforms`，避免坏数值被静默忽略。
- `RuntimePBRVerification` 新增 scene package negative probes：写入 missing schema、invalid line、missing count 三个损坏 package，并要求 loader 拒绝；同时写入 unknown type package，要求 loader fallback 成功并输出 unknown/invalid 计数。
- `verify_pbr.ps1` 新增 `Engine world scene package negative stats` 断言：要求损坏 package 被拒绝，unknown type package 可加载，且 `unknownActorTypes>=1`、`unknownSceneComponentTypes>=1`、`invalidSceneComponentTransforms>=1`。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-package -DiscardCaptures`：构建通过；正常 package 输出 `unknownActorTypes=0`、`unknownSceneComponentTypes=0`、`invalidSceneComponentTransforms=0`；negative probe 输出 `missingSchemaRejected=yes`、`invalidLineRejected=yes`、`missingCountRejected=yes`、`unknownTypesLoaded=yes`、`unknownActorTypes=1`、`unknownSceneComponentTypes=1`、`invalidSceneComponentTransforms=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过；`engine-world-scene-package` summary 包含正常 package 0 unknown/invalid 计数和 negative probe reject/fallback 计数。

结论：

- scene package loader 现在对“未来类型兼容”和“损坏文件拒绝”都有验证边界。
- 下一步可以进入 imported mesh/material/texture stable handle，或继续补更严格的 package graph validation，例如 duplicate persistent id、self-parent/cycle attach、跨 actor parent reference 等。

### 2026-05-27 Imported Asset Stable Handles

本轮继续 AssetRegistry 方向，把 Assimp PBR import probe 的 mesh / material / texture 纳入 `asset:<kind>:<source>:<path>` 统一 handle 格式。目标仍然是建立资产身份边界，不让 registry 接管 renderer object 生命周期。

新增与修改：

- `LegacySceneImportOptions` 新增 `assetRegistry`、`assetHandleSource`、`assetHandleScope`，允许旧 `Scene / Object` 导入 runtime `World` 时同步注册资产句柄。
- `LegacySceneImportStats` 新增 `assetHandles`、`meshAssetHandles`、`materialAssetHandles`、`textureAssetHandles`，用于 verification 证明导入资产确实进入 stable handle 体系。
- `LegacySceneWorldBuilder` 在遇到 imported `Mesh` 时会注册：
  - `asset:mesh:imported-asset:*`
  - `asset:material:imported-asset:*`
  - `asset:texture:imported-asset:*`
- `RuntimePBRVerification` 的 `--verify-pbr-import` 现在创建临时 `AssetRegistry`，以 `assetHandleSource=imported-asset`、`assetHandleScope=fbx-test-test-fbx` 导入 `fbx/test/test.fbx`。
- `verify_pbr.ps1` 的 `import` mode 现在断言 imported asset stats 中 mesh/material/texture handle 数量都大于 0。

实现说明：

- 当前没有修改 `framework/texture.*`，因为该文件不是 UTF-8，不能用当前 patch 工具安全编辑。
- 因此第一版 texture handle 使用 imported asset scope + mesh path + PBR texture slot label + texture dimensions/unit 生成；后续整理 Texture 模块时，再把真实 source path 作为 Texture 元数据补入 handle path。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes import -DiscardCaptures`：构建通过；`importedAssetWorldImport` 输出 `assetHandles=3`、`meshAssetHandles=1`、`materialAssetHandles=1`、`textureAssetHandles=1`、`assetHandleSource=imported-asset`、`persistentIdSource=imported-asset`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过；`import` summary 保持 `assetHandles=3`、`meshAssetHandles=1`、`materialAssetHandles=1`、`textureAssetHandles=1`。

结论：

- imported asset 已从“只有 persistent id”推进到“mesh/material/texture 都有 stable asset handle”。
- 下一步建议把这条 handle 注册结果持久化到 scene package 或 editor asset browser 可见层；如果先继续 package 健壮性，则补 duplicate persistent id、self-parent/cycle attach、跨 actor parent reference。

### 2026-05-27 Imported Asset Scene Package Manifest

本轮继续上一节的下一步，把 imported asset handle 从“运行时临时 registry stats”推进到“scene package 文件可持久化、可重新读取”的 asset manifest。

新增与修改：

- `ScenePackageSaveResult` 新增 `assetManifestCount`、`meshAssetManifestCount`、`materialAssetManifestCount`、`textureAssetManifestCount`。
- `ScenePackageLoadResult` 新增 `loadedAssetManifestCount`、`loadedMeshAssetManifestCount`、`loadedMaterialAssetManifestCount`、`loadedTextureAssetManifestCount`。
- `saveScenePackage(...)` 现在会扫描 persistent id 来源为 `component:imported-asset:*` 的 `MeshComponent`，并写出：
  - `assetManifest.N.handle=asset:mesh:imported-asset:*`
  - `assetManifest.N.handle=asset:material:imported-asset:*`
  - `assetManifest.N.handle=asset:texture:imported-asset:*`
- `loadScenePackage(...)` 会读取 asset manifest 并统计 mesh/material/texture manifest 数量。
- `RuntimePBRVerification` 的 `--verify-pbr-import` 现在会在导入 `fbx/test/test.fbx` 后保存 `out/pbr_import_asset_scene_package.verification.ini`，再加载该 package 并输出 `PBR imported asset scene package stats`。
- `verify_pbr.ps1` 的 `import` mode 新增断言：package 必须保存/加载成功，asset manifest 必须包含 imported mesh/material/texture handles，且 load 阶段能读回三类 manifest。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes import -DiscardCaptures`：构建通过；`importedAssetScenePackage` 输出 `saved=yes`、`loaded=yes`、`assetManifest=3`、`meshAssets=1`、`materialAssets=1`、`textureAssets=1`、`loadedAssetManifest=3`、`loadedMeshAssets=1`、`loadedMaterialAssets=1`、`loadedTextureAssets=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过；`import` summary 包含 `importedAssetScenePackage`，并保持 mesh/material/texture manifest 保存和读回断言通过。

结论：

- imported mesh/material/texture stable handle 已进入 scene package 持久化边界。
- 下一步建议转到 editor asset browser / inspector 可见层，或者继续强化 package graph validation。

### 2026-05-27 Editor Asset Browser / Inspector Visibility

本轮把 imported asset handle 从 package / verification 数据推进到 editor 可见层。目标仍然是只暴露资产身份，不让 ImGui 面板接管 renderer object ownership。

新增与修改：

- `AssetRegistry` 新增 `listAssets()`，返回按 handle 排序的 `AssetDescriptor` 快照，避免 editor 直接遍历 `unordered_map` 内部存储。
- `AppRuntimeContext` 新增 `assetRegistry`，importer、verification 和 editor 共享同一个 runtime registry 数据源。
- `RuntimeEditorPanelCoordinator` 会把 `context.assetRegistry` 作为只读指针传入 `EditorPanelContext`。
- Editor 新增 `asset browser` 面板，显示总 asset 数、mesh/material/texture 数和 imported handle 数，并列出 imported / all asset handles。
- `SelectionKind` 新增 `Asset`，asset browser 点击后只保存 `selectedAssetHandle` 字符串，不保存 renderer pointer 或 registry descriptor pointer。
- `inspector` 支持 asset selection，通过 `AssetRegistry::find(AssetHandle)` 显示 name、kind、handle、source、path、material type。
- `RuntimePBRVerification` 的 import probe 改为写入 `context.assetRegistry`，并输出 `Runtime asset registry stats`。
- `verify_pbr.ps1` 的 `import` mode 断言 runtime registry 中可见 imported mesh/material/texture handles。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes import -DiscardCaptures`：构建通过；`runtimeAssetRegistry` 输出 `assets=3`、`meshAssets=1`、`materialAssets=1`、`textureAssets=1`、`importedAssetHandles=3`。

subagent 审计结论：

- Editor Asset Visibility Agent 的主要建议已采纳：context-owned registry、只读枚举 API、editor context 指针、按 handle 字符串选中、verification registry stats。
- Scene Package Graph Validation Agent 建议下一步优先补 package graph validation：duplicate actor/component persistent id、unresolved parent persistent id、invalid parent index、self-parent、cycle attach，以及合法 cross-actor parent restore 计数。

结论：

- imported asset handle 已完成“注册 -> package manifest -> editor browser -> inspector”的第一条可见链路。
- AssetRegistry 仍只是资产身份 registry，不是 live renderer object owner。
- 下一步建议转入 scene package graph validation，而不是继续扩展 PBR 效果。

### 2026-05-27 Scene Package Graph Validation

本轮根据 Scene Package Graph Validation Agent 的审计，补上 package loader 的结构图验证。目标是让 package 在进入 runtime World 前先拒绝坏对象图，而不是把错误 attachment 静默吞掉。

新增与修改：

- `ScenePackageLoadResult` 新增 graph diagnostics：duplicate actor/component persistent id、invalid root index、unresolved parent、invalid parent index、self-parent、cyclic parent、restored cross-actor parent。
- `loadScenePackage(...)` 现在先构造 Actor / SceneComponent，再验证 parent graph，最后统一执行 `attachTo(...)`。
- duplicate actor/component persistent id 会导致 load 失败，避免 component persistent id map 被后写入项覆盖。
- `parentPersistentId` 明确存在但解析失败时会直接记录 unresolved parent，不再 fallback 到 `parentSceneComponent`。
- `parentSceneComponent` index 只有在没有 `parentPersistentId` 时作为 same-actor fallback 使用。
- self-parent 和 cycle parent 会在 attach 前被检测并拒绝。
- 合法 cross-actor parent reference 继续支持，并通过 `restoredCrossActorParentReferences` 计数。
- scene package negative probes 新增 duplicate actor id、duplicate component id、unresolved parent、invalid parent index、self-parent、cycle 和 valid cross-actor parent package。
- `verify_pbr.ps1` 扩展 `engine-world-scene-package` 断言：有效 package 的 graph error 计数必须为 0，坏图必须被拒绝，合法 cross-actor parent 必须恢复。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes engine-world-scene-package -DiscardCaptures`：构建通过；有效 package 输出 `duplicateActorPersistentIds=0`、`duplicateSceneComponentPersistentIds=0`、`unresolvedParentReferences=0`、`invalidParentSceneComponentIndices=0`、`selfParentReferences=0`、`cyclicParentReferences=0`，并输出 `restoredCrossActorParentReferences=5`。
- negative stats 输出 `duplicateActorIdRejected=yes`、`duplicateSceneComponentIdRejected=yes`、`unresolvedParentRejected=yes`、`invalidParentIndexRejected=yes`、`selfParentRejected=yes`、`cycleRejected=yes`、`validCrossActorParentLoaded=yes`。

结论：

- scene package loader 从“解析并尽量恢复”推进到“先验证对象图，再恢复 attachment”的边界。
- 这一步比继续扩展 PBR 更接近游戏引擎目标，因为它强化的是场景资产和 runtime World 的可靠性。
- 下一步建议把 AssetRegistry 从 runtime context 字段推进到 EngineSubsystem/AssetSubsystem，或继续把 package asset manifest 读回后注册进 runtime registry。

### 2026-05-27 Scene Package Asset Manifest Registry Reload

本轮继续资产链路，把 package 文件中的 `assetManifest.N.*` 从“可读统计”推进到“load 时可注册进 runtime AssetRegistry”。这让从 package 恢复出来的资产身份也能重新进入 editor asset browser / inspector 可见层。

新增与修改：

- `ScenePackageLoadOptions` 新增 `AssetRegistry* assetRegistry`，调用方可以选择在 load package 时同步注册 asset manifest。
- `ScenePackageLoadResult` 新增 `registeredAssetManifestCount`、`registeredMeshAssetManifestCount`、`registeredMaterialAssetManifestCount`、`registeredTextureAssetManifestCount`。
- `loadScenePackage(...)` 读取 `assetManifest.N.handle/kind/source/name/path/materialType` 后，会构造 `AssetDescriptor` 并注册到传入的 runtime `AssetRegistry`。
- `RuntimePBRVerification` 的 import probe 保存 imported asset scene package 后，会清空 runtime registry，再通过 package load options 从 manifest 注册回 registry。
- `verify_pbr.ps1` 的 `import` mode 现在断言 imported asset package load 能注册回 mesh/material/texture manifest assets。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes import -DiscardCaptures`：构建通过；`importedAssetScenePackage` 输出 `registeredAssetManifest=3`、`registeredMeshAssets=1`、`registeredMaterialAssets=1`、`registeredTextureAssets=1`，`runtimeAssetRegistry` 继续输出 `assets=3`、`meshAssets=1`、`materialAssets=1`、`textureAssets=1`、`importedAssetHandles=3`。

结论：

- imported asset handle 现在具备完整的 “import 注册 -> package manifest 保存 -> package load 注册回 runtime registry -> editor 可见” 链路。
- 下一步建议把 `AssetRegistry` 从 `AppRuntimeContext` 字段提升为 engine subsystem / asset subsystem 边界，避免长期把资产系统放在 application context。

### 2026-05-27 AssetRegistry Subsystem Boundary

本轮把 `AssetRegistry` 从 application context 的直接字段推进到 engine subsystem 边界。当前目标不是做完整异步资产系统，而是先把 registry owner 放到 engine 层，后续再接入真正的 Engine lifecycle。

新增与修改：

- 新增 `AssetSubsystem`，继承 `EngineSubsystem`，内部持有 `AssetRegistry`。
- `AssetSubsystem` 当前提供 `getRegistry()`、`clear()`、`isInitialized()`，并实现 `initialize/tick/shutdown` 生命周期接口。
- `AppRuntimeContext` 不再直接持有 `AssetRegistry`，改为持有 `AssetSubsystem`。
- Editor panel context 通过 `context.assetSubsystem.getRegistry()` 获取只读 registry。
- PBR import probe 和 scene package manifest reload 通过 `assetSubsystem.getRegistry()` 注册和验证 imported asset handles。
- `text2.vcxproj` / `.filters` 已注册 `AssetSubsystem` 新文件。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes import -DiscardCaptures`：构建通过；`AssetSubsystem.cpp` 参与编译；`runtimeAssetRegistry` 继续输出 `assets=3`、`meshAssets=1`、`materialAssets=1`、`textureAssets=1`、`importedAssetHandles=3`。

结论：

- AssetRegistry 已不再作为裸 registry 字段暴露在 application context 中，而是通过 engine-side `AssetSubsystem` 访问。
- 这一节先完成 subsystem 类型边界；下一节继续把 owner 从 runtime context 移到 `Engine`。

### 2026-05-27 Engine-Owned AssetSubsystem Handoff

本轮继续上一节，把 `AssetSubsystem` 从“application context 直接持有的 engine-side 类型”推进到“由 `Engine` 创建和管理生命周期，runtime context 只保存非拥有引用”的边界。这样 asset registry 不再是 application context 的长期 owner，后续其他系统可以按同一方式接入 Engine lifecycle。

新增与修改：

- `AppRuntimeContext` 中的 `AssetSubsystem` 实例改为 `AssetSubsystem*` 非拥有指针。
- `RuntimeApplicationShell` 持有 `GLengine::Engine`，初始化阶段通过 `Engine::addSubsystem<AssetSubsystem>()` 创建 asset subsystem。
- `RuntimeApplicationShell` 根据当前 runtime 配置生成 `EngineDesc`，并调用 `mEngine.initialize(...)` / `mEngine.shutdown()`。
- runtime context 在 shell 初始化时获得 Engine-owned `AssetSubsystem` 指针，cleanup 时显式置空。
- Editor asset browser / inspector 通过可空 subsystem 指针取得 registry；当 subsystem 不可用时不会解引用空指针。
- PBR import verification 通过 Engine-owned `AssetSubsystem` 的 registry 注册 imported mesh/material/texture handles，并继续验证 package manifest reload。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes import -DiscardCaptures`：构建通过；`runtimeAssetRegistry` 输出 `assets=3`、`meshAssets=1`、`materialAssets=1`、`textureAssets=1`、`importedAssetHandles=3`；`importedAssetScenePackage` 继续输出 `registeredAssetManifest=3`、`registeredMeshAssets=1`、`registeredMaterialAssets=1`、`registeredTextureAssets=1`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过。

结论：

- AssetRegistry 的 owner 已从 application context 移到 Engine subsystem。
- 当前 `Engine` 已进入 runtime shell lifecycle，但 frame tick 还没有成为统一调度入口。
- 下一步建议优先选择一个新的核心 owner 迁移对象：要么把 runtime `World` 从 `AppRuntimeContext` 迁到 Engine-owned `WorldSubsystem` / `SceneSubsystem`，要么让现有 `RendererSubsystem` 更正式地进入 Engine tick/frame pipeline。

### 2026-05-27 Engine-Owned World Handoff First Slice

本轮继续上一节的下一步，把 runtime `World` 的 ownership 从 `AppRuntimeContext` 移到 `Engine`。当前没有新增单独的 `WorldSubsystem`，原因是 `Engine` 已经有 `createWorld(...)` / `getActiveWorld()` 和 `mActiveWorld` owner；如果此时再新增一个 subsystem 持有另一个 active World，会形成重复 owner。更合理的第一步是先让 application context 只保存非拥有指针，后续再决定是否把 `Engine::mActiveWorld` 抽成正式 `WorldSubsystem`。

新增与修改：

- `AppRuntimeContext::engineWorld` 从 `std::shared_ptr<GLengine::World>` 改为非拥有 `GLengine::World*`。
- `RuntimeScenePreparer::prepare(...)` 现在接收 shell 持有的 `GLengine::Engine&`，并把 `Engine*` 传入 scene setup context。
- `SceneSetupPipeline` 的 legacy mirror、World-driven probe 和 minimal scene 都通过 `Engine::createWorld(...)` 创建 runtime World，然后把 active World 指针写回 context。
- `EditorPanelContext`、hierarchy、transaction summary、snapshot save/apply 和 editor create actor 入口都改为使用非拥有 `World*`。
- `RuntimeApplicationShell::cleanup()` 会在 `Engine::shutdown()` 前清空 runtime `engineWorld` 指针和 editable 标记，避免 editor/runtime context 留下悬空 owner 语义。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；默认 legacy mirror 仍输出 `runtimeWorldActors=33`；World-driven minimal scene 仍输出 `runtimeWorldActors=5`、`engineWorldMinimalMeshes=4`，snapshot save/apply 继续通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过；`engine-world-scene-package` 继续保存/加载 package、验证 graph negative probes 和 snapshot apply。
- 已执行 `rg` 检查，代码路径中不再有 `std::shared_ptr<GLengine::World>`、`std::make_shared<GLengine::World>` 或 `engineWorld.get()` 残留；仅历史文档中保留旧阶段记录。

结论：

- runtime `World` 已不再由 application context 共享持有，而是由 `Engine` active world 持有。
- `AppRuntimeContext` 现在对 `AssetSubsystem` 和 `World` 都只保存非拥有引用，composition root 的 owner 责任开始收敛到 `RuntimeApplicationShell` / `Engine`。
- 下一步可以继续推进两个方向：把 `Engine::mActiveWorld` 抽成正式 `WorldSubsystem`，或让 `RuntimeApplicationShell::runFrame()` 开始调用 `Engine::tick(...)` 并逐步把 frame lifecycle 迁入 Engine。

### 2026-05-27 Engine Tick Frame Loop First Slice

本轮继续上一节的第二个方向，把 `Engine::tick(...)` 接入 runtime frame loop。目标不是立刻把 renderer pipeline 全部迁到 Engine，而是先让 Engine lifecycle 每帧真实运行，并让 verification 能证明 active World 已经进入 begin/tick/end 路径。

新增与修改：

- `RuntimeFrameConfig` 新增 `GLengine::Engine* engine` 与 `deltaSeconds`。
- `RuntimeFrameRunner::run(...)` 在旧 renderer frame pipeline 前调用 `engine->tick(deltaSeconds)`。
- `RuntimeApplicationShell::makeFrameConfig()` 把 shell 持有的 `mEngine` 传给 frame runner，并先使用固定 `1.0f / 60.0f` delta，避免 verification 受平台计时抖动影响。
- `Engine::initialize(...)` 在 Engine 已有 active World 时调用 `beginPlay()`；`Engine::createWorld(...)` 在 Engine 已初始化时也会让新 active World 立即 begin play。
- `RuntimePBRVerification::reportRenderedFrame(...)` 现在接收 `Engine&`，并输出 `Runtime engine tick stats`：initialized、activeWorld、activeWorldPlaying、timeSeconds、deltaSeconds。
- `verify_pbr.ps1` 对每个 verification mode 解析并断言 `Runtime engine tick stats`，要求 Engine 已初始化、active World 存在且 playing、`timeSeconds > 0`、`deltaSeconds > 0`。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；默认 legacy mirror 和 World-driven minimal scene 均输出 `runtimeEngineTick: initialized=yes, activeWorld=yes, activeWorldPlaying=yes, timeSeconds=0.033333, deltaSeconds=0.016667`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过；每个 mode 都输出并断言 `Runtime engine tick stats`。

结论：

- Engine 不再只是初始化期 owner；它已经进入每帧 runtime 调度。
- 当前 renderer frame pipeline 仍由 `RuntimeFramePipeline` 执行，Engine tick 先作为独立 lifecycle phase 接入。
- 下一步可以把固定 delta 替换为 runtime frame clock，也可以继续把 `RendererSubsystem` 接入 Engine tick/frame pipeline。

### 2026-05-27 Runtime Frame Clock First Slice

本轮继续上一节，把 `Engine::tick(...)` 使用的 delta 从 shell 中写死的 `1.0f / 60.0f` 推进到独立 frame clock。目标是让普通 editor/game 运行具备真实 frame delta，同时保持 verification 的固定 delta，避免测试结果被平台计时抖动影响。

新增与修改：

- 新增 `RuntimeFrameClock`，使用 `std::chrono::steady_clock` 计算普通运行的 frame delta。
- `RuntimeFrameClockConfig` 提供 `useFixedDelta`、`fixedDeltaSeconds`、`maxDeltaSeconds`。
- `RuntimeApplicationShellConfig` 新增 `frameClock` 配置，shell 内部持有 `RuntimeFrameClock mFrameClock`。
- shell 初始化完成后调用 `mFrameClock.reset()`。
- `RuntimeApplicationShell::makeFrameConfig()` 通过 `mFrameClock.tick(makeFrameClockConfig())` 获取 delta，再交给 `RuntimeFrameRunner`。
- `RuntimeApplicationShell::makeFrameClockConfig()` 在 verification 模式下强制 `useFixedDelta=true` 和 `fixedDeltaSeconds=1.0f / 60.0f`；普通运行则使用真实 `steady_clock` delta，并通过 `maxDeltaSeconds` 避免长帧导致 simulation jump。
- `text2.vcxproj` / `.filters` 已注册新增 `RuntimeFrameClock` 文件。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimeFrameClock.cpp` 参与编译；verification 路径继续输出 `runtimeEngineTick: initialized=yes, activeWorld=yes, activeWorldPlaying=yes, timeSeconds=0.033333, deltaSeconds=0.016667`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 33 个 verification mode 全部通过，所有 mode 的 Engine tick stats 继续保持固定 verification delta。

结论：

- Engine tick 已从“固定常量 delta”升级为“runtime frame clock 驱动”。
- verification 仍使用固定 delta，保证已有 33 个验证模式保持稳定。
- 下一步可以继续把 `RendererSubsystem` 接入 Engine tick/frame pipeline，或把 frame clock stats 暴露到 editor/debug panel。

### 2026-05-27 RendererSubsystem Frame Bridge First Slice

本轮继续上一节的主线，把现有 `RendererSubsystem` 从“已注册但不参与 frame”的状态推进到 Engine-owned runtime frame bridge。当前仍不迁移完整 `RuntimeFramePipeline`，避免一次性改变渲染执行路径；本轮只建立 Engine subsystem 生命周期、runtime renderer 非拥有绑定，以及 render 前后 bridge 统计。

新增与修改：

- `RuntimeApplicationShell` 在 `Engine::initialize(...)` 前通过 `Engine::addSubsystem<RendererSubsystem>()` 创建 renderer subsystem，并在 scene prepare 完成后把 `mRuntime.renderer.get()` 作为非拥有指针绑定给 subsystem。
- `RuntimeFrameConfig` 新增 `RendererSubsystem*`，frame runner 在旧 renderer pipeline 前调用 `beginFrameBridge(...)`，在 `RuntimeFramePipeline::render(...)` 后调用 `endFrameBridge(...)`。
- `RendererSubsystem` 新增 `RendererSubsystemFrameBridgeStats`，记录 initialized、hasRenderer、frameBridgeActive、begin/completed frame count、observed renderer pass count、Engine time/delta。
- `RuntimePBRVerification::reportRenderedFrame(...)` 输出 `Runtime renderer subsystem stats`，并验证 subsystem 绑定的是当前 runtime renderer。
- `verify_pbr.ps1` 对所有 verification mode 新增全局断言：renderer subsystem stats 必须存在，initialized/hasRenderer/runtimeRendererAttached/frameBridgeActive 必须为 yes，begin/completed frames 必须大于 0 且相等，observed renderer passes、engine time 和 delta 必须大于 0。
- `docs/subagents_coordination.md` 已切换到本轮 RendererSubsystem frame bridge 协作边界，并明确两个子 agent 只读审查，不写代码。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；`RendererSubsystem.cpp`、`RuntimeFrameRunner.cpp`、`RuntimeApplicationShell.cpp` 和 verification 代码均参与编译。
- `forward` 输出 `runtimeRendererSubsystem: initialized=yes, hasRenderer=yes, runtimeRendererAttached=yes, frameBridgeActive=yes, beginFrames=2, completedFrames=2, observedRendererPasses=8`。
- `engine-world-minimal-scene` 输出 `runtimeRendererSubsystem: initialized=yes, hasRenderer=yes, runtimeRendererAttached=yes, frameBridgeActive=yes, beginFrames=2, completedFrames=2, observedRendererPasses=6`。

结论：

- RendererSubsystem 已进入 Engine-owned runtime frame lifecycle，但当前仍是 bridge，不接管实际 renderer pass execution。
- 旧 `RuntimeFramePipeline` 继续负责渲染执行；Engine/RendererSubsystem 现在能够观察并验证该帧渲染确实完成。
- 下一步可以把 `RuntimeFramePipeline::render(...)` 的入口进一步移动到 RendererSubsystem API 后面，或者先把 RendererSubsystem stats 暴露到 editor/debug panel。

### 2026-05-27 RendererSubsystem Render Entry Wrapper

本轮继续上一节的下一步，把 `RuntimeFrameRunner` 中手写的 `beginFrameBridge -> RuntimeFramePipeline::render -> endFrameBridge` 收敛为一个 `RendererSubsystem` API。当前仍不把 application 层的 `RuntimeFramePipeline` 放进 engine 目录，原因是那会让 engine 反向依赖 application；本轮采用 callback 形式，让 engine subsystem 负责 frame bridge lifecycle，application 层只提供旧 pipeline 的 render callback。

新增与修改：

- `RendererSubsystemFrameBridgeStats` 新增 `renderFrameBridgeCallCount`，用于区分“只是 begin/end 被调用”和“渲染入口确实通过 subsystem API 包裹”。
- `RendererSubsystem` 新增 `renderFrameBridge(const EngineContext&, const std::function<void()>&)`，内部负责 bridge call count、begin、调用 render callback、end；如果 callback 抛出异常，会先记录 end 再继续抛出。
- `RuntimeFrameRunner::run(...)` 不再直接展开 begin/render/end 三步；当 Engine 和 RendererSubsystem 都存在时，通过 `rendererSubsystem->renderFrameBridge(...)` 包裹旧 `RuntimeFramePipeline::render(...)`，否则保留直接 render fallback。
- `RuntimePBRVerification` 输出新增 `renderFrameBridgeCalls`。
- `verify_pbr.ps1` 新增全局断言：`renderFrameBridgeCalls > 0`，并且 `renderFrameBridgeCalls == beginFrames`，证明旧 pipeline 入口已经通过 RendererSubsystem API。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出 `renderFrameBridgeCalls=2`、`beginFrames=2`、`completedFrames=2`，且 observed renderer passes 大于 0。

结论：

- Runtime renderer frame entry 已从 runner 的手写三段式逻辑推进到 `RendererSubsystem` API 包裹。
- Engine 目录仍不依赖 application runtime 类型，当前依赖方向保持可控。
- 下一步可以把 `RuntimeFramePipeline` 的配置与 pass plan 逐步下沉到 renderer subsystem 管理，或先把 renderer subsystem stats 暴露给 editor/debug panel。

### 2026-05-27 RendererSubsystem Editor Stats Visibility

本轮继续上一节的第二个低风险方向，把 Engine-owned `RendererSubsystem` 的 frame bridge stats 暴露到 runtime context 和 editor/debug panel。目标不是新增渲染功能，而是让 Engine subsystem 的生命周期状态可以在运行时 UI 中直接观察，为后续迁移 pass plan / config 提供可见反馈。

新增与修改：

- `AppRuntimeContext` 新增非拥有 `GLengine::RendererSubsystem* rendererSubsystem`。
- `RuntimeApplicationShell` 创建 `RendererSubsystem` 后把指针写入 runtime context，cleanup 时清空，保持与 `AssetSubsystem` / `World` 一致的非拥有引用模式。
- `DebugControllerContext` 新增 `const RendererSubsystem* rendererSubsystem`，`RuntimeEditorPanelCoordinator` 从 runtime context 传入该指针。
- `DebugControllerPanel` 新增 `Renderer Subsystem Frame Bridge` 折叠区，展示 initialized、hasRenderer、frameBridgeActive、renderFrameBridgeCalls、begin/completed frames、observed renderer passes、Engine time/delta。
- `RuntimePBRVerification` 的 `Runtime renderer subsystem stats` 新增 `runtimeContextRendererSubsystemAttached` 字段。
- `verify_pbr.ps1` 新增全局断言：runtime context 暴露的 RendererSubsystem 必须和 shell 传入 verification 的 Engine-owned subsystem 相同。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimeApplicationShell.cpp`、`RuntimeEditorPanelCoordinator.cpp`、`DebugControllerPanel.cpp`、`RuntimePBRVerification.cpp` 均参与编译。
- 两个 mode 均输出 `runtimeContextRendererSubsystemAttached=yes`，并继续保持 `renderFrameBridgeCalls=2`、`beginFrames=2`、`completedFrames=2`、`observedRendererPasses > 0`。

结论：

- RendererSubsystem 不再只对 verification 可见，也进入了 editor/debug UI 的可观察边界。
- runtime context 对核心 Engine-owned 对象继续采用非拥有指针，不引入新的 owner。
- 下一步可以继续把 runtime frame pipeline 的 pass plan / config 管理向 RendererSubsystem 收敛，或者把 debug panel 的 stats 进一步整理成 Engine diagnostics 面板。

### 2026-05-27 RendererSubsystem Frame Entry Config

本轮继续上一节的下一步，把 frame entry 的基础配置纳入 `RendererSubsystem` bridge API。目标是让 RendererSubsystem 不只知道“render callback 被调用”，也知道本帧 renderer entry 使用的 framebuffer 配置，为后续把 runtime frame pipeline 的 pass plan / config 管理继续收敛到 subsystem 边界做准备。

新增与修改：

- 新增 `RendererSubsystemFrameConfig`，当前包含 `framebufferWidth` / `framebufferHeight`。
- `RendererSubsystem::renderFrameBridge(...)` 现在接收 `RendererSubsystemFrameConfig`，并在 begin/end bridge 时记录 frame config。
- `RendererSubsystemFrameBridgeStats` 新增 `frameConfigValid`、`framebufferWidth`、`framebufferHeight`。
- `RuntimeFrameRunner` 构造 `RendererSubsystemFrameConfig` 后交给 `RendererSubsystem::renderFrameBridge(...)`，旧 `RuntimeFramePipeline::render(...)` callback 使用同一份 config 生成 `RuntimeFramePipelineConfig`。
- `RuntimePBRVerification` 的 `Runtime renderer subsystem stats` 新增 `frameConfigValid=yes` 和 `framebuffer=WxH`。
- `verify_pbr.ps1` 新增全局断言：RendererSubsystem frame config 必须有效，framebuffer width / height 必须大于 0。
- `DebugControllerPanel` 的 `Renderer Subsystem Frame Bridge` 区块新增 Frame Config Valid 与 Framebuffer 显示。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出并断言 `frameConfigValid=yes`、`framebuffer=1280x720`，同时保持 `renderFrameBridgeCalls=2`、`beginFrames=2`、`completedFrames=2` 和 observed renderer passes 大于 0。

结论：

- frame entry 的第一批配置已经进入 RendererSubsystem API，而不是只停留在 application runner 内部。
- Engine 目录仍不依赖 `RuntimeFramePipelineConfig`，当前通过 engine-side frame config + application callback 维持依赖方向。
- 下一步可以继续把 pass plan/profile 的只读摘要或 key 传入 RendererSubsystem stats，或者把 runtime frame pipeline config 构建进一步集中到专门的 bridge adapter。

### 2026-05-27 RendererSubsystem Runtime Pipeline Pass Stats

本轮继续上一节的下一步，把 runtime frame pipeline 的 pass plan 执行统计接入 RendererSubsystem stats。目标是让 Engine-owned renderer subsystem 不只观察 renderer 内部 pass count，也能观察 application runtime pipeline 这一层的 planned / executed / skipped pass 数量。

新增与修改：

- `RuntimeFramePipeline::render(...)` 从 `void` 改为返回 `RuntimeFramePipelineStats`。
- `RuntimeFramePipelineStats` 当前记录 planned pass count、executed pass count、skipped pass count。
- `RuntimeFramePipeline::render(...)` 会统计 `RuntimeFramePassRegistry::buildPassPlan(...)` 生成的计划数量，执行 enabled pass 时递增 executed，遇到 null pass 或 disabled pass 时递增 skipped。
- `RendererSubsystemFrameRenderResult` 新增为 engine-side 桥接结果类型，避免 engine 直接依赖 application 的 `RuntimeFramePipelineStats`。
- `RendererSubsystem::renderFrameBridge(...)` 的 callback 现在返回 `RendererSubsystemFrameRenderResult`，subsystem 记录 runtimePipeline planned / executed / skipped pass 数。
- `RuntimeFrameRunner` 把 `RuntimeFramePipeline::render(...)` 的 stats 转成 `RendererSubsystemFrameRenderResult`。
- `RuntimePBRVerification` 的 `Runtime renderer subsystem stats` 新增 `runtimePipelinePasses=planned/executed/skipped`。
- `verify_pbr.ps1` 新增全局断言：planned runtime pipeline passes 必须大于 0，executed 必须大于 0，skipped 必须存在，并且 planned == executed + skipped。
- `DebugControllerPanel` 的 Renderer Subsystem 区块新增 Runtime Pipeline Passes 显示。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出并断言 `runtimePipelinePasses=4/4/0`，同时保持 RendererSubsystem frame bridge、frame config 和 renderer pass 断言。

结论：

- RendererSubsystem 已能观察 runtime pipeline pass plan 的执行摘要。
- 当前仍没有把 `RuntimeFramePassRegistry` 或 `RuntimeFramePipeline` 移入 engine，因此依赖方向仍然可控。
- 下一步可以继续把 pass plan/profile key 或 profile path 纳入 RendererSubsystem stats，或者抽一个 application-side `RendererFrameBridgeAdapter` 来进一步减少 `RuntimeFrameRunner` 中的 glue code。

### 2026-05-27 Runtime Renderer Frame Bridge Adapter

本轮接受 sidecar audit 的建议，先做 application-side bridge adapter，而不是先把 profile path / key 直接放进 Engine 侧 stats。目标是把 `RuntimeFramePipeline` 到 `RendererSubsystemFrameRenderResult` 的翻译逻辑从 `RuntimeFrameRunner` 中移走，保持 Runner 只负责 frame 顺序。

新增与修改：

- 新增 `RuntimeRendererFrameBridgeAdapter`，位于 application 层。
- `RuntimeRendererFrameBridgeAdapter::renderRuntimeFrame(...)` 调用旧 `RuntimeFramePipeline::render(...)`，并把 application-side `RuntimeFramePipelineStats` 转换为 engine-side `RendererSubsystemFrameRenderResult`。
- `RuntimeFrameRunner` 不再直接 include `RuntimeFramePipeline.h`，也不再手写 stats 字段映射；Engine/RendererSubsystem 路径和 fallback 路径都通过 adapter 调用 runtime render。
- VS 工程和 filters 已加入 `RuntimeRendererFrameBridgeAdapter.cpp/.h`。

结论：

- `RendererSubsystem` 接口暂时不扩大，Engine 目录仍不依赖 application runtime 类型。
- 后续如果需要暴露 `runtimePipelineProfileKey`，应从这个 adapter 继续向 `RendererSubsystemFrameRenderResult` 传递稳定 token，不建议暴露本地 profile path。

### 2026-05-27 RendererSubsystem Runtime Pipeline Profile Key

本轮继续上一节，把 runtime frame pipeline 的 profile/render intent 以稳定 token 形式传入 Engine-owned `RendererSubsystem`。目标是证明 application-side adapter 不只传递 pass count，也能传递“本帧 runtime pipeline 配置来源”的摘要，同时避免把本地配置文件路径做成 Engine 契约。

新增与修改：

- `RendererSubsystemFrameRenderResult` 新增 `runtimePipelineProfileKey`，默认值为 `none`。
- `RendererSubsystemFrameBridgeStats` 新增同名字段，并在 `applyRenderResult(...)` 中接收 render callback 返回的 key。
- `RuntimeRendererFrameBridgeAdapter` 根据当前 `RuntimeFramePipelineProfile` 生成稳定 token，格式为 `runtime-frame-pipeline:order=<sanitized-order>:enabled=<toggles>`。
- `RuntimePBRVerification` 的 `Runtime renderer subsystem stats` 新增 `runtimePipelineProfileKey=...`。
- `verify_pbr.ps1` 新增全局断言：`runtimePipelineProfileKey` 必须存在，且不能是空值或 `none`。
- `DebugControllerPanel` 的 Renderer Subsystem 区块新增 Runtime Pipeline Profile Key 显示。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出并断言 `runtimePipelineProfileKey=runtime-frame-pipeline:order=SceneColor+SceneResolve+Bloom+ScreenComposite:enabled=1111`，同时保持 `runtimePipelinePasses=4/4/0`。

结论：

- RendererSubsystem 已能观察 runtime pipeline 的 pass 执行摘要和稳定 profile key。
- Engine 仍不读取本地 profile path，也不依赖 application 的 `RuntimeFramePipelineProfile` 类型。
- 下一步可以继续把 RendererSubsystem bridge result 扩展成更正式的 render intent/result DTO，或者开始整理 renderer diagnostics 面板。

### 2026-05-27 RendererSubsystem Frame Intent / Result DTO

本轮继续上一节的下一步，把 `RendererSubsystem` bridge 的输入/输出类型从临时命名整理为正式 DTO。目标不是改变渲染行为，而是让 Engine 侧接口语义更清楚：进入一帧的是 intent，render callback 返回的是 result。

新增与修改：

- `RendererSubsystemFrameConfig` 重命名为 `RendererSubsystemFrameIntent`，当前仍包含 framebuffer width / height。
- `RendererSubsystemFrameRenderResult` 重命名为 `RendererSubsystemFrameResult`，当前包含 runtime pipeline profile key 和 planned / executed / skipped pass 统计。
- `RendererSubsystem::renderFrameBridge(...)`、`beginFrameBridge(...)`、`endFrameBridge(...)` 现在接收 `RendererSubsystemFrameIntent`。
- 内部 helper 从 `applyFrameConfig(...)` / `applyRenderResult(...)` 改为 `applyFrameIntent(...)` / `applyFrameResult(...)`。
- `RuntimeFrameRunner` 和 `RuntimeRendererFrameBridgeAdapter` 同步使用 intent/result DTO 命名。

结论：

- Engine 侧 renderer bridge 的入参和出参语义更接近正式 render intent/result 边界。
- verification 输出字段保持不变，避免 UI 和脚本契约无意义变动。
- 下一步可以在这个 DTO 边界上继续扩展 renderer diagnostics，或拆出独立 Engine diagnostics 面板。

### 2026-05-27 Engine Diagnostics Panel First Slice

本轮继续上一节的下一步，把 RendererSubsystem diagnostics 从 `DebugControllerPanel` 中拆出到独立 Engine diagnostics 面板文件。目标是降低 debug controller 的职责膨胀，让 Engine-owned subsystem 的可观察 UI 有自己的入口。

新增与修改：

- 新增 `EngineDiagnosticsPanel.h/.cpp`。
- 新增 `EngineDiagnosticsContext`，初始只包含 `const RendererSubsystem* rendererSubsystem`。
- 原 `drawRendererSubsystemStats(...)` 逻辑迁入 `EngineDiagnosticsPanel.cpp`，UI 文案和显示字段保持不变。
- `DebugControllerPanel.cpp` 只构造 `EngineDiagnosticsContext` 并调用 `drawEngineDiagnosticsPanel(...)`，不再直接读取 `RendererSubsystem::getFrameBridgeStats()`。
- VS 工程和 filters 已注册新增 diagnostics panel 文件。

结论：

- RendererSubsystem debug UI 已从通用 DebugController 中拆出第一层边界。
- 后续 Engine diagnostics 可以继续接入 AssetSubsystem / World / Engine tick 等统计，而不继续扩大 DebugControllerPanel。

### 2026-05-27 Engine Diagnostics Unified Context

本轮继续上一节，把 Engine diagnostics panel 从单一 RendererSubsystem 观察入口扩展为 Engine / World / AssetSubsystem / RendererSubsystem 的统一基础诊断入口。目标是让后续 runtime 状态观察不继续塞回 DebugControllerPanel。

新增与修改：

- `AppRuntimeContext` 新增非拥有 `GLengine::Engine* engine`，与 `engineWorld` / `assetSubsystem` / `rendererSubsystem` 保持同一类 runtime context 引用边界。
- `RuntimeApplicationShell` 初始化时写入 `mRuntime.engine = &mEngine`，cleanup 时清空。
- `DebugControllerContext` 与 `EngineDiagnosticsContext` 新增 engine、engineWorld、assetSubsystem 指针。
- `RuntimeEditorPanelCoordinator` 把 runtime context 中的 Engine / World / AssetSubsystem 传给 debug controller，再由 debug controller 转交 Engine diagnostics panel。
- `EngineDiagnosticsPanel` 新增 `Engine Runtime`、`Engine World` 和 `Asset Subsystem` 折叠区，显示 Engine initialized / run mode / viewport / time / delta、World playing / level actor count、AssetRegistry 总量和主要 kind 计数。
- `RuntimePBRVerification` 的 `Runtime engine tick stats` 新增 `runtimeContextEngineAttached=yes`。
- `verify_pbr.ps1` 新增断言：每个 verification mode 的 `AppRuntimeContext` 必须暴露 runtime Engine。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出并断言 `runtimeContextEngineAttached=yes`，同时保持 renderer subsystem verification stats。

结论：

- Engine diagnostics panel 已从 RendererSubsystem 单点扩展为 Engine runtime 状态的统一观察入口。
- verification 现在能证明 AppRuntimeContext 中的 Engine 指针来自 shell 持有的真实 Engine，而不是只靠 UI 编译间接证明。

### 2026-05-27 Goal Restart / Subagents Coordination

本轮按 active goal 继续推进，不重新定义目标，也不把当前状态标记为完成。当前项目已经进入 Engine-owned runtime pipeline 的边界收敛阶段，下一步不应继续扩张 PBR 特性，而应优先选择能降低耦合、能被 verification 稳定证明的 engine slice。

本轮协作边界：

- 父 agent 继续负责实际实现、验证、文档同步和 `worked.md` 记录。
- 子 agent 只做只读审查，不编辑文件，不处理 `imgui.ini`，不执行 destructive git。
- `Engine Diagnostics Health Counter Agent` 审查下一步 subsystem health counter 与稳定 verification 断言。
- `Renderer Frame Contract Boundary Agent` 审查 `RendererSubsystemFrameIntent` / `RendererSubsystemFrameResult` 下一步最小 contract 改进。

当前倾向：

- 如果 diagnostics 审查确认稳定，优先做 subsystem health counters，因为这能加强 Engine runtime 的可观察性，同时不扩大 renderer/PBR 范围。
- 如果 renderer contract 审查指出已有 DTO 仍缺少必要的 engine-side intent/result 字段，则只补通用 frame contract 字段，避免把 legacy pipeline 的 pass 细节继续上移到 Engine。

### 2026-05-27 Renderer Frame Contract Neutralization

本轮采纳 `Renderer Frame Contract Boundary Agent` 的只读审查结论：Engine 侧 DTO / stats 不应继续使用 `runtimePipeline*` 字段名。pass count 和 plan key 本身是合理的 frame contract，但 `RuntimeFramePipeline` 是 application legacy 管线，不应该成为 Engine 命名空间的抽象。

新增与修改：

- `RendererSubsystemFrameResult` 与 `RendererSubsystemFrameBridgeStats` 把 `runtimePipelineProfileKey` 改为 `framePlanKey`。
- planned / executed / skipped 统计改为 `plannedPassCount`、`executedPassCount`、`skippedPassCount`。
- `RuntimePBRVerification` 的 `Runtime renderer subsystem stats` 输出改为 `framePlanKey=...` 与 `framePasses=planned/executed/skipped`。
- `verify_pbr.ps1` 同步改为断言 `framePlanKey` 非空且不为 `none`，并继续断言 `planned == executed + skipped`。
- `EngineDiagnosticsPanel` UI 改为显示 `Frame Plan Key` 和 `Frame Passes`，不再在 Engine diagnostics 中暴露 `Runtime Pipeline` 命名。

结论：

- 旧 `RuntimeFramePipeline` 的具体来源仍保留在 application-side `RuntimeRendererFrameBridgeAdapter` 内部。
- Engine 侧只表达通用 frame intent/result，不表达 legacy runtime pipeline 类型。
- 下一步可按另一个子 agent 建议推进 Engine-driven subsystem health counters。

### 2026-05-27 Engine-Driven Subsystem Health Counters

本轮采纳 `Engine Diagnostics Health Counter Agent` 的只读审查结论：在继续拆 renderer ownership 之前，先补一个不依赖 PBR 场景细节的 Engine health contract。目标是证明 Engine tick loop 正在统一驱动 active World 和 Engine-owned subsystems，而不是只靠 renderer bridge stats 间接证明。

新增与修改：

- `Engine` 新增 tick counter，并在 `Engine::tick(...)` 中自增。
- `World` 新增 tick counter，并在 `World::tick(...)` 中自增，`beginPlay()` 时重置。
- `AssetSubsystem` 新增 tick counter，`tick(...)` 中自增，initialize/shutdown 时重置。
- `RendererSubsystem` 新增 tick counter，`tick(...)` 中自增，initialize/shutdown 时重置。
- `EngineDiagnosticsPanel` 新增 `Subsystem Health` 折叠区，显示 Engine / World / AssetSubsystem / RendererSubsystem tick count、context world 是否匹配 active world、World actor count。
- `RuntimePBRVerification` 新增 `Runtime subsystem health stats` 输出：`engineTicks`、`worldTicks`、`assetSubsystemTicks`、`rendererSubsystemTicks`、`contextWorldMatchesActive`、`worldActors`。
- `verify_pbr.ps1` 新增全局断言：四个 tick counter 必须大于 0 且相等；`contextWorldMatchesActive=yes`；health `worldActors` 不能低于 scene stats 的 `runtimeWorldActors`，允许后续 package/editor probes 在 scene stats 输出后追加 Actor。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出并断言 `engineTicks=2, worldTicks=2, assetSubsystemTicks=2, rendererSubsystemTicks=2`，且 `worldActors` 分别为 33 和 5。

结论：

- Engine tick loop 现在有可验证的统一驱动证据。
- 这个 contract 不绑定 PBR pass、灯光数量或具体渲染场景，适合作为后续 engine runtime ownership 的基础健康检查。

### 2026-05-27 Renderer Frame Executor Interface

本轮继续上一节的下一步，把 `RendererSubsystem::renderFrameBridge(...)` 的渲染入口从 `std::function` callback 收敛为 Engine-side executor interface。目标是让 Engine 侧 renderer bridge 不再只是“调用一个任意 lambda”，而是依赖明确的 `RendererFrameExecutor` 抽象；application 层 adapter 负责实现这个接口并调用旧 `RuntimeFramePipeline`。

新增与修改：

- `RendererSubsystem.h` 新增 `RendererFrameExecutor` interface，定义 `renderFrame(const EngineContext&, const RendererSubsystemFrameIntent&) -> RendererSubsystemFrameResult`。
- `RendererSubsystem::renderFrameBridge(...)` 第三个参数改为 `RendererFrameExecutor&`，内部通过 executor 执行 frame render，并继续负责 begin/end bridge lifecycle 和异常时 end 记录。
- `RendererSubsystemFrameBridgeStats` 新增 `frameExecutorAttached` 与 `frameExecutorCallCount`，用于证明这一帧确实通过 executor interface 进入 renderer。
- `RuntimeRendererFrameBridgeAdapter` 现在继承 `RendererFrameExecutor`，持有 non-owning `AppRuntimeContext&`，把 engine-side frame intent 转换为旧 `RuntimeFramePipeline::render(...)` 调用。
- `RuntimeFrameRunner` 不再传 lambda 给 RendererSubsystem，而是在 stack 上创建 `RuntimeRendererFrameBridgeAdapter frameExecutor(context)` 并交给 `renderFrameBridge(...)`。
- `EngineDiagnosticsPanel` 显示 Frame Executor Attached / Frame Executor Calls。
- `RuntimePBRVerification` 与 `verify_pbr.ps1` 新增 `frameExecutorAttached=yes` 和 `frameExecutorCalls=N` 输出 / 断言，并要求 `frameExecutorCalls == renderFrameBridgeCalls`。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出并断言 `frameExecutorAttached=yes`、`renderFrameBridgeCalls=2`、`frameExecutorCalls=2`。

结论：

- Engine 侧 renderer frame bridge 已有第一个 backend/executor 抽象，不再直接接收 application lambda。
- 旧 `RuntimeFramePipeline` 仍留在 application adapter 内部，Engine 只依赖 `RendererFrameExecutor`、frame intent 和 frame result。

### 2026-05-27 Renderer Executor Attachment Boundary

本轮继续上一节，把 executor 从“每帧传入 `renderFrameBridge(...)`”上移为 `RendererSubsystem` 的非拥有 attachment。目标是让 RendererSubsystem 拥有稳定的 backend/executor 连接状态，`RuntimeFrameRunner` 每帧只提交 frame intent，不再负责构造或传递 executor。

新增与修改：

- `RendererSubsystem` 新增 `setFrameExecutor(...)`、`getFrameExecutor()`、`hasFrameExecutor()`，并持有非拥有 `RendererFrameExecutor*`。
- `RendererSubsystem::renderFrameBridge(...)` 现在只接收 `EngineContext` 与 `RendererSubsystemFrameIntent`，内部调用已附着的 executor。
- `RuntimeApplicationShell` 新增成员 `RuntimeRendererFrameBridgeAdapter mRendererFrameExecutor{ mRuntime }`，初始化 renderer subsystem 后调用 `setFrameExecutor(&mRendererFrameExecutor)`，cleanup 时清空。
- `RuntimeFrameRunner` 不再每帧创建 `RuntimeRendererFrameBridgeAdapter`，只在 Engine / RendererSubsystem / executor 都可用时调用 `renderFrameBridge(...)`；否则保留旧 direct render fallback。
- 现有 `frameExecutorAttached` / `frameExecutorCalls` verification contract 保持不变，但现在证明的是 subsystem attachment，而不是单次函数参数。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出并断言 `frameExecutorAttached=yes`、`renderFrameBridgeCalls=2`、`frameExecutorCalls=2`。

结论：

- RendererSubsystem 现在具备稳定 executor attachment 边界，更接近可替换 renderer backend contract。
- Application runtime pipeline 仍被限制在 `RuntimeRendererFrameBridgeAdapter` 内部，Engine 不直接依赖 application runtime 类型。

### 2026-05-27 Renderer Backend Metadata Contract

本轮继续 executor attachment boundary，但不扩张 PBR 功能。目标是让 Engine-owned `RendererSubsystem` 不只知道“有 executor”，还知道当前 renderer backend 的身份和是否真正 ready。

新增与修改：

- `RendererFrameExecutor` 新增 `getBackendKey()` 与 `isBackendReady()`，形成第一版 renderer backend metadata contract。
- `RendererSubsystemFrameBridgeStats` 新增 `rendererBackendReady` 与 `rendererBackendKey`，`setFrameExecutor(...)` 和 frame stats refresh 都会同步 backend metadata。
- `RuntimeRendererFrameBridgeAdapter` 返回稳定 key `runtime-frame-pipeline-adapter`，并按当前启用的 runtime frame pass 检查 renderer、scene、camera、frame targets、bloom、screen quad 等硬依赖。
- `RendererSubsystem::renderFrameBridge(...)` 只在 executor 已附着且 backend ready 时调用 executor；否则记录默认 frame result。
- `EngineDiagnosticsPanel` 在 RendererSubsystem frame bridge 区块显示 `Renderer Backend Ready` 与 `Renderer Backend Key`。
- `RuntimePBRVerification` 输出 `rendererBackendReady` / `rendererBackendKey`，`verify_pbr.ps1` 断言 backend ready 且 key 非 `none`，但不绑定具体 key 值，给后续替换 backend 留空间。
- `RendererSubsystem::setRenderer(...)` 同步基础 renderer stats，减少初始化后首帧前 diagnostics 的短暂滞后。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出并断言 `rendererBackendReady=yes` 与非空 `rendererBackendKey`。

结论：

- 这一步把 executor attachment 推进为 backend metadata contract 的第一刀，但旧 `RuntimeFramePipeline` 仍然留在 application adapter 内部。
- 当前没有新增 shader、pass、材质或 PBR 场景功能；它只服务于后续可替换 renderer backend 的 Engine 边界。

### 2026-05-27 Renderer Backend Lifecycle Stats

本轮继续 renderer backend contract，从 metadata 扩展到可观察 lifecycle。目标不是替换 renderer backend，而是先让 Engine 侧能稳定证明 backend 的 attach、ready、detached 状态和每帧参与情况。

新增与修改：

- `RendererSubsystemFrameBridgeStats` 新增 `rendererBackendState`、`rendererBackendAttachCount`、`rendererBackendDetachCount`、`rendererBackendReadyFrameCount`、`rendererBackendNotReadyFrameCount`。
- `RendererSubsystem::setFrameExecutor(...)` 现在只在 executor 指针变化时记录 attach / detach count，并刷新 backend key / state / ready。
- `RendererSubsystem::shutdown(...)` 通过 `setFrameExecutor(nullptr)` 清理 backend attachment，避免 shutdown 绕过 lifecycle 计数。
- `RendererSubsystem::renderFrameBridge(...)` 每次 frame bridge 都记录 backend ready 或 not-ready frame count；当前正常 runtime path 要求 ready frame count 等于 render bridge call count。
- `EngineDiagnosticsPanel` 在 RendererSubsystem frame bridge 区块显示 backend state、attach/detach count、ready/not-ready frame count。
- `RuntimePBRVerification` 输出 backend lifecycle 字段，`verify_pbr.ps1` 断言 state 为 `ready`、attach count 大于 0、not-ready frames 为 0，并校验 ready/not-ready frame 总数与 render bridge call count 一致。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出 `rendererBackendState=ready`、`rendererBackendAttachCount=1`、`rendererBackendReadyFrames=2`、`rendererBackendNotReadyFrames=0`。

结论：

- RendererSubsystem 现在不只知道 backend 是否 ready，还能记录 backend attachment lifecycle 和每帧 readiness 参与情况。
- 这一步仍不扩张 PBR；它为后续替换 backend 或引入 backend lifecycle owner 提供 verification contract。

### 2026-05-27 Runtime Renderer Backend Factory

本轮继续上一节，从“可观察 lifecycle”推进到“application 侧可替换 backend owner / factory”。目标是先把旧 runtime pipeline adapter 从 `RuntimeApplicationShell` 的固定成员对象中移出，改为由 factory 创建、由 shell 通过 `std::unique_ptr<RendererFrameExecutor>` 拥有。

新增与修改：

- 新增 `RuntimeRendererBackendFactory`，集中创建 `RuntimeRendererFrameBridgeAdapter`，返回 engine-side `std::unique_ptr<RendererFrameExecutor>`。
- `RuntimeApplicationShell` 不再直接持有 `RuntimeRendererFrameBridgeAdapter mRendererFrameExecutor`，改为持有 `std::unique_ptr<RendererFrameExecutor> mRendererBackend`。
- `RuntimeApplicationShell::initialize()` 通过 factory 创建 runtime frame pipeline backend，并附着到 `RendererSubsystem`。
- `RuntimeApplicationShell::cleanup()` 先从 `RendererSubsystem` detach backend，再 reset shell-owned backend，最后清理 camera/runtime context。
- 新增 `RendererBackendAttachmentDesc`，`RendererSubsystem::setFrameExecutor(...)` 接收 backend owner key 与 ownership metadata。
- `RendererSubsystemFrameBridgeStats` 新增 `rendererBackendOwnerKey` 与 `rendererBackendOwnership`，diagnostics 和 verification 均可观察。
- `verify_pbr.ps1` 断言 runtime path 中 backend owner key 非 `none`，并且 ownership 为 `application-owned`。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出 `rendererBackendOwnerKey=runtime-application-shell` 与 `rendererBackendOwnership=application-owned`。

结论：

- 当前 runtime renderer backend 已经由 factory 创建、由 shell 显式拥有，再以 engine-side executor 接口附着给 RendererSubsystem。
- 这一步仍然不新增 renderer pass 或 PBR 功能；它只是把 backend owner/factory 边界从硬编码成员对象中拆出来，为后续多 backend 选择或 Engine-owned backend owner 做准备。

### 2026-05-27 Runtime Renderer Backend Registry

本轮继续上一节，从单一 factory 创建函数推进到可选择 backend registry。当前仍然只注册一个 backend，但 shell 已经按 key 从 registry 创建 backend，后续添加新 backend 不需要再改 shell 的持有方式。

新增与修改：

- `RuntimeRendererBackendFactory` 新增 `RuntimeRendererBackendRegistration`、`defaultBackendKey()`、`registeredBackends()`、`isRegisteredBackendKey(...)` 和 `createBackend(context, backendKey)`。
- `RuntimeApplicationShellConfig` 新增 `rendererBackendKey`，默认选择 `runtime-frame-pipeline-adapter`。
- `RuntimeApplicationShell::initialize()` 改为通过 `createBackend(mRuntime, mConfig.rendererBackendKey)` 创建 backend；未知 key 会返回 `nullptr` 并让初始化失败。
- `RendererBackendAttachmentDesc` 新增 `registryKey` 与 `registryBackendCount`，RendererSubsystem stats 对应新增 `rendererBackendRegistryKey` 与 `rendererBackendRegistryCount`。
- `EngineDiagnosticsPanel` 显示 backend registry key 和 registry count。
- `RuntimePBRVerification` 输出 registry 字段，`verify_pbr.ps1` 断言 registry key 非 `none`、registry count 大于 0，并要求 registry key 与实际 selected backend key 一致。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene -DiscardCaptures`：构建通过；两个 mode 均输出 `rendererBackendRegistryKey=runtime-frame-pipeline-adapter` 与 `rendererBackendRegistryCount=1`。

结论：

- Runtime renderer backend 的选择入口已经从硬编码 factory 调用推进为 key-based registry。
- 这一步仍然保持 single backend，但架构上已经允许下一步添加 no-op/test backend、OpenGL backend wrapper 或后续 Engine-owned backend owner。

### 2026-05-27 Renderer Backend Registry No-op Verification

本轮继续上一节，把 registry 从“只有一个可选项”推进到“可以切换到第二个 backend 并被 verification 证明”。新增 backend 是 test/no-op renderer backend：它不调用旧 `RuntimeFramePipeline`，只清默认 framebuffer 并返回 engine-side neutral frame result。

新增与修改：

- 新增 `RuntimeNoOpRendererBackend`，实现 `RendererFrameExecutor`，稳定 key 为 `test-noop-renderer-backend`。
- `RuntimeNoOpRendererBackend::renderFrame(...)` 只绑定默认 framebuffer、设置 viewport、清成固定非黑色，并返回 `framePlanKey=test-noop-renderer-backend:clear` 与 `framePasses=1/1/0`。
- `RuntimeRendererBackendFactory` 注册第二个 backend，并新增 `testNoOpBackendKey()`；默认 backend 仍是 `runtime-frame-pipeline-adapter`。
- `RuntimePBRVerificationArgs` 新增 `--verify-renderer-backend-registry-noop`，该 mode 设置 `RuntimeApplicationShellConfig::rendererBackendKey` 为 no-op backend key，capture 写入 `out/renderer_backend_registry_noop_verification.ppm`。
- `verify_pbr.ps1` 新增 `renderer-backend-registry-noop` mode，并对该 mode 精确断言 backend key、registry key、registry count、frame plan key、frame pass count、legacy renderer pass count 与 capture 非黑比例。
- 子 agent `Pauli` 的只读审查建议已采纳关键方向：backend/registry key 精确断言、registry count 至少 2、legacy renderer pass 为 0。实现上没有采纳黑屏/`0/0/0` pass 建议，而是采用非黑色 clear 和 `1/1/0` neutral pass，用来证明 no-op executor 确实执行过。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes renderer-backend-registry-noop -DiscardCaptures`：构建通过；mode 输出 `rendererBackendKey=test-noop-renderer-backend`、`rendererBackendRegistryCount=2`、`framePlanKey=test-noop-renderer-backend:clear`、`framePasses=1/1/0`、`observedRendererPasses=0`、`rendererPasses=0`、capture `nonblack=100%`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过；原 33 个 mode 继续选择 `runtime-frame-pipeline-adapter`，新增 no-op mode 选择 `test-noop-renderer-backend`。
- `git diff --check`：没有 whitespace error；仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- Renderer backend registry 现在不再只是单 backend facade，已经有第二个可选择 backend，并且 verification 证明它不依赖旧 runtime renderer pipeline。
- 这一步仍不扩张 PBR；它只把 renderer backend contract 从 metadata/registry 推进到可替换执行路径。

### 2026-05-27 Renderer Backend Engine-owned Ownership

本轮继续上一节，把 renderer backend 的实际生命周期从 `RuntimeApplicationShell` 继续向 Engine-owned `RendererSubsystem` 收敛。保留 application 层 factory 是因为具体 backend 仍依赖 application runtime context，但 factory 创建出的 `std::unique_ptr<RendererFrameExecutor>` 会立即移交给 `RendererSubsystem` 持有。

新增与修改：

- `RendererSubsystem` 的 executor attachment 从非拥有 `RendererFrameExecutor*` 改为拥有 `std::unique_ptr<RendererFrameExecutor>`。
- `RendererSubsystem::setFrameExecutor(...)` 现在接收 `std::unique_ptr<RendererFrameExecutor>`，并继续记录 attach/detach、backend key、ready state、registry metadata。
- 新增 `RendererSubsystem::clearFrameExecutor()`，`RendererSubsystem::shutdown(...)` 和 `RuntimeApplicationShell::cleanup()` 通过该 API 清理 owned backend。
- `RuntimeApplicationShell` 不再持有 `mRendererBackend` 成员；initialize 阶段只创建局部 backend，并 `std::move` 移交给 Engine-owned `RendererSubsystem`。
- `RuntimeRendererBackendFactory::makeRendererSubsystemAttachmentDesc(...)` 现在报告 owner key 为 `engine-renderer-subsystem`，ownership 为 `engine-owned`。
- `verify_pbr.ps1` 全局断言 renderer backend owner key 必须是 `engine-renderer-subsystem`，ownership 必须是 `engine-owned`。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop -DiscardCaptures`：构建通过；legacy runtime backend 与 no-op backend 两条路径均输出 `rendererBackendOwnerKey=engine-renderer-subsystem` 与 `rendererBackendOwnership=engine-owned`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过；所有 mode 均输出并断言 `rendererBackendOwnerKey=engine-renderer-subsystem` 与 `rendererBackendOwnership=engine-owned`。
- `git diff --check`：没有 whitespace error；仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- Renderer backend 现在由 Engine-owned `RendererSubsystem` 实际持有，`RuntimeApplicationShell` 只承担 backend creation / handoff 的 composition root 职责。
- 这一步没有改变 PBR pass 或 shader，只收敛 renderer backend ownership，减少 application shell 对 renderer backend 生命周期的长期耦合。

### 2026-05-27 Renderer Backend Cleanup Verification

本轮继续上一节，补齐 Engine-owned renderer backend 的 cleanup verification。前一轮已经证明 backend 由 `RendererSubsystem` 持有并参与 frame，但 verification 只观察了 rendered-frame 时的 attached/ready 状态，没有证明 `RuntimeApplicationShell::cleanup()` 会在 Engine shutdown 前确定性 detach backend。

新增与修改：

- `RuntimePBRVerification` 新增 `reportRendererSubsystemCleanup(...)`，输出 verification-only `Runtime renderer subsystem cleanup stats`。
- `RuntimeApplicationShell::cleanup()` 在 `mRendererSubsystem->clearFrameExecutor()` 后、清空 runtime context 前调用 cleanup report，确保报告能看到 context 仍指向同一个 Engine-owned `RendererSubsystem`。
- `verify_pbr.ps1` 新增全局 cleanup 断言：每个 verification mode 必须输出 cleanup stats，且 cleanup 时 `frameExecutorAttached=no`、`rendererBackendReady=no`、`rendererBackendState=detached`、backend key / owner / registry 均已清空。
- `verify_pbr.ps1` 还断言 cleanup attach/detach count 均大于 0 且相等，证明本轮不是只清空字符串，而是真正记录了 backend detach lifecycle。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop -DiscardCaptures`：构建通过；legacy runtime backend 与 no-op backend 两条路径均输出 cleanup stats，且 `rendererBackendDetachCount=1`、`rendererBackendState=detached`、`frameExecutorAttached=no`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过；所有 mode 均输出并断言 `Runtime renderer subsystem cleanup stats`，且 cleanup 阶段 backend detached、metadata cleared、attach/detach count 闭合。
- `git diff --check`：没有 whitespace error；仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- Renderer backend lifecycle 现在覆盖 attach -> ready frame execution -> cleanup detach 三段。
- 这一步继续保持不扩张 PBR；它只强化 Engine-owned backend lifecycle 的可验证性。

### 2026-05-27 Engine World Cleanup Verification

本轮从 renderer backend lifecycle 转向 Engine-owned World / runtime context cleanup contract。目标不是继续增加 renderer 或 PBR 功能，而是证明 `RuntimeApplicationShell::cleanup()` 后，Engine-owned active World 已经由 `Engine::shutdown()` 结束并 reset，runtime context 中的 Engine / World / subsystem 非拥有指针也已经清空。

新增与修改：

- `RuntimePBRVerification` 新增 `reportEngineWorldCleanup(...)`，输出 verification-only `Runtime engine world cleanup stats`。
- `RuntimeApplicationShell::cleanup()` 保持现有 renderer cleanup report 的观察窗口不变：先 detach renderer backend 并报告 `Runtime renderer subsystem cleanup stats`，再清理 camera 和 runtime context raw pointers，然后执行 `mEngine.shutdown()`。
- `RuntimeApplicationShell::cleanup()` 在 `mEngine.shutdown()` 之后报告 Engine World cleanup，确保 report 看到的是 post-shutdown 状态：`engineInitialized=no`、`activeWorld=no`、runtime context raw pointers 均为空。
- `verify_pbr.ps1` 捕获 `Runtime engine world cleanup stats`，写入 summary，并对所有 verification mode 增加全局断言：Engine 已 shutdown、active World 已 reset、runtime context engine/world/asset subsystem/renderer subsystem 指针均已清空，且 engine tick/time/delta 保留为已运行状态。
- 只读子 agent `Hubble` 审查后未修改文件；采纳其建议，把 World cleanup report 独立于 renderer cleanup report，并增加显式 `runtimeContext*Null=yes` 字段，避免只用“不等于当前对象”代替空指针断言。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条路径均输出 `Runtime engine world cleanup stats`，且 `engineInitialized=no`、`activeWorld=no`、`runtimeContextEngineNull=yes`、`runtimeContextWorldNull=yes`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过；所有 mode 均输出并断言 Engine World cleanup contract。
- `git diff --check`：没有 whitespace error；仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- Engine-owned World lifecycle 现在覆盖 rendered-frame active/playing 状态和 cleanup 后 shutdown/reset 状态。
- 这一步继续保持不扩张 PBR；它把 runtime ownership 从“frame 中可观察”推进到“cleanup 后可证明闭合”。

### 2026-05-27 Engine Subsystem Cleanup Verification

本轮继续上一节，把 post-cleanup contract 从 active World 扩展到 Engine-owned subsystems。目标是证明 `Engine::shutdown()` 不只是 reset World，也会让 `AssetSubsystem` 和 `RendererSubsystem` 进入确定性的 shutdown 状态，同时 runtime context 不再保留指向这些 subsystem 的非拥有指针。

新增与修改：

- `RuntimePBRVerification` 新增 `reportEngineSubsystemCleanup(...)`，输出 verification-only `Runtime engine subsystem cleanup stats`。
- `RuntimeApplicationShell::cleanup()` 在清空 runtime context 和执行 `mEngine.shutdown()` 前保存 Engine-owned subsystem 的只读 raw pointer；shutdown 后再报告 subsystem 状态。这样不会让 runtime context 保留悬空引用，同时仍能检查 Engine 持有的 subsystem 对象。
- `Runtime engine subsystem cleanup stats` 检查 `AssetSubsystem`：post-shutdown 仍可被 Engine 持有、`assetSubsystemInitialized=no`、`assetSubsystemTicks=0`、`assetRegistryAssets=0`。
- `Runtime engine subsystem cleanup stats` 检查 `RendererSubsystem`：post-shutdown 仍可被 Engine 持有、`rendererSubsystemInitialized=no`、`rendererSubsystemTicks=0`、`rendererHasRenderer=no`、`rendererFrameExecutorAttached=no`、backend key / owner / registry metadata 已清空，且 attach/detach count 仍闭合。
- `verify_pbr.ps1` 捕获 `Runtime engine subsystem cleanup stats`，写入 summary，并对所有 verification mode 增加全局断言。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条路径均输出 `Runtime engine subsystem cleanup stats`，且 AssetSubsystem / RendererSubsystem post-shutdown 状态符合预期。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过；所有 mode 均输出并断言 subsystem cleanup contract。
- `git diff --check`：没有 whitespace error；仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- Engine runtime lifecycle 现在覆盖 frame 中 Engine / World / subsystem 同步 tick、renderer backend cleanup、World shutdown/reset、subsystem shutdown/reset。
- 这一步继续保持不扩张 PBR；它强化的是 Engine-owned subsystem 生命周期闭合证据。

### 2026-05-27 Runtime Engine Lifecycle Extraction

本轮继续缩小 `RuntimeApplicationShell` 的 composition root，把 Engine / subsystem / renderer backend 生命周期编排抽到独立 `RuntimeEngineLifecycle`。目标不是改变渲染行为，而是让 shell 不再直接负责创建 AssetSubsystem、RendererSubsystem、附着 backend、清理 runtime context 和 shutdown Engine 的细节。

新增与修改：

- 新增 `RuntimeEngineLifecycle.h/.cpp`，定义 `RuntimeEngineLifecycleState` 和 `RuntimeEngineLifecycleCleanupRefs`。
- `RuntimeEngineLifecycle::initializeEngine(...)` 负责把 `Engine` 写入 runtime context、创建 Engine-owned `AssetSubsystem` / `RendererSubsystem`、把非拥有 subsystem 指针写入 runtime context，并调用 `Engine::initialize(...)`。
- `RuntimeEngineLifecycle::attachRendererBackend(...)` 负责把现有 runtime renderer 绑定到 Engine-owned `RendererSubsystem`，并通过 `RuntimeRendererBackendFactory` 创建/附着 selected backend。
- `RuntimeEngineLifecycle::beginCleanup(...)` 负责保存 post-shutdown verification 所需的 Engine-owned subsystem 只读指针，并先 detach renderer backend，保留原有 `Runtime renderer subsystem cleanup stats` 的观察窗口。
- `RuntimeEngineLifecycle::detachRuntimeContext(...)` 负责清空 runtime context 中 Engine / World / AssetSubsystem / RendererSubsystem 非拥有指针，并清空 lifecycle state。
- `RuntimeEngineLifecycle::shutdownEngine(...)` 负责调用 `Engine::shutdown()`。
- `RuntimeApplicationShell` 不再持有 `mAssetSubsystem` / `mRendererSubsystem` 两个裸成员，改为持有 `RuntimeEngineLifecycleState mEngineLifecycle`；frame config 与 verification report 都从该 state 读取 renderer subsystem。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 lifecycle 源文件和头文件。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeEngineLifecycle.cpp` 已参与编译，三条路径 runtime lifecycle 输出保持一致。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- `git diff --check`：没有 whitespace error；仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- `RuntimeApplicationShell` 的 Engine lifecycle 细节继续外移，shell 更接近 runtime composition coordinator，而不是直接管理每个 Engine-owned subsystem 的生命周期细节。
- 这一步继续保持不扩张 PBR；它是 application composition root 降耦合。

### 2026-05-27 Runtime Verification Lifecycle Extraction

本轮继续缩小 `RuntimeApplicationShell` 的 composition root，把 verification mode 的启动、scene report、capture、cleanup report 和 frame-count stop 条件集中到 `RuntimeVerificationLifecycle`。目标是让 shell 不再直接编排 `RuntimePBRVerification::*` 的细节，只按 runtime lifecycle 阶段调用 verification wrapper。

新增与修改：

- 新增 `RuntimeVerificationLifecycle.h/.cpp`，作为 application 层 verification lifecycle adapter。
- `RuntimeVerificationLifecycle::applyStartupProfile(...)` 负责 verification enabled 时调用 `RuntimePBRVerification::applyProfile(...)`。
- `RuntimeVerificationLifecycle::reportPreparedScene(...)` 负责 verification scene probe、renderer pass profile 应用和 prepared scene stats 输出。
- `RuntimeVerificationLifecycle::captureFrameIfNeeded(...)` 负责 capture frame gate、default framebuffer capture 和 rendered frame stats 输出。
- `RuntimeVerificationLifecycle::reportRendererSubsystemCleanup(...)` 与 `reportEngineCleanup(...)` 负责保持现有 cleanup verification 输出窗口。
- `RuntimeVerificationLifecycle::shouldStopAfterFrames(...)` 负责 verification max frame stop 条件。
- `RuntimeApplicationShell` 不再直接调用 `RuntimePBRVerification::*`；shell 只调用 `RuntimeVerificationLifecycle`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 verification lifecycle 源文件和头文件。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeVerificationLifecycle.cpp` 已参与编译，三条路径 verification 输出保持一致。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- `git diff --check`：没有 whitespace error；仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- `RuntimeApplicationShell` 的 verification 细节继续外移，shell 更接近生命周期协调器。
- 这一步继续保持不扩张 PBR；它只降低 application shell 对 verification/PBR test surface 的直接耦合。

### 2026-05-27 Runtime Content Lifecycle Extraction

本轮继续缩小 `RuntimeApplicationShell` 的 composition root，把 startup content composition 抽到 `RuntimeContentLifecycle`。目标不是改变任何渲染场景，而是把 camera/profile/verification startup/scene prepare/backend attach/prepared scene report 这条启动内容链路集中管理，避免 shell 继续直接知道每个启动细节。

新增与修改：

- 新增 `RuntimeContentLifecycle.h/.cpp`，定义 `RuntimeContentLifecycleConfig` 和 `RuntimeContentLifecycle::prepare(...)`。
- `RuntimeContentLifecycle::prepare(...)` 现在统一执行 camera 初始化、runtime profile load、verification startup profile、scene prepare、renderer backend attach 和 prepared scene report。
- `RuntimeApplicationShell::initialize()` 不再直接调用 `RuntimeProfileLoader::loadAll(...)`、`RuntimeScenePreparer::prepare(...)`、`RuntimeEngineLifecycle::attachRendererBackend(...)`、`RuntimeVerificationLifecycle::applyStartupProfile(...)` 或 `reportPreparedScene(...)`。
- `RuntimeApplicationShell` 只保留 `makeContentLifecycleConfig()`，负责把现有 config 聚合成 content lifecycle config；这是后续继续下沉 verification-to-content policy 的剩余轻耦合点。
- `RuntimeContentLifecycle::prepare(...)` 在 scene prepare 后新增 fail-fast gate：如果 `context.renderer` 仍为空，则记录 `LogError` 并停止 backend attach，避免异常启动状态被延迟到 frame 阶段才暴露。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 content lifecycle 源文件和头文件。
- 本轮使用只读 sidecar subagent `Tesla` 审查 content lifecycle ordering；该 agent 未修改文件。采纳其 fail-fast 建议，保留其“后续可继续下沉 pbrVerification -> content config 映射”的建议作为下一步候选。

已完成验证：

- 静态耦合检查确认 `RuntimeApplicationShell` 只保留 `RuntimeContentLifecycle::prepare(...)` 与 `makeContentLifecycleConfig()`，直接 scene/profile/backend/verification startup 调用已集中到 `RuntimeContentLifecycle.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,deferred,engine-world-scene-probe,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationShell` 的 startup content 细节继续外移，shell 更接近高层 runtime coordinator。
- 这一步继续保持不扩张 PBR；它只收敛启动内容生命周期和异常启动状态的 fail-fast 边界。

### 2026-05-27 Runtime Content Config Policy Extraction

本轮继续上一节，把 `RuntimeApplicationShell` 中最后一段明显的 startup content config policy 下沉。上一轮 shell 已不再直接执行 camera/profile/scene/backend/verification startup，但仍在 `makeContentLifecycleConfig()` 中知道 `pbrVerification` 如何映射到 `SceneSetupPipelineConfig`。本轮把这段映射迁到独立 application policy 模块。

新增与修改：

- 新增 `RuntimeContentConfigPolicy.h/.cpp`，集中把 `RuntimeApplicationShellConfig` 转换为 `RuntimeContentLifecycleConfig`。
- `RuntimeContentConfigPolicy::makeContentLifecycleConfig(...)` 负责设置 camera framebuffer size、scene window/skybox/legacy grass config、renderer backend key、verification config，以及 `enableEngineWorldMinimalScene` / `enableEngineWorldSceneProbe` 到 scene setup pipeline 的映射。
- `RuntimeApplicationShell` 删除成员函数 `makeContentLifecycleConfig()`，initialize 阶段只调用 `RuntimeContentConfigPolicy::makeContentLifecycleConfig(...)` 并把结果交给 `RuntimeContentLifecycle::prepare(...)`。
- `RuntimeApplicationShell.*` 不再直接引用 `RuntimeContentLifecycleConfig`；shell 的 startup content policy 认知进一步减少。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 content config policy 源文件和头文件。

已完成验证：

- 静态耦合检查确认 `RuntimeApplicationShell.*` 不再直接引用 `RuntimeContentLifecycleConfig`，`enableEngineWorldMinimalScene` / `enableEngineWorldSceneProbe` 的映射只存在于 `RuntimeContentConfigPolicy.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeContentConfigPolicy.cpp` 已参与编译，四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationShell` 现在只负责调用 content lifecycle，不再保存 content lifecycle config assembly 的具体规则。
- 这一步继续保持不扩张 PBR；它只把 verification scene policy 与 shell 启动流程解耦，便于后续把 remaining config/gui 责任继续拆成独立 coordinator。

### 2026-05-27 Runtime Frame Lifecycle Extraction

本轮继续缩小 `RuntimeApplicationShell` 的 frame orchestration 责任。上一轮已经把 startup content config policy 外移，本轮处理运行帧阶段仍留在 shell 里的职责：continue 条件、frame clock、frame runner 调用、rendered frame count、verification capture 状态。

新增与修改：

- 新增 `RuntimeFrameLifecycle.h/.cpp`，定义 `RuntimeFrameLifecycleConfig`、`RuntimeFrameLifecycleState` 和 frame lifecycle 入口。
- `RuntimeFrameLifecycleState` 现在持有 frame clock、rendered frame count 和 verification capture written flag。
- `RuntimeFrameLifecycle::shouldContinue(...)` 负责 verification max-frame stop 与 `GL_APP->update()`。
- `RuntimeFrameLifecycle::runFrame(...)` 负责 frame clock tick、`RuntimeFrameRunner::run(...)`、GUI render callback gating、rendered frame count 递增和 verification capture。
- `RuntimeApplicationShell` 不再持有 `mFrameClock`、`mRenderedFrameCount`、`mVerificationCaptureWritten`，改为持有 `RuntimeFrameLifecycleState mFrameLifecycle`。
- `RuntimeApplicationShell` 删除 `makeFrameConfig()`、`makeFrameClockConfig()` 和 `captureVerificationFrameIfNeeded()`；shell 的 frame path 现在只调用 `RuntimeFrameLifecycle::shouldContinue(...)` / `runFrame(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 frame lifecycle 源文件和头文件。

已完成验证：

- 静态耦合检查确认 `RuntimeApplicationShell.*` 不再包含 `makeFrameConfig()`、`makeFrameClockConfig()`、`captureVerificationFrameIfNeeded()`、`mFrameClock`、`mRenderedFrameCount` 或 `mVerificationCaptureWritten`；frame runner 与 verification capture 直接调用已集中到 `RuntimeFrameLifecycle.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeFrameLifecycle.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationShell` 的 frame lifecycle 细节继续外移，shell 更接近高层 runtime coordinator。
- 这一步继续保持不扩张 PBR；它只收敛 runtime frame lifecycle 和 verification capture 状态边界。

### 2026-05-27 Runtime Editor Lifecycle Extraction

本轮继续缩小 `RuntimeApplicationShell` 的 GUI/editor orchestration 责任。上一轮已经把 frame lifecycle 外移，本轮处理仍留在 shell 里的 GUI 初始化、editor panel frame callback、selection state 和 edit transaction state。

新增与修改：

- 新增 `RuntimeEditorLifecycle.h/.cpp`，定义 `RuntimeEditorLifecycleConfig`、`RuntimeEditorLifecycleState` 和 editor lifecycle 入口。
- `RuntimeEditorLifecycleState` 现在集中持有 editor selection 与 edit transaction log。
- `RuntimeEditorLifecycle::initialize(...)` 负责按 `enableGui` 初始化 `RuntimeGuiHost`。
- `RuntimeEditorLifecycle::makeFrameCallbacks(...)` 负责按 `enableGui` 创建 GUI frame callback，并在 callback 内调用 `RuntimeEditorPanelCoordinator::drawPanels(...)`。
- `RuntimeApplicationShell` 不再直接包含或调用 `RuntimeGuiHost` / `RuntimeEditorPanelCoordinator`。
- `RuntimeApplicationShell` 不再直接持有 `mEditorSelection` / `mEditorEditTransactions`，改为持有 `RuntimeEditorLifecycleState mEditorLifecycle`。
- `RuntimeApplicationShell` 删除 `renderFrameUi()` 与 `drawEditorPanels()`，每帧只把 editor lifecycle callbacks 交给 `RuntimeFrameLifecycle::runFrame(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 editor lifecycle 源文件和头文件。

已完成验证：

- 静态耦合检查确认 `RuntimeApplicationShell.*` 不再直接引用 `RuntimeGuiHost` / `RuntimeEditorPanelCoordinator`，也不再包含 `SelectionContext` / `EditTransactionLog` 具体成员。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeEditorLifecycle.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationShell` 的 editor lifecycle 细节继续外移，shell 现在更接近纯 runtime composition coordinator。
- 这一步继续保持不扩张 PBR；它只收敛 editor UI 生命周期和 editor 状态 owner 边界。

### 2026-05-27 Runtime Graphics Lifecycle Extraction

本轮继续缩小 `RuntimeApplicationShell` 的 startup graphics/bootstrap glue。上一轮已经把 GUI/editor lifecycle 外移，本轮处理仍留在 shell 里的窗口尺寸提示、viewport 初始化、clear color 设置和 OpenGL capability 输出。

新增与修改：

- 新增 `RuntimeGraphicsLifecycle.h/.cpp`，定义 `RuntimeGraphicsLifecycleConfig` 和 graphics lifecycle 入口。
- `RuntimeGraphicsLifecycle::reportWindowSetupPrompt()` 负责原有窗口尺寸提示输出。
- `RuntimeGraphicsLifecycle::initializeAfterWindow(...)` 负责窗口创建后的 viewport 初始化、clear color 设置和 OpenGL capability 输出。
- `RuntimeApplicationShell` 不再直接包含 `iostream`、`GL_ERROR_FIND.h`、`RuntimeViewport.h` 或 `core.h`。
- `RuntimeApplicationShell` 删除 `printOpenGLCapabilities()`，改为通过 `makeGraphicsLifecycleConfig()` 生成配置并调用 `RuntimeGraphicsLifecycle::initializeAfterWindow(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 graphics lifecycle 源文件和头文件。

已完成验证：

- 静态耦合检查确认 `RuntimeApplicationShell.*` 不再直接包含或调用 `RuntimeViewport`、`GL_CALL`、`glClearColor`、`glGetIntegerv`、`std::cout` 或 `printOpenGLCapabilities()`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeGraphicsLifecycle.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationShell` 的 graphics context startup 细节继续外移，shell 中剩余职责更集中在高层 lifecycle 排序。
- 这一步继续保持不扩张 PBR；它只收敛启动图形上下文和诊断输出边界。

### 2026-05-27 Engine Lifecycle Snapshot

本轮继续推进 Engine runtime ownership 的可观察边界。上一轮已经把 application shell 中的 graphics/bootstrap 细节外移，本轮不再继续拆 shell，而是补一个正式的 Engine lifecycle diagnostic snapshot，让 diagnostics UI 与 verification 都通过同一个 Engine-owned 快照读取生命周期状态。

新增与修改：

- 新增 `EngineLifecycleSnapshot`，集中描述 Engine 是否初始化、run mode、viewport、time/delta、Engine tick、subsystem count、active World playing 状态、active World tick 和 actor count。
- `Engine::captureLifecycleSnapshot()` 现在从 Engine 内部生成该快照，不要求外部模块分散读取 `EngineContext`、active World 和 Level actor 列表。
- `EngineDiagnosticsPanel` 的 Engine runtime、subsystem health 和 world sections 改为使用 `captureLifecycleSnapshot()` 中的 Engine/World lifecycle 字段。
- `RuntimePBRVerification::reportRenderedFrame(...)` 新增 `Runtime engine lifecycle snapshot stats` 输出，并让原有 Engine tick / subsystem health 输出复用同一份快照。
- `RuntimePBRVerification::reportEngineWorldCleanup(...)` 也改为从 `EngineLifecycleSnapshot` 读取 shutdown 后的 Engine/World 状态。
- `tools/verify_pbr.ps1` 新增对 `Runtime engine lifecycle snapshot stats` 的抓取、summary 输出和断言，覆盖 run mode、viewport、active World、tick count、subsystem count、time/delta 与 scene actor 数量关系。

已完成验证：

- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部输出并断言 `runtimeEngineLifecycleSnapshot`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，所有模式均覆盖新增 Engine lifecycle snapshot 断言。

结论：

- Engine 生命周期状态现在有了正式快照边界，Editor diagnostics 和自动 verification 不再各自拼装 Engine runtime 状态。
- 这一步继续保持不扩张 PBR；它只强化 Engine runtime ownership 的可观察契约。

### 2026-05-27 Runtime Application Config Policy Extraction

本轮继续缩小 `RuntimeApplicationShell` 的剩余 config glue。上一轮完成 Engine lifecycle snapshot 后，shell 中仍保留了四个非 content 配置组装函数：Engine desc、frame lifecycle config、editor lifecycle config 和 graphics lifecycle config。本轮把它们迁入独立 policy。

新增与修改：

- 新增 `RuntimeApplicationConfigPolicy.h/.cpp`，集中把 `RuntimeApplicationShellConfig` 映射为 application-level lifecycle configs。
- `RuntimeApplicationConfigPolicy::makeEngineDesc(...)` 负责根据 verification 状态选择 `EngineRunMode`，并设置 viewport size。
- `RuntimeApplicationConfigPolicy::makeFrameLifecycleConfig(...)` 负责组装 GUI gate、frame clock config 和 verification config。
- `RuntimeApplicationConfigPolicy::makeEditorLifecycleConfig(...)` 负责组装 GUI gate、GLFW window 指针和 editor orbit angle 可变指针。
- `RuntimeApplicationConfigPolicy::makeGraphicsLifecycleConfig(...)` 负责组装 startup graphics viewport config。
- `RuntimeApplicationShell` 删除 `makeEngineDesc()`、`makeFrameLifecycleConfig()`、`makeEditorLifecycleConfig()` 和 `makeGraphicsLifecycleConfig()` 私有成员。
- `RuntimeApplicationShell.h` 不再暴露 config policy 或 graphics lifecycle include，相关依赖移动到 `RuntimeApplicationShell.cpp`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 application config policy 源文件和头文件。

已完成验证：

- 静态耦合检查确认 `RuntimeApplicationShell.*` 不再声明或定义四个私有 `make*Config` 成员，调用点已迁到 `RuntimeApplicationConfigPolicy::*`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationConfigPolicy.cpp` 已参与编译，三条 focused verification mode 全部通过。

结论：

- `RuntimeApplicationShell` 的职责进一步收敛到 lifecycle 顺序编排，不再保存非 content config 映射规则。
- 这一步继续保持不扩张 PBR；它只收敛 application 层配置映射边界。

### 2026-05-27 Runtime Window Lifecycle Snapshot Boundary

本轮继续处理上一阶段标记的剩余 window lifecycle glue。`RuntimeApplicationShell` 在配置策略抽取后仍直接读取 `GL_APP->getWidth()`、`GL_APP->getHeight()`、`GL_APP->getWindow()`，并直接调用 `GL_APP->destroy()`。本轮把这些窗口单例访问收敛到 `RuntimeWindowLifecycle`。

新增与修改：

- `RuntimeWindowLifecycle.h/.cpp` 新增 `RuntimeWindowSnapshot`，集中描述当前 framebuffer width、height 和 native `GLFWwindow*`。
- `RuntimeWindowLifecycle::captureSnapshot()` 统一从 `Application` 单例读取当前窗口快照。
- `RuntimeWindowLifecycle::destroy()` 统一执行 window/application destroy。
- `RuntimeApplicationShell` 初始化 content/editor lifecycle 和每帧 frame/editor callbacks 时改为使用 `RuntimeWindowLifecycle::captureSnapshot()`。
- `RuntimeApplicationShell::destroy()` 改为调用 `RuntimeWindowLifecycle::destroy()`。
- `RuntimeApplicationShell.cpp` 不再直接包含 `Application.h`，也不再直接调用 `GL_APP->getWindow()`、`GL_APP->getWidth()`、`GL_APP->getHeight()` 或 `GL_APP->destroy()`。

已完成验证：

- 静态耦合检查确认 `RuntimeApplicationShell.*` 中不再出现 `Application.h`、`GL_APP->getWindow`、`GL_APP->getWidth`、`GL_APP->getHeight` 或 `GL_APP->destroy`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Window singleton access 现在集中在 `RuntimeWindowLifecycle`，Shell 继续靠 lifecycle 边界协调初始化、帧循环和销毁。
- 这一步继续保持不扩张 PBR；它只收敛窗口运行时状态和 native window 指针访问边界。

### 2026-05-27 Engine Subsystem Lifecycle Summary Snapshot

本轮继续上一阶段的下一步，把 `Engine::captureLifecycleSnapshot()` 从只提供 `subsystemCount` 扩展为可枚举的 subsystem summary。目标是让 diagnostics UI 和 verification 不再各自从具体 `AssetSubsystem*` / `RendererSubsystem*` 拼 health counters，而是优先读取 Engine-owned snapshot 中的通用 subsystem 列表。

新增与修改：

- `EngineSubsystem` 新增只读诊断虚接口：`getDebugName()`、`isInitializedForDiagnostics()`、`getTickCountForDiagnostics()`。
- `AssetSubsystem` 覆盖诊断接口，报告 `AssetSubsystem`、initialized 状态和 tick count。
- `RendererSubsystem` 覆盖诊断接口，报告 `RendererSubsystem`、initialized 状态和 tick count。
- `EngineLifecycleSnapshot` 新增 `EngineSubsystemLifecycleSummary` 列表、`initializedSubsystemCount` 和 `tickedSubsystemCount`。
- `Engine::captureLifecycleSnapshot()` 遍历 Engine-owned subsystem，收集 index、name、initialized 和 tick count。
- `EngineDiagnosticsPanel` 的 `Subsystem Health` tick 显示改为优先读取 snapshot summary；`Engine Runtime` 中新增 initialized / ticked subsystem count 和可展开 subsystem summary 列表。
- `RuntimePBRVerification::reportRenderedFrame(...)` 新增 `Runtime engine subsystem summary stats` 输出，记录 subsystem count、initialized/ticked count、names、initialized flags 和 ticks。
- `Runtime subsystem health stats` 中的 Asset / Renderer tick 现在来自 Engine snapshot summary，而不是直接从具体 subsystem 指针读取。
- `Runtime engine subsystem cleanup stats` 增加 shutdown 后 snapshot summary 字段，证明 Engine 仍可枚举 owned subsystem，但 initialized/ticked 已清零。
- `tools/verify_pbr.ps1` 新增 runtime subsystem summary 和 cleanup snapshot summary 断言。

已完成验证：

- 静态检查确认 `getDebugName`、`isInitializedForDiagnostics`、`getTickCountForDiagnostics`、`EngineSubsystemLifecycleSummary`、`subsystemSummaries`、`Runtime engine subsystem summary stats` 和 `snapshotSubsystem*` 均已接入 engine / verification / diagnostics。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过，并输出 `names=AssetSubsystem|RendererSubsystem`、`ticks=2|2`、cleanup `snapshotSubsystemTicks=0|0`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，所有 mode 均覆盖 runtime subsystem summary 和 shutdown snapshot summary 断言。

结论：

- Engine lifecycle snapshot 现在不只是 Engine/World 快照，也能表达 Engine-owned subsystem 的通用健康摘要。
- 这一步继续保持不扩张 PBR；它只加强 Engine runtime ownership 的可观察和验证边界。

### 2026-05-27 Runtime Application Startup Lifecycle Extraction

本轮继续收敛 `RuntimeApplicationShell` 的高层顺序编排。上一轮已经让 Engine snapshot 能表达 subsystem summary；本轮回到 Shell，把 initialize 阶段中的 Engine / Window / Graphics / Content / Editor / Frame reset 顺序迁入独立 startup lifecycle，避免 Shell 继续承载启动流程细节。

新增与修改：

- 新增 `RuntimeApplicationStartupLifecycle.h/.cpp`。
- `RuntimeApplicationStartupLifecycle::initialize(...)` 统一编排 Engine 初始化、Window 初始化、Graphics 初始化、Content prepare、Editor 初始化和 Frame lifecycle reset。
- `RuntimeApplicationShell::initialize()` 改为单行委托到 `RuntimeApplicationStartupLifecycle::initialize(...)`。
- `RuntimeApplicationShell.cpp` 不再直接包含或调用 `RuntimeContentLifecycle`、`RuntimeContentConfigPolicy`、`RuntimeGraphicsLifecycle`、`RuntimeEngineLifecycle::initializeEngine(...)`、`RuntimeWindowLifecycle::initialize(...)`、`RuntimeEditorLifecycle::initialize(...)` 或 `RuntimeFrameLifecycle::reset(...)` 这些 startup 细节。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 startup lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 startup 细节调用集中在 `RuntimeApplicationStartupLifecycle.cpp`，Shell 中只保留 `RuntimeApplicationStartupLifecycle::initialize(...)` 入口。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationStartupLifecycle.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Shell 的 initialize 阶段现在更接近 composition root 的入口，不再直接展开 startup flow。
- 这一步继续保持不扩张 PBR；它只收敛 application runtime 的启动编排边界。

### 2026-05-27 Runtime Application Shutdown Lifecycle Extraction

本轮继续收敛 `RuntimeApplicationShell` 的高层顺序编排。上一轮已把 startup 顺序迁入 `RuntimeApplicationStartupLifecycle`；本轮把 cleanup / destroy 阶段中的 renderer cleanup report、camera cleanup、runtime context detach、Engine shutdown、Engine cleanup report 和 window destroy 迁入独立 shutdown lifecycle。

新增与修改：

- 新增 `RuntimeApplicationShutdownLifecycle.h/.cpp`。
- `RuntimeApplicationShutdownLifecycle::cleanup(...)` 统一编排 cleanup 顺序：先 `RuntimeEngineLifecycle::beginCleanup(...)` detach renderer backend 并保留只读 cleanup refs，再报告 renderer subsystem cleanup，随后清理 camera、detach runtime context、shutdown Engine，并报告 Engine cleanup。
- `RuntimeApplicationShutdownLifecycle::destroy()` 统一委托 `RuntimeWindowLifecycle::destroy()`。
- `RuntimeApplicationShell::cleanup()` 改为委托 `RuntimeApplicationShutdownLifecycle::cleanup(...)`。
- `RuntimeApplicationShell::destroy()` 改为委托 `RuntimeApplicationShutdownLifecycle::destroy()`。
- `RuntimeApplicationShell.cpp` 不再直接包含或调用 `RuntimeCameraLifecycle`、`RuntimeVerificationLifecycle`、`RuntimeEngineLifecycle::beginCleanup(...)`、`RuntimeEngineLifecycle::detachRuntimeContext(...)`、`RuntimeEngineLifecycle::shutdownEngine(...)` 或 `RuntimeWindowLifecycle::destroy()` 这些 shutdown 细节。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 shutdown lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 cleanup/shutdown 细节调用集中在 `RuntimeApplicationShutdownLifecycle.cpp`，Shell 中只保留 shutdown 委托入口。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationShutdownLifecycle.cpp` 已参与编译，三条 focused verification mode 全部通过，并继续输出 renderer/world/subsystem cleanup stats。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，所有 mode 均继续覆盖 runtime renderer/world/subsystem cleanup contract。

结论：

- Shell 的 cleanup/destroy 阶段现在与 initialize 阶段一致，都是只保留 application lifecycle 委托入口。
- 这一步继续保持不扩张 PBR；它只收敛 application runtime 的关闭编排边界。

### 2026-05-27 Runtime Application Frame Lifecycle Extraction

本轮继续收敛 `RuntimeApplicationShell` 的剩余 frame-level glue。startup 和 shutdown 已经分别迁入独立 lifecycle；本轮把 `shouldContinue()` / `runFrame()` 中的 frame config 映射、window snapshot、editor callbacks 组装和 `RuntimeFrameLifecycle` 调用迁入独立 application frame lifecycle。

新增与修改：

- 新增 `RuntimeApplicationFrameLifecycle.h/.cpp`。
- `RuntimeApplicationFrameLifecycle::shouldContinue(...)` 统一从 `RuntimeApplicationConfigPolicy` 构造 frame lifecycle config，并委托 `RuntimeFrameLifecycle::shouldContinue(...)`。
- `RuntimeApplicationFrameLifecycle::runFrame(...)` 统一捕获 `RuntimeWindowLifecycle::captureSnapshot()`，构造 frame lifecycle config 和 editor frame callbacks，再调用 `RuntimeFrameLifecycle::runFrame(...)`。
- `RuntimeApplicationShell::shouldContinue()` 改为委托 `RuntimeApplicationFrameLifecycle::shouldContinue(...)`。
- `RuntimeApplicationShell::runFrame()` 改为委托 `RuntimeApplicationFrameLifecycle::runFrame(...)`。
- `RuntimeApplicationShell.cpp` 不再直接包含或调用 `RuntimeApplicationConfigPolicy`、`RuntimeEditorLifecycle`、`RuntimeFrameLifecycle::shouldContinue(...)`、`RuntimeFrameLifecycle::runFrame(...)`、`RuntimeWindowLifecycle::captureSnapshot()` 或 frame/editor config mapping。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 frame lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 frame-level 细节调用集中在 `RuntimeApplicationFrameLifecycle.cpp`，Shell 中只保留 frame lifecycle 委托入口。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationFrameLifecycle.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationShell` 现在对 initialize、shouldContinue/runFrame、cleanup/destroy 都只保留 application lifecycle 委托入口。
- 这一步继续保持不扩张 PBR；它只收敛 application runtime 的 frame 编排边界。

### 2026-05-27 Runtime Application State Context Extraction

本轮继续收敛 `RuntimeApplicationShell`。上一轮已经让 Shell 的各生命周期函数只保留委托入口；本轮把 Shell 中分散持有的 Engine、runtime context、engine lifecycle state、editor lifecycle state、legacy experiments 和 frame lifecycle state 聚合成独立 `RuntimeApplicationState`，让 startup / frame / shutdown lifecycle 共享同一个 application runtime state 边界。

新增与修改：

- 新增 `RuntimeApplicationState.h`。
- `RuntimeApplicationState` 统一持有 `GLengine::Engine`、`RuntimeEngineLifecycleState`、`AppRuntimeContext`、`RuntimeEditorLifecycleState`、`LegacyExperimentRunner` 和 `RuntimeFrameLifecycleState`。
- `RuntimeApplicationShell` 删除分散的 `mEngine`、`mEngineLifecycle`、`mRuntime`、`mEditorLifecycle`、`mLegacyExperiments` 和 `mFrameLifecycle` 成员，改为只持有 `RuntimeApplicationState mState` 与 `RuntimeApplicationShellConfig mConfig`。
- `RuntimeApplicationStartupLifecycle::initialize(...)` 改为接收 `RuntimeApplicationState&`，并通过 state 访问 Engine、runtime context、engine lifecycle、legacy experiments 和 frame lifecycle。
- `RuntimeApplicationFrameLifecycle::shouldContinue(...)` / `runFrame(...)` 改为接收 `RuntimeApplicationState`，frame 编排不再从 Shell 接收多组分散状态参数。
- `RuntimeApplicationShutdownLifecycle::cleanup(...)` 改为接收 `RuntimeApplicationState&`，shutdown 编排不再从 Shell 接收 Engine、runtime context 和 engine lifecycle 三个独立参数。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 state header。

已完成验证：

- 静态检查确认 Shell 中只保留 `mState` 和 `mConfig`，旧的分散 runtime state 成员已移除。
- 静态检查确认 startup / frame / shutdown lifecycle header 中不再暴露 `AppRuntimeContext&`、`Engine&`、`RuntimeEngineLifecycleState&`、`RuntimeFrameLifecycleState&` 或 `LegacyExperimentRunner&` 这些分散参数。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；相关 application lifecycle 文件重新编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationShell` 现在只持有 config 和一个明确的 runtime state aggregate，职责更接近“创建 callback wrapper + 持有 composition state”。
- 这一步继续保持不扩张 PBR；它只收敛 application runtime 的状态边界。

### 2026-05-27 Runtime Renderer Backend Contract Verification Naming Neutralization

本轮把 renderer backend verification 的对外表述从旧的 subsystem/PBR 语境中进一步中性化。实现上保留 `Runtime renderer subsystem stats` 与 cleanup stats，避免破坏已有观察入口；同时新增并断言 `Runtime renderer backend contract stats` 与 `Runtime renderer backend contract cleanup stats`，让 backend key、readiness、registry、frame pass、no-op backend 和 cleanup detach 状态都有通用 contract 视角。

新增与修改：

- `RuntimePBRVerification` 在 rendered frame report 中新增 `Runtime renderer backend contract stats`，字段覆盖 executor attachment、backend ready/key/state/owner/ownership、registry key/count、attach/detach count、ready/not-ready frames、framebuffer、frame plan key、planned/executed/skipped pass、bridge/executor call count 和 observed renderer pass count。
- `RuntimePBRVerification` 在 cleanup report 中新增 `Runtime renderer backend contract cleanup stats`，字段覆盖 executor detach、backend detached state、metadata clear、registry clear、attach/detach closure 与 ready-frame evidence。
- `tools/verify_pbr.ps1` 捕获并汇总新 contract 行，新增普通 backend 与 no-op backend 的稳定断言，同时保留旧 runtime renderer subsystem stats 断言。
- `docs/subagents_coordination.md` 已把当前 round 更新为本轮 contract naming neutralization，并限定旁路 subagent 只能只读审计，不拥有代码写入范围。

已完成验证：

- 静态检查确认新 contract stats / cleanup stats 在 C++ 输出、PowerShell 捕获、断言和 summary 中均存在，旧 `Runtime renderer subsystem ...` 输出仍保留。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过，普通 backend 与 no-op backend 均输出新 contract 行。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Renderer backend verification 现在同时具备 legacy subsystem 视角与通用 backend contract 视角。
- 这一步不扩张 PBR 功能，只把已有 backend contract 的可验证边界从 PBR 命名中剥离出来。

### 2026-05-27 Renderer Backend Contract Header Extraction

本轮继续收敛 renderer backend 抽象。上一轮已经把 verification 输出中性化；本轮把 backend contract 类型本身从 `RendererSubsystem.h` 中抽出，避免 application runtime backend / factory 为了实现 backend contract 而依赖完整 subsystem header。

新增与修改：

- 新增 `engine/RendererBackend.h`。
- `RendererBackend.h` 定义 `RendererBackend`、`RendererFrameIntent`、`RendererFrameResult` 和 `RendererBackendAttachmentDesc`。
- `RendererFrameExecutor`、`RendererSubsystemFrameIntent` 与 `RendererSubsystemFrameResult` 保留为兼容 alias，避免一次性破坏既有命名入口。
- `RendererSubsystem.h/.cpp` 改为消费 `RendererBackend.h`，自身不再定义 backend contract 类型。
- `RuntimeRendererFrameBridgeAdapter`、`RuntimeNoOpRendererBackend` 和 `RuntimeRendererBackendFactory` 改为包含 `RendererBackend.h`，并使用 `RendererBackend` / `RendererFrameIntent` / `RendererFrameResult`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 `engine/RendererBackend.h`。

已完成验证：

- 静态检查确认 application backend/factory 不再包含 `RendererSubsystem.h`，只依赖 `RendererBackend.h`。
- 静态检查确认 `RendererSubsystemFrameIntent` / `RendererSubsystemFrameResult` 在代码中只剩兼容 alias。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Renderer backend contract 现在有独立 engine header，application backend 实现不再需要依赖完整 RendererSubsystem 定义。
- 这一步继续保持不扩张 PBR；它只收敛 renderer backend 与 subsystem 的代码依赖方向。

### 2026-05-27 Renderer Backend Registry Metadata Contract Extraction

本轮继续收敛 renderer backend registry 的命名和归属。上一轮已经让 backend contract 类型进入 `RendererBackend.h`；本轮把 backend registry entry metadata 也提升到 engine contract 层，避免 `RuntimeRendererBackendRegistration` 这种 application-only 类型继续描述通用 backend registry。

新增与修改：

- `engine/RendererBackend.h` 新增 `RendererBackendRegistration`，字段保留 `key`、`displayName` 和 `defaultBackend`。
- `RuntimeRendererBackendFactory::registeredBackends()` 改为返回 `std::vector<GLengine::RendererBackendRegistration>`。
- 删除 application 层的 `RuntimeRendererBackendRegistration` 定义。
- `RuntimeRendererBackendFactory::isRegisteredBackendKey(...)` 改为使用 engine-level registration entry。

已完成验证：

- 静态检查确认 `RuntimeRendererBackendRegistration` 已无残留，registry metadata 类型只存在于 `engine/RendererBackend.h` 和 runtime factory 返回值中。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Renderer backend registry metadata 现在属于 engine-level backend contract；runtime factory 仍只负责具体 application backend 创建。
- 这一步继续保持不扩张 PBR；它只减少 application 命名对 renderer backend registry contract 的占用。

### 2026-05-27 Renderer Backend Registry Helper Extraction

本轮继续推进 renderer backend registry 从 runtime factory 私有逻辑向 engine-level contract/service 迁移。具体 backend 列表与创建仍留在 `RuntimeRendererBackendFactory`，因为它依赖 application runtime context；但 registry 查询、默认 key 选择和 attachment desc 组装已经迁入 `RendererBackendRegistry`。

新增与修改：

- 新增 `engine/RendererBackendRegistry.h/.cpp`。
- `RendererBackendRegistry` 持有 `RendererBackendRegistration` 列表，并提供 `getRegisteredBackends()`、`getBackendCount()`、`isRegisteredBackendKey(...)`、`getDefaultBackendKey(...)` 和 `makeAttachmentDesc(...)`。
- `RuntimeRendererBackendFactory` 现在只声明 runtime 可用 backend 列表和创建具体 backend；默认 key、contains 判断和 attachment desc 由 engine-level helper 完成。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 registry helper 源文件和头文件。

已完成验证：

- 静态检查确认 registry 查询/默认 key/attachment desc 逻辑集中在 `RendererBackendRegistry`，runtime factory 不再直接手写 `std::any_of` 或 registry count 组装。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RendererBackendRegistry.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Renderer backend registry 的查询/默认值/attachment metadata 现在有 engine-level helper，runtime factory 只保留 runtime backend 列表和创建职责。
- 这一步继续保持不扩张 PBR；它只把 backend registry service 边界从 application factory 私有实现中剥离出来。

### 2026-05-27 Renderer Backend Registry Selection Policy Extraction

本轮继续沿着上一轮 registry helper 边界推进，把 backend key 的选择结果从 runtime factory 私有判断中抽成 engine-level `RendererBackendSelection`。runtime factory 仍负责把已选中的 key 映射为具体 application backend 类型，但“请求 key / 默认 key / 是否注册 / 实际 selected key / registry count”这些选择语义现在由 `RendererBackendRegistry` 统一给出。

新增与修改：

- `engine/RendererBackendRegistry.h` 新增 `RendererBackendSelection`，字段包括 `requestedKey`、`selectedKey`、`defaultKey`、`registered`、`usedDefault` 和 `registryBackendCount`。
- `RendererBackendRegistry::resolveBackendSelection(...)` 负责把外部请求 key 解析为 selection result：空请求可落到默认 backend，未知 key 不做隐式 fallback，并以 `registered=false` 表示无效选择。
- `RuntimeRendererBackendFactory::createBackend(...)` 改为先获取 engine-level selection result，再只根据 `selection.selectedKey` 创建具体 runtime backend。
- `RuntimeRendererBackendFactory::makeRendererSubsystemAttachmentDesc(...)` 改为使用 selection result 的 selected key 生成 attachment metadata。
- `RuntimeRendererBackendFactory::isRegisteredBackendKey(...)` 保持精确注册检查，不把空 key 默认化为已注册 key。

已完成验证：

- 静态检查确认 `RendererBackendSelection` 与 `resolveBackendSelection(...)` 已接入 registry helper 和 runtime factory。
- 静态检查确认具体 backend 创建分支只依赖 `selection.selectedKey`，请求 key 的注册/default 解析集中在 `RendererBackendRegistry`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RendererBackendRegistry.cpp` 与 `RuntimeRendererBackendFactory.cpp` 已重新编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Backend selection policy 已从 runtime factory 的字符串判断中抽到 engine-level registry helper；runtime factory 只保留“selected key -> concrete backend object”的 application composition 职责。
- 这一步继续保持不扩张 PBR；它推进的是 renderer backend contract/service 边界。

### 2026-05-27 Runtime Renderer Backend Catalog Extraction

本轮把 runtime backend 的 key、registry metadata、selection 入口和 attachment metadata 从 `RuntimeRendererBackendFactory` 中拆到独立 `RuntimeRendererBackendCatalog`。这样 factory 的职责进一步收敛为“根据已经解析好的 `RendererBackendSelection` 创建具体 backend object”，不再同时承担 registry/catalog/query/attachment desc 职责。

新增与修改：

- 新增 `application/RuntimeRendererBackendCatalog.h/.cpp`。
- `RuntimeRendererBackendCatalog` 负责 `runtimeFramePipelineBackendKey()`、`testNoOpBackendKey()`、`defaultBackendKey()`、`makeRegistry()`、`registeredBackends()`、`isRegisteredBackendKey(...)`、`resolveBackendSelection(...)` 和 `makeRendererSubsystemAttachmentDesc(...)`。
- `RuntimeRendererBackendFactory` 删除 registry/query/default/attachment desc API，只保留 `createBackend(context, selection)` 和 `createRuntimeFramePipelineBackend(context)`。
- `RuntimeEngineLifecycle::attachRendererBackend(...)` 现在先通过 catalog 解析 selection，再把 selection 交给 factory 创建 backend，并由 catalog 生成 renderer subsystem attachment desc。
- `RuntimePBRVerificationArgs` 改为从 catalog 获取 no-op backend key。
- `RuntimeApplicationShellConfig::rendererBackendKey` 默认值改为 `RuntimeRendererBackendCatalog::defaultBackendKey()`，避免默认 backend key 字符串在 config 和 catalog 中双源维护。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 catalog 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeRendererBackendFactory` 中不再暴露 `defaultBackendKey()`、`testNoOpBackendKey()`、`registeredBackends()`、`isRegisteredBackendKey(...)` 或 `makeRendererSubsystemAttachmentDesc(...)`。
- 静态检查确认 runtime backend key 字符串只集中在 `RuntimeRendererBackendCatalog.cpp`，shell config、verification args 和 engine lifecycle 都通过 catalog 工作。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeRendererBackendCatalog.cpp`、`RuntimeRendererBackendFactory.cpp`、`RuntimeEngineLifecycle.cpp`、`RuntimePBRVerificationArgs.cpp` 和 `RuntimeApplicationShell.cpp` 已重新编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Runtime backend factory 现在更接近纯 backend object construction provider；registry/catalog/selection/attachment metadata 已集中到 runtime catalog + engine registry helper。
- 这一步继续保持不扩张 PBR；它只推进 renderer backend composition boundary。

### 2026-05-27 Runtime Application Config Header Extraction

本轮继续收敛 application composition root 的头文件依赖。`RuntimeApplicationShellConfig` 原本定义在 `RuntimeApplicationShell.h` 中，导致 config policy、content policy、startup/frame/shutdown lifecycle 和 verification args 只为了读取配置类型就包含完整 Shell。本轮把 config struct 抽到独立 `RuntimeApplicationConfig.h`，让 Shell 只保留 state/config/callback glue 的拥有者边界。

新增与修改：

- 新增 `application/RuntimeApplicationConfig.h`，集中定义 `RuntimeApplicationShellConfig`。
- `RuntimeApplicationShell.h` 改为包含 `RuntimeApplicationConfig.h`，自身不再定义 config struct，也不再直接包含 frame clock、verification、renderer backend catalog 或 window lifecycle header。
- `RuntimeApplicationConfigPolicy`、`RuntimeContentConfigPolicy`、`RuntimeApplicationStartupLifecycle`、`RuntimeApplicationFrameLifecycle`、`RuntimeApplicationShutdownLifecycle` 和 `RuntimePBRVerificationArgs` 改为依赖 `RuntimeApplicationConfig.h`，不再为了 `RuntimeApplicationShellConfig` 包含 `RuntimeApplicationShell.h`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 config header。

已完成验证：

- 静态检查确认除 `main.cpp` 和 `RuntimeApplicationShell.*` 外，application 模块不再包含 `RuntimeApplicationShell.h`。
- 静态检查确认 config users 通过 `RuntimeApplicationConfig.h` 使用 `RuntimeApplicationShellConfig`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationConfigPolicy.cpp`、`RuntimeApplicationFrameLifecycle.cpp`、`RuntimeApplicationShell.cpp`、`RuntimeApplicationShutdownLifecycle.cpp`、`RuntimeApplicationStartupLifecycle.cpp`、`RuntimeContentConfigPolicy.cpp`、`RuntimePBRVerificationArgs.cpp` 和 `main.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Shell config 现在是独立 application config contract；Shell header 不再作为 config type 的传递依赖中心。
- 这一步继续保持不扩张 PBR；它只降低 application composition root 的头文件耦合。

### 2026-05-27 Runtime Application Callback Binder Extraction

本轮继续收敛 `RuntimeApplicationShell` 的最后一层 callback glue。上一轮后 Shell 已经只持有 `RuntimeApplicationState` 和 `RuntimeApplicationShellConfig`，但仍保留 `initialize()`、`shouldContinue()`、`runFrame()`、`cleanup()` 和 `destroy()` 五个私有 wrapper，只用于把 `RuntimeBootstrapperCallbacks` 转发到 startup/frame/shutdown lifecycle。本轮把这层绑定抽成独立 `RuntimeApplicationCallbackBinder`。

新增与修改：

- 新增 `application/RuntimeApplicationCallbackBinder.h/.cpp`。
- `RuntimeApplicationCallbackBinder::makeCallbacks(state, config)` 负责生成 `RuntimeBootstrapperCallbacks`，并把 initialize / shouldContinue / runFrame / cleanup / destroy 分别绑定到 application startup/frame/shutdown lifecycle。
- `RuntimeApplicationShell` 删除五个私有 lifecycle wrapper；`makeCallbacks()` 现在只委托 `RuntimeApplicationCallbackBinder::makeCallbacks(mState, mConfig)`。
- `RuntimeApplicationShell.cpp` 不再直接包含 startup/frame/shutdown lifecycle header，只包含 callback binder。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 binder 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationShell.*` 中不再直接引用 `RuntimeApplicationStartupLifecycle`、`RuntimeApplicationFrameLifecycle` 或 `RuntimeApplicationShutdownLifecycle`，也不再保留私有 wrapper。
- 静态检查确认 lifecycle 绑定调用集中在 `RuntimeApplicationCallbackBinder.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationCallbackBinder.cpp`、`RuntimeApplicationShell.cpp` 和 `main.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Shell 现在更接近纯 application owning shell：只拥有 state/config，并暴露 callback 创建入口。
- Bootstrapper callback 到 lifecycle 的绑定被隔离到独立 binder；后续可继续检查是否需要把 owning shell 与 bootstrapper/run boundary 进一步分开。
- 这一步继续保持不扩张 PBR；它只降低 application composition root 的 callback 耦合。

### 2026-05-27 Runtime Application Runner Boundary Extraction

本轮继续收敛 application run boundary。上一轮后 Shell 已只负责 state/config ownership 和 callback 创建，但 `main.cpp` 仍直接知道 “构造 Shell -> 取 callbacks -> RuntimeBootstrapper::run(...)” 的组合方式。本轮新增 `RuntimeApplicationRunner`，把 config 到 runtime run loop 的组装从入口文件移入 application 层。

新增与修改：

- 新增 `application/RuntimeApplicationRunner.h/.cpp`。
- `RuntimeApplicationRunner::run(RuntimeApplicationShellConfig config)` 负责构造 `RuntimeApplicationShell`，再把 `shell.makeCallbacks()` 交给 `RuntimeBootstrapper::run(...)`。
- `main.cpp` 不再包含 `RuntimeApplicationShell.h` 或 `RuntimeBootstrapper.h`，只保留日志等级设置、旧 `PointLightShadow::MAX_POINT_LIGHTS` 全局兼容设置、verification args 解析和 `RuntimeApplicationRunner::run(...)` 调用。
- `RuntimeApplicationShell.h` 对 `RuntimeBootstrapperCallbacks` 改为前置声明，不再传递包含 `RuntimeBootstrapper.h`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 runner 源文件和头文件。

已完成验证：

- 静态检查确认 `main.cpp` 中不再出现 `RuntimeApplicationShell`、`RuntimeBootstrapper`、`shell.makeCallbacks()` 或 `RuntimeBootstrapper::run(...)`。
- 静态检查确认 “构造 Shell + 调 Bootstrapper” 只集中在 `RuntimeApplicationRunner.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationRunner.cpp`、`RuntimeApplicationShell.cpp` 和 `main.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 程序入口现在更接近 thin entrypoint：设置少量全局兼容状态、解析 runtime config、交给 application runner。
- Shell/Bootstrapper 的组装方式已进入 application run boundary，后续可继续检查 args parsing、logger setup 和 `PointLightShadow::MAX_POINT_LIGHTS` 这类旧全局兼容点是否也需要下沉。
- 这一步继续保持不扩张 PBR；它只降低 main/application run loop 的耦合。

### 2026-05-27 Runtime Application Entry Boundary Extraction

本轮继续收敛入口残留。上一轮后 `main.cpp` 已不直接组合 Shell/Bootstrapper，但仍负责日志初始化、verification args 解析和 `PointLightShadow::MAX_POINT_LIGHTS` 静态成员定义。本轮新增 `RuntimeApplicationEntry`，把 runtime entry 启动策略移入 application 层，并把点光阴影静态默认值归位到 `pointLightShadow.cpp`。

新增与修改：

- 新增 `application/RuntimeApplicationEntry.h/.cpp`。
- `RuntimeApplicationEntry::run(argc, argv)` 负责设置 `LogManager` 默认等级、解析 `RuntimePBRVerificationArgs`，并调用 `RuntimeApplicationRunner::run(...)`。
- `main.cpp` 现在只包含 `RuntimeApplicationEntry.h` 并调用 `RuntimeApplicationEntry::run(argc, argv)`。
- `PointLightShadow::MAX_POINT_LIGHTS` 的静态成员定义从 `main.cpp` 移到 `light/shadow/pointLightShadow/pointLightShadow.cpp`。
- `PointLightShadow` 默认构造中的 `PerspectiveCamera` aspect 参数从 `1.0` 改为 `1.0f`，消除本轮重新编译该文件时出现的 double-to-float warning。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 entry 源文件和头文件。

已完成验证：

- 静态检查确认 `main.cpp` 中不再出现 `RuntimeApplicationRunner`、`RuntimePBRVerificationArgs`、`LogManager`、`PointLightShadow`、`makeShellConfigFromArguments(...)` 或 `MAX_POINT_LIGHTS`。
- 静态检查确认 `PointLightShadow::MAX_POINT_LIGHTS` 只有一个定义，且位于 `pointLightShadow.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationEntry.cpp`、`pointLightShadow.cpp` 和 `main.cpp` 已参与编译，三条 focused verification mode 全部通过；除既有 LNK4075 提示外无 C4244 warning。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `main.cpp` 现在是真正的 thin entrypoint，只委托 runtime application entry。
- runtime 启动策略、参数解析和日志默认设置进入 application entry boundary。
- 旧点光阴影静态成员定义不再挂在程序入口文件上，减少全局兼容代码对入口的污染。
- 这一步继续保持不扩张 PBR；它只降低 main/application launch boundary 的耦合。

### 2026-05-27 Runtime Verification Args Neutral Alias Extraction

本轮按 `/subagents` 要求先更新多 agent 协作边界，并启动两个只读 sidecar audit agent。随后继续收敛 application entry 的命名依赖：上一轮 `RuntimeApplicationEntry` 已接管 logger setup / args parsing / runner 调用，但它仍直接包含 `RuntimePBRVerificationArgs.h`。本轮新增中性 `RuntimeVerificationArgs.h` 作为 entry-facing public header，旧 `RuntimePBRVerificationArgs.h` 降级为兼容 wrapper。

新增与修改：

- 更新 `docs/subagents_coordination.md`，把本轮写入 owner 固定为 parent agent，两个子 agent 只读审查 args boundary 与文档一致性。
- 新增 `application/RuntimeVerificationArgs.h`，直接声明 `makeShellConfigFromArguments(argc, argv)`，并依赖 `RuntimeApplicationConfig.h`。
- `RuntimeApplicationEntry.cpp` 改为包含 `RuntimeVerificationArgs.h`，不再直接包含 PBR-specific args header。
- `RuntimePBRVerificationArgs.h` 改为包含 `RuntimeVerificationArgs.h` 的兼容 wrapper，保留旧 include path。
- `RuntimePBRVerificationArgs.cpp` 仍承载现有 implementation 与 `PbrVerificationModeDescriptor` 表；本轮不声称完成 `.cpp` 迁移，避免 duplicate symbol 或大范围搬迁风险。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 neutral header。

已完成验证：

- 子 agent `Runtime Args Boundary Audit` 确认当前切片是 header neutralization，不是完整 `.cpp` extraction，并指出后续若迁移实现必须避免 duplicate symbol。
- 子 agent `Documentation Consistency Audit` 确认主要文档需把 verification args neutralization 从“下一步”改为“已接入 first slice”，且不能过度声称完整迁移。
- 静态检查确认 `main.cpp` 和 `RuntimeApplicationEntry.cpp` 中不再出现 `RuntimePBRVerificationArgs.h` 直接 include。
- 静态检查确认 `RuntimeVerificationArgs.h` 直接声明公共 API，`RuntimePBRVerificationArgs.h` 只保留兼容 wrapper 角色。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationEntry.cpp` 与 `RuntimePBRVerificationArgs.cpp` 已重新编译，三条 focused verification mode 全部通过；仅保留既有 LNK4075 提示。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- application entry 的 public include surface 已从 PBR-specific args header 切到 neutral runtime verification args header。
- 旧 `RuntimePBRVerificationArgs.*` 仍作为兼容 implementation 留存，下一步可选择完整迁移 `.cpp` 实现和 descriptor 命名，或继续先收敛 `RuntimeApplicationConfig::pbrVerification` 等更大的配置命名边界。
- 这一步继续保持不扩张 PBR；它只降低 application entry 与 PBR verification 命名的耦合。

### 2026-05-27 Runtime Verification Args Implementation Extraction

本轮继续上一节，把 verification args 的 implementation translation unit 从 `RuntimePBRVerificationArgs.cpp` 迁移到 `RuntimeVerificationArgs.cpp`。上一轮只完成 public header / include surface neutralization，本轮完成 `.cpp` 实现归位，并把内部 descriptor/table/function 命名从 PBR-specific 收敛到 runtime verification 语义。

新增与修改：

- 新增 `application/RuntimeVerificationArgs.cpp`，承载 `makeShellConfigFromArguments(...)` 的实际实现。
- `RuntimeVerificationArgs.cpp` 包含 `RuntimeVerificationArgs.h`，不再通过旧 PBR header 暴露实现。
- `RuntimePBRVerificationArgs.cpp` 改为兼容 translation unit，只包含 `RuntimePBRVerificationArgs.h`，不再定义 `makeShellConfigFromArguments(...)`。
- 内部 `PbrVerificationModeDescriptor` / `kPbrVerificationModes` / `applyPbrVerificationMode(...)` / `applyRequestedPbrVerificationModes(...)` 改名为 `RuntimeVerificationModeDescriptor` / `kRuntimeVerificationModes` / `applyRuntimeVerificationMode(...)` / `applyRequestedRuntimeVerificationModes(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 `RuntimeVerificationArgs.cpp`，并保留旧 `RuntimePBRVerificationArgs.cpp` 兼容 stub。

已完成验证：

- 静态检查确认 `makeShellConfigFromArguments(...)` 只在 `RuntimeVerificationArgs.cpp` 定义。
- 静态检查确认旧 `PbrVerificationModeDescriptor` / `kPbrVerificationModes` 等内部实现命名已无残留。
- 静态检查确认 `RuntimeVerificationArgs.cpp` 和 `RuntimePBRVerificationArgs.cpp` 均已在 VS 工程中注册，且旧 `.cpp` 不再包含实现体。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimePBRVerificationArgs.cpp` 与 `RuntimeVerificationArgs.cpp` 已重新编译，三条 focused verification mode 全部通过；没有 duplicate symbol，仅保留既有 LNK4075 提示。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- runtime verification args 的正式 implementation 已进入 neutral `RuntimeVerificationArgs.cpp`。
- 旧 `RuntimePBRVerificationArgs.*` 现在只作为兼容 include / translation unit 保留。
- 剩余 PBR-specific 命名主要在真实配置字段和具体 probe option 上，例如 `RuntimeApplicationConfig::pbrVerification` 与 `RuntimePBRVerificationConfig`；这些字段仍准确描述当前 PBR verification data，需要单独设计再迁移。
- 这一步继续保持不扩张 PBR；它只完成 application entry args parsing boundary 的命名和 translation unit 收敛。

### 2026-05-27 Runtime Verification Config Field Neutralization

本轮继续上一节，把 application shell config 暴露给上层的 verification 字段从 PBR-specific 命名收敛为 runtime-level 命名。目标不是扩张 PBR verification，而是减少 application composition root 对 PBR 语义的直接暴露。

新增与修改：

- 新增 `application/RuntimeVerificationConfig.h`，定义 `RuntimeVerificationConfig` 作为当前 `RuntimePBRVerificationConfig` 的 neutral alias。
- `RuntimeApplicationShellConfig::pbrVerification` 改名为 `RuntimeApplicationShellConfig::verification`。
- `RuntimeApplicationConfigPolicy`、`RuntimeContentConfigPolicy`、`RuntimeApplicationShutdownLifecycle`、`RuntimeVerificationArgs`、`RuntimeContentLifecycle`、`RuntimeFrameLifecycle` 与 `RuntimeVerificationLifecycle` 已改为消费 `config.verification` / `RuntimeVerificationConfig`。
- `RuntimeVerificationLifecycle` 的 public method 参数从 `RuntimePBRVerificationConfig` 收敛为 `RuntimeVerificationConfig`，但内部仍委托现有 `RuntimePBRVerification` 实现。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 neutral config header。

已完成验证：

- 静态检查确认当前 `application` 代码中不再存在 `shellConfig.pbrVerification` 或 `config.pbrVerification`。
- 静态检查确认 `RuntimeApplicationConfig.h` 与 `RuntimeVerificationLifecycle.h` 只包含 neutral `RuntimeVerificationConfig.h`。
- 静态检查确认剩余 `RuntimePBRVerificationConfig` 集中在具体 PBR verification 实现、neutral alias 和历史文档记录中。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- application-level config 字段已经不再把 verification 入口命名为 PBR。
- 当前 `RuntimeVerificationConfig` 仍是兼容 alias，因为字段内容仍包含 PBR probe option；这避免了在同一轮拆数据模型时引入过大风险。
- 下一步如果继续收敛，应该拆出通用 verification 字段（enabled/maxFrames/capture/engine-world/noop backend 等）与 PBR-specific probe 字段，而不是继续扩张 PBR pass。

### 2026-05-30 Runtime Verification Config Data Model Split

本轮继续上一节，把 `RuntimeVerificationConfig` 从 `RuntimePBRVerificationConfig` 的兼容 alias 推进为真实的 runtime-level verification data model。目标是让 application 层持有 generic verification request，同时把 PBR probe、Engine World probe 与 renderer timing probe 放到明确的子配置中。

新增与修改：

- `RuntimeVerificationConfig` 现在是独立 struct，直接拥有 `enabled`、`maxFrames`、`captureFrame`、`capturePath` 这些 generic verification 字段。
- `RuntimeVerificationConfig` 新增 `pbr`、`engineWorld`、`renderer` 子配置，分别承载 PBR probe/pass option、Engine World verification option 和 renderer timing probe option。
- `RuntimePBRVerificationConfig` 删除 generic lifecycle/capture 字段、Engine World 字段和 renderer timing 字段，只保留 PBR-specific probe/pass 配置。
- `RuntimeVerificationArgs.cpp` 改为把参数解析结果写入 `config.verification.pbr`、`config.verification.engineWorld` 和 `config.verification.renderer`。
- `RuntimeContentConfigPolicy` 改为通过 `shellConfig.verification.engineWorld` 决定 world-driven minimal scene 与 scene probe setup。
- `RuntimePBRVerification` public API 现在接收完整 `RuntimeVerificationConfig`，内部只读取需要的 `pbr`、`engineWorld` 和 `renderer` 子配置。

已完成验证：

- 静态检查确认不存在 `RuntimeVerificationConfig = RuntimePBRVerificationConfig` alias。
- 静态检查确认 application 代码中不再存在 `verification.enablePbr*`、`verification.enableEngineWorld*`、`verification.enableRendererGpuTimingProbe` 这类 top-level probe 字段访问。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；仅保留既有 LNK4075 提示。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，覆盖 PBR probe、Engine World probe、scene package 和 no-op renderer backend。

结论：

- `RuntimeVerificationConfig` 现在不再是 PBR config 的别名，application 层已经拥有独立 runtime verification data model。
- PBR-specific verification 仍由 `RuntimePBRVerification` 执行，但它现在从 runtime-level config 中提取 PBR/EngineWorld/Renderer 子配置。
- 下一步建议继续把 `RuntimePBRVerification` 内部承担的 Engine World verification report 与 renderer backend verification report 逐步拆到更中性的 verification report 模块；不建议继续扩张 PBR pass。

### 2026-05-30 Runtime Verification Report Extraction

本轮继续上一节，把 `RuntimePBRVerification` 中已经不属于 PBR 的 runtime / Engine / renderer backend verification report 迁移到独立 `RuntimeVerificationReport`。目标是让 PBR 文件继续负责 PBR scene / renderer stats，而 runtime lifecycle、subsystem、backend contract 和 cleanup report 进入中性 verification report 模块。

新增与修改：

- 新增 `application/RuntimeVerificationReport.h/.cpp`。
- `RuntimeVerificationReport::reportRenderedFrameRuntimeStats(...)` 负责输出 `Runtime engine lifecycle snapshot stats`、`Runtime engine subsystem summary stats`、`Runtime engine tick stats`、`Runtime subsystem health stats`、`Runtime renderer subsystem stats` 和 `Runtime renderer backend contract stats`。
- `RuntimeVerificationReport::reportRendererSubsystemCleanup(...)` 负责输出 renderer subsystem cleanup 与 renderer backend contract cleanup stats。
- `RuntimeVerificationReport::reportEngineWorldCleanup(...)` 与 `reportEngineSubsystemCleanup(...)` 负责输出 Engine World / subsystem cleanup stats。
- `RuntimeVerificationLifecycle` 在 capture 后先调用 `RuntimeVerificationReport` 输出 generic runtime/backend stats，再调用 `RuntimePBRVerification::reportRenderedFrame(...)` 输出 PBR renderer stats。
- `RuntimePBRVerification` 删除 generic runtime/backend cleanup report API，只保留 PBR profile、scene probe、prepared scene、PBR renderer stats 和 framebuffer capture。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 report 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeVerificationLifecycle` 调用 `RuntimeVerificationReport` 处理 generic runtime/backend/cleanup report。
- 静态检查确认 `RuntimePBRVerification.cpp` 中只保留 `PBR verification renderer stats`，`Runtime engine lifecycle snapshot` 与 `Runtime renderer backend contract` 输出迁入 `RuntimeVerificationReport.cpp`。
- 静态检查确认 `RuntimePBRVerification::reportRendererSubsystemCleanup`、`reportEngineWorldCleanup`、`reportEngineSubsystemCleanup` 不再存在。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeVerificationReport.cpp` 已参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- runtime / Engine / renderer backend verification report 已脱离 PBR 文件，进入中性 `RuntimeVerificationReport`。
- `RuntimePBRVerification` 仍承担 Engine World prepared-scene snapshot / scene-package roundtrip 等逻辑；下一步可以继续把 prepared-scene 里的 Engine World snapshot、scene package 和 negative package probe 拆出，进一步缩小 PBR 文件职责。
- 这一步仍不扩张 PBR pass，只减少 verification report 的命名和职责耦合。

### 2026-05-30 Runtime Engine World Verification Extraction

本轮根据 `/subagents` 指令先更新多 agent 协作边界，并启动只读 sidecar 审查 prepared-scene 拆分边界。实现上继续上一节，把 `RuntimePBRVerification` 中剩余的 generic Engine World prepared-scene verification 迁出到独立 `RuntimeEngineWorldVerification`，让 PBR 文件只保留 PBR profile、PBR probe、PBR scene stats、PBR imported asset stats、PBR renderer stats 和 framebuffer capture。

新增与修改：

- 新增 `application/RuntimeEngineWorldVerification.h/.cpp`。
- `RuntimeEngineWorldVerification::addVerificationSceneProbes(...)` 接管 `Engine world editor create stats` 相关 probe 和输出。
- `RuntimeEngineWorldVerification::reportPreparedScene(...)` 接管 transform snapshot save/apply、scene package round-trip、scene package negative probes、package light probe 和 package runtime asset resolver。
- `RuntimePBRVerification::reportPreparedScene(...)` 在该 extraction 阶段只输出 `PBR verification scene stats`，并临时通过 `RuntimeEngineWorldVerification::countRuntimeWorldActors(...)` 获取 `runtimeWorldActors`；下一节已继续拆出该兼容字段。
- `RuntimePBRVerification::addVerificationSceneProbes(...)` 现在只负责 PBR probe 创建，不再创建 editor-created actor。
- `RuntimeVerificationLifecycle::reportPreparedScene(...)` 调用顺序改为：Engine World verification probe、PBR probe、PBR renderer pass profile、PBR scene stats、Engine World prepared-scene report。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 Engine World verification 源文件和头文件。

已完成验证：

- 子 agent `Prepared Scene Verification Extraction Agent` 完成只读审查，确认最小拆分边界是 `RuntimeEngineWorldVerification`，且不应移动 `PBR verification scene stats`。
- 静态检查确认 `Engine world transform snapshot`、`Engine world scene package stats`、`Engine world scene package negative stats`、`Engine world editor create stats`、`ScenePackageNegativeProbeStats` 和 `RuntimeScenePackageAssetResolver` 只存在于 `RuntimeEngineWorldVerification.cpp`。
- 静态检查确认 `PBR verification scene stats`、`PBR imported asset engine world import stats`、`PBR imported asset scene package stats`、`Runtime asset registry stats` 和 `PBR verification renderer stats` 仍留在 `RuntimePBRVerification.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,import,engine-world-minimal-scene,engine-world-editor-create,engine-world-scene-package -DiscardCaptures`：构建通过；`RuntimeEngineWorldVerification.cpp`、`RuntimePBRVerification.cpp` 和 `RuntimeVerificationLifecycle.cpp` 已参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Engine World prepared-scene verification 已从 PBR 文件中迁出，PBR verification 文件的职责进一步收缩。
- 该 extraction 完成时曾临时保留 `PBR verification scene stats` 中的 `engineWorld*` 与 `runtimeWorldActors` 兼容字段；下一节已把这些字段拆到 neutral Engine World prepared-scene stats 行。
- 这一步继续保持不扩张 PBR pass，只降低 verification 命名和职责耦合。

### 2026-05-30 Runtime Engine World Prepared Scene Stats Neutralization

本轮继续上一节，把 Engine World prepared-scene counters 从 `PBR verification scene stats` 中拆出。目标是让 PBR scene stats 只描述 PBR/render-scene 事实，把 World actor 数和 Engine World 专用 mesh counter 交给 `RuntimeEngineWorldVerification` 的中性输出行。

新增与修改：

- `RuntimePBRVerification::reportPreparedScene(...)` 现在只输出 PBR/render-scene counters：objects、meshes、PBR mesh 分类、import/texture/showcase counters 与 `iblReady`。
- `RuntimePBRVerification.cpp` 删除 `engineWorldProbeMeshes`、`engineWorldMinimalMeshes` 和 `runtimeWorldActors` 的 scene stats 字段、统计逻辑和输出。
- `RuntimeEngineWorldVerification.cpp` 新增 `EngineWorldPreparedSceneStats`，遍历 prepared render scene 中的 Engine World probe/minimal mesh，并通过 `countRuntimeWorldActors(...)` 读取 runtime World actor 数。
- `RuntimeEngineWorldVerification::reportPreparedScene(...)` 新增中性输出行：`Engine world prepared scene stats: engineWorldProbeMeshes=..., engineWorldMinimalMeshes=..., runtimeWorldActors=...`。
- `tools/verify_pbr.ps1` 新增 `EngineWorldPreparedSceneStats` 捕获和 summary 输出，所有 `engineWorldProbeMeshes`、`engineWorldMinimalMeshes`、`runtimeWorldActors` 断言均改为读取 neutral Engine World stats line，而不是 `PBR verification scene stats`。
- `docs/subagents_coordination.md` 新增本轮只读 `Prepared Scene Stats Neutralization Agent` 边界，并记录 sidecar 审查只读完成。

已完成验证：

- 只读子 agent `Prepared Scene Stats Neutralization Agent` 完成审查，确认代码侧 PBR stats 已清理，Engine World neutral stats line 已接管三个 counter；指出当前文档需要更新。
- 静态检查确认 `RuntimePBRVerification.cpp` 中不再出现 `engineWorldProbeMeshes`、`engineWorldMinimalMeshes` 或 `runtimeWorldActors` 输出。
- 静态检查确认 `RuntimeEngineWorldVerification.cpp` 输出 `Engine world prepared scene stats`，`tools/verify_pbr.ps1` 从 `$engineWorldPreparedSceneLine` 读取三个 Engine World counter。
- 静态检查确认 `tools/verify_pbr.ps1` 中不存在 `Get-RegexValue -Text $sceneLine` 读取 `runtimeWorldActors`、`engineWorldProbeMeshes` 或 `engineWorldMinimalMeshes` 的残留。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,import,engine-world-scene-probe,engine-world-minimal-scene,engine-world-editor-create,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeEngineWorldVerification.cpp` 与 `RuntimePBRVerification.cpp` 重新编译；七条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，所有 mode 均输出新的 `engineWorldPreparedScene` summary 行。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- PBR prepared-scene stats 已回到 PBR/render-scene 语义，不再承担 Engine World actor/counter 兼容字段。
- Engine World prepared-scene counters 已进入 neutral `RuntimeEngineWorldVerification` 输出面，verification 脚本也已迁移到该新契约。
- 下一步应继续收敛 application startup / content composition，或继续把剩余 PBR import compatibility report 做边界拆分；当前仍不建议扩张 PBR feature scope。

### 2026-05-30 Runtime Imported Asset Verification Extraction

本轮继续上一节的边界收敛，把 imported asset probe 中的 Engine World import、AssetRegistry stats 和 scene package manifest round-trip 从 `RuntimePBRVerification` 中迁出。目标不是改变 `--verify-pbr-import` 的行为，也不是扩张 PBR import 功能，而是让 asset / package verification 职责从 PBR scene/render stats 文件中分离。

新增与修改：

- 新增 `application/RuntimeImportedAssetVerification.h/.cpp`。
- `RuntimeImportedAssetVerification::addVerificationSceneProbes(...)` 接管 `fbx/test/test.fbx` imported asset probe 的旧 renderer scene 注入、runtime World import、AssetRegistry handle 注册统计和 imported asset scene package manifest 保存/加载验证。
- 现有输出字符串保持兼容：`PBR imported asset engine world import stats`、`Runtime asset registry stats` 和 `PBR imported asset scene package stats` 没有改名，避免破坏 `tools/verify_pbr.ps1` 断言。
- `RuntimePBRVerification::addVerificationSceneProbes(...)` 删除 imported asset probe 实现，只保留 PBR material probe、texture set probe、showcase scene 等 PBR render-scene 相关 probe。
- `RuntimePBRVerification.cpp` 移除 `AssimpLoader`、`AssetRegistry`、`AssetSubsystem`、`LegacySceneWorldBuilder`、`PersistentIdPolicy` 和 `ScenePackage` 相关直接依赖。
- `RuntimeVerificationLifecycle::reportPreparedScene(...)` 现在按顺序调用 Engine World probe、PBR probe、Imported Asset verification probe、renderer pass profile、PBR scene stats 和 Engine World prepared-scene report。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 Imported Asset verification 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimePBRVerification.cpp` 中不再包含 imported asset world import / registry / scene package stats formatter，也不再直接包含 `AssimpLoader`、`AssetRegistry`、`AssetSubsystem`、`LegacySceneWorldBuilder`、`PersistentIdPolicy` 或 `ScenePackage`。
- 静态检查确认 `RuntimeImportedAssetVerification` 已在 lifecycle 和 VS 工程中注册，并拥有现有 imported asset verification 输出。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes import,forward,engine-world-scene-package -DiscardCaptures`：构建通过；`RuntimeImportedAssetVerification.cpp`、`RuntimePBRVerification.cpp` 和 `RuntimeVerificationLifecycle.cpp` 重新编译；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- imported asset 的 Engine World / AssetRegistry / ScenePackage verification 已从 PBR 文件中迁出。
- `RuntimePBRVerification` 现在更接近 PBR render verification 本体：profile、PBR material probes、PBR scene stats、PBR renderer stats 和 framebuffer capture。
- 下一步仍建议优先收敛 application startup/content composition；如果继续切 verification，应该围绕剩余 PBR renderer/capture 边界做小切片，而不是新增 PBR 功能。

### 2026-05-30 Runtime Verification Capture Extraction

本轮继续上一节，把默认 framebuffer capture 和 PPM 写盘从 `RuntimePBRVerification` 迁出。capture 是 generic verification lifecycle 职责，不应由 PBR renderer verification 类暴露 API。

新增与修改：

- 新增 `application/RuntimeVerificationCapture.h/.cpp`。
- `RuntimeVerificationCapture::captureDefaultFramebuffer(...)` 接管默认 framebuffer 绑定、`glReadPixels`、PPM 行翻转写出、capture 目录创建和失败日志。
- capture 日志从 `PBR verification capture ...` 调整为 `Runtime verification capture ...`，因为该行为对所有 verification mode 通用。
- `RuntimeVerificationLifecycle::captureFrameIfNeeded(...)` 改为调用 `RuntimeVerificationCapture::captureDefaultFramebuffer(...)`。
- `RuntimePBRVerification` 删除 `captureDefaultFramebuffer(...)` public API，并移除 `writePpmRows(...)`、`GL_CALL` / OpenGL framebuffer readback、`std::filesystem`、`std::ofstream`、`std::vector` 等 capture 相关实现依赖。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 capture 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimePBRVerification.h/.cpp` 不再暴露或实现 capture API。
- 静态检查确认 `RuntimeVerificationLifecycle.cpp` 调用 `RuntimeVerificationCapture::captureDefaultFramebuffer(...)`，`RuntimeVerificationCapture.cpp` 是唯一持有 `glReadPixels` / `writePpmRows` / capture PPM 写盘实现的 runtime verification 模块。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeVerificationCapture.cpp` 与相关 lifecycle / verification 文件重新编译，两条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，说明默认 capture 文件仍可写出并被脚本读取。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- framebuffer capture 已脱离 PBR verification 类，进入 neutral runtime verification capture module。
- `RuntimeVerificationLifecycle` 不再为了 capture 依赖 `RuntimePBRVerification`。
- `RuntimePBRVerification` 当前进一步收缩为 PBR profile、PBR scene probes、PBR scene stats 和 PBR renderer stats；后续若继续拆 verification，应优先考虑 PBR renderer stats 是否需要保留为单独模块，或者转向 application startup/content composition。

### 2026-05-30 Runtime PBR Renderer Stats Verification Extraction

本轮继续上一节，把 capture 帧输出的 PBR renderer stats 从 `RuntimePBRVerification` 迁出。该输出仍然是 PBR renderer verification 契约，不能改名，否则会破坏 `tools/verify_pbr.ps1`、golden baseline 和历史 profiling 文档；本轮只改变代码 ownership。

新增与修改：

- 新增 `application/RuntimePBRRendererStatsVerification.h/.cpp`。
- `RuntimePBRRendererStatsVerification::reportRenderedFrame(...)` 接管 `PBR verification renderer stats` 输出，包括 forward draw calls、shadow atlas counters、G-buffer/deferred draw calls、tiled/clustered light grid stats 和 GPU timing counters。
- `RuntimeVerificationLifecycle::captureFrameIfNeeded(...)` 在 generic runtime report 后直接调用 `RuntimePBRRendererStatsVerification::reportRenderedFrame(context)`。
- `RuntimePBRVerification` 删除 `reportRenderedFrame(...)` public API 和实现，不再为了 rendered-frame stats 暴露 Engine / RendererSubsystem 参数。
- `RuntimePBRVerification.cpp` 移除 `RendererSubsystem.h` 依赖；当前职责进一步收缩为 PBR profile、PBR scene probes 和 prepared scene stats。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 PBR renderer stats verification 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimePBRRendererStatsVerification` 已在 lifecycle、VS 工程和 filters 中注册，并且 `PBR verification renderer stats` 只由新模块输出。
- 静态检查确认 `RuntimePBRVerification.h/.cpp` 不再暴露或实现 `RuntimePBRVerification::reportRenderedFrame(...)`，也不再持有 renderer stats 输出字段。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,deferred-tiled-lights-pressure,deferred-clustered-grid-timing,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimePBRRendererStatsVerification.cpp` 参与编译，四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，说明 renderer stats 输出契约保持兼容。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- capture 帧的 PBR renderer stats 已脱离 `RuntimePBRVerification`，进入专门的 PBR renderer stats verification module。
- `RuntimeVerificationLifecycle` 现在按顺序调用 generic runtime report、PBR renderer stats report 和 capture flag 更新，边界更清晰。
- `RuntimePBRVerification` 当前只剩 PBR startup profile、renderer pass profile、PBR scene probe construction 和 prepared scene stats；下一步更适合回到 application startup/content composition，或继续细拆 PBR profile/scene probe 边界，而不是继续扩张 PBR pass。

### 2026-05-30 Runtime PBR Profile Verification Extraction

本轮继续上一节，把 PBR verification startup profile 和 renderer pass profile 从 `RuntimePBRVerification` 迁出。该切片不改变 profile 输出字符串，也不改变 renderer pass order；目标是让 PBR scene probe/stats 与 runtime startup/profile 配置分开。

新增与修改：

- 新增 `application/RuntimePBRProfileVerification.h/.cpp`。
- `RuntimePBRProfileVerification::applyProfile(...)` 接管 procedural IBL、post-process、runtime frame pipeline profile、PBR preview grid、light rig、camera rig 和 `PBR verification profile applied` 输出。
- `RuntimePBRProfileVerification::applyRendererPassProfile(...)` 接管 PBR forward/deferred/G-buffer/debug/tiled/clustered/GPU timing pass profile 写入。
- `RuntimeVerificationLifecycle::applyStartupProfile(...)` 改为调用 `RuntimePBRProfileVerification::applyProfile(...)`。
- `RuntimeVerificationLifecycle::reportPreparedScene(...)` 在 scene probes 注入后改为调用 `RuntimePBRProfileVerification::applyRendererPassProfile(...)`。
- `RuntimePBRVerification` 删除 `applyProfile(...)` 和 `applyRendererPassProfile(...)` public API，不再持有 light/camera/pass profile 写入逻辑。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 PBR profile verification 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimePBRProfileVerification` 已在 lifecycle、VS 工程和 filters 中注册，并且 light rig、camera rig、profile line 和 renderer pass profile 写入集中在新模块。
- 静态检查确认 `RuntimePBRVerification.h/.cpp` 不再暴露或实现 `RuntimePBRVerification::applyProfile(...)` / `RuntimePBRVerification::applyRendererPassProfile(...)`，也不再持有 `PBR verification profile applied`、`applyPressurePointLightRig`、light/camera profile 或 GPU timing pass-profile 写入。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,deferred-tiled-lights-pressure,deferred-clustered-grid-timing,showcase-spheres,engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimePBRProfileVerification.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，说明 profile / pass profile 输出契约保持兼容。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- PBR startup/profile 配置已脱离 `RuntimePBRVerification`，进入专门的 PBR profile verification module。
- `RuntimePBRVerification` 当前进一步收缩为 PBR scene probe construction 和 prepared scene stats。
- 下一步如继续切 verification，应优先考虑把 PBR scene probe construction 与 prepared-scene stats 再拆开；若回到主线，则继续收敛 application startup/content composition。

### 2026-05-30 Runtime PBR Scene Probe Verification Extraction

本轮继续上一节，把 PBR verification scene probe construction 从 `RuntimePBRVerification` 迁出。这个切片不改变任何 probe 输出契约，也不新增 PBR 功能；目标是让 scene probe 注入和 prepared-scene stats 分离。

新增与修改：

- 新增 `application/RuntimePBRSceneProbeVerification.h/.cpp`。
- `RuntimePBRSceneProbeVerification::addVerificationSceneProbes(...)` 接管 transparent fallback、deferred emissive、material IBL、alpha mask、texture set 和 showcase spheres 的 scene probe construction。
- `RuntimeVerificationLifecycle::reportPreparedScene(...)` 改为调用 `RuntimePBRSceneProbeVerification::addVerificationSceneProbes(...)`。
- `RuntimePBRVerification` 删除 `addVerificationSceneProbes(...)` public API；当前只保留 PBR prepared scene stats 输出。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 PBR scene probe verification 源文件和头文件。
- 本轮按用户 `/subagents` 指令启动只读 sidecar agent 审查多 agent 边界与文档落点；该 agent 未修改文件，建议已整合到 `docs/subagents_coordination.md`。

已完成验证：

- 静态检查确认 `RuntimePBRVerification.h/.cpp` 不再暴露或实现 `RuntimePBRVerification::addVerificationSceneProbes(...)`。
- 静态检查确认 transparent/emissive/material IBL/alpha mask/texture set/showcase probe construction helper 集中在 `RuntimePBRSceneProbeVerification.cpp`；`RuntimePBRVerification.cpp` 中保留的 `PBR Showcase Sphere` 字符串只用于 stats 统计。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes deferred-transparent,deferred-emissive,deferred-material-ibl,deferred-alpha-mask,deferred-texture-set,showcase-spheres -DiscardCaptures`：构建通过；`RuntimePBRSceneProbeVerification.cpp` 参与编译，六条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，说明 scene probe 注入与既有输出契约保持兼容。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- PBR scene probe construction 已脱离 `RuntimePBRVerification`，进入专门的 PBR scene probe verification module。
- `RuntimePBRVerification` 当前只保留 PBR prepared scene stats，职责边界比上一轮更窄。
- 下一步如果继续切 verification，可把 prepared scene stats 进一步拆成 `RuntimePBRPreparedSceneStatsVerification`；如果回到主线，则继续收敛 application startup/content composition。

### 2026-05-30 Runtime PBR Prepared Scene Stats Verification Extraction

本轮继续上一节，把 `PBR verification scene stats` 的 prepared-scene 统计从 `RuntimePBRVerification` 迁出。该输出仍然是现有 verification 契约，不能改名；本轮只改变代码 ownership，并移除旧 `RuntimePBRVerification.cpp`。

新增与修改：

- 新增 `application/RuntimePBRPreparedSceneStatsVerification.h/.cpp`。
- `RuntimePBRPreparedSceneStatsVerification::reportPreparedScene(...)` 接管 `PBR verification scene stats` 输出，包括 object/mesh/PBR mesh、transparent、emissive、custom IBL、alpha mask、imported、texture set、showcase spheres 和 `iblReady` counters。
- `RuntimeVerificationLifecycle::reportPreparedScene(...)` 改为调用 `RuntimePBRPreparedSceneStatsVerification::reportPreparedScene(...)`。
- 删除 `application/RuntimePBRVerification.cpp`，并从 `text2.vcxproj` / `text2.vcxproj.filters` 移除该 translation unit。
- 在本切片完成时，`RuntimePBRVerification.h` 只保留 `RuntimePBRVerificationConfig`，作为下一步 config header rename 的临时承载头；旧 `RuntimePBRVerification` class 已移除。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 PBR prepared scene stats verification 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimePBRPreparedSceneStatsVerification` 已在 lifecycle、VS 工程和 filters 中注册，并且 `PBR verification scene stats` 只由新模块输出。
- 静态检查确认 `RuntimePBRVerification::...`、`class RuntimePBRVerification` 和 `RuntimePBRVerification.cpp` 工程注册均无残留。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,import,deferred-transparent,deferred-emissive,deferred-material-ibl,deferred-alpha-mask,deferred-texture-set,showcase-spheres,engine-world-minimal-scene -DiscardCaptures`：构建通过；`RuntimePBRPreparedSceneStatsVerification.cpp` 参与编译，九条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过，说明 PBR scene stats 输出契约保持兼容。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- 旧 `RuntimePBRVerification` class 与 `.cpp` 已被移除；PBR verification 运行时职责已经分散到 profile、scene probe、prepared scene stats、renderer stats、imported asset、capture 和 generic report 等专门模块。
- 本切片完成时仅遗留 `RuntimePBRVerification.h` 承载 `RuntimePBRVerificationConfig`；该临时遗留已在下一节迁移为 `RuntimePBRVerificationConfig.h`。
- 如果回到主线，优先继续收敛 application startup/content composition 或 renderer backend contract，而不是扩张 PBR 功能。

### 2026-05-30 Runtime PBR Verification Config Header Rename

本轮继续上一节，把最后一个只因历史命名存在的 `RuntimePBRVerification.h` 重命名为明确的 config header。该切片不改变任何 verification 参数、输出字符串或 PBR 功能，只改变 config ownership 与工程注册命名。

新增与修改：

- 新增 `application/RuntimePBRVerificationConfig.h`。
- `RuntimePBRVerificationConfig` 从旧 `RuntimePBRVerification.h` 迁入 `RuntimePBRVerificationConfig.h`。
- `RuntimeVerificationConfig.h` 改为包含 `RuntimePBRVerificationConfig.h`，继续通过 `RuntimeVerificationConfig::pbr` 暴露 PBR-specific verification config。
- 删除 `application/RuntimePBRVerification.h`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 改为注册 `application\RuntimePBRVerificationConfig.h`，不再注册旧 header。
- 按用户 `/subagents` 指令更新 `docs/subagents_coordination.md`：父 agent 继续负责最终集成、测试和 `worked.md`，sidecar 默认只读，写入必须显式分配且互不重叠。

已完成验证：

- 静态检查确认 `RuntimePBRVerificationConfig.h` 已在 `RuntimeVerificationConfig.h`、VS 工程和 filters 中注册。
- 静态检查确认 live code / VS 工程文件中不再存在 `RuntimePBRVerification.h`、`RuntimePBRVerification.cpp`、`RuntimePBRVerification::...` 或 `class RuntimePBRVerification`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,import,deferred-clustered-grid,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- 旧 `RuntimePBRVerification` class、`.cpp` 和 `.h` 已从 live code 与 VS 工程注册中移除。
- PBR verification config 现在有明确的 `RuntimePBRVerificationConfig.h` ownership，不再借用旧 verification class/header 命名。
- 下一步如果继续切 verification，可考虑把 PBR verification config 按 pass/probe 分组；如果回到主线，优先继续收敛 application startup/content composition 或 renderer backend contract，而不是扩张 PBR 功能。

### 2026-05-30 Runtime PBR Verification Config Pass/Probe/Deferred Split

本轮继续上一节，把 `RuntimePBRVerificationConfig` 从 flat field list 拆成按职责分组的 config data model。该切片不改变任何命令行参数、verification 输出字符串或 renderer pass 行为；目标是降低 args/profile/probe/import verification 对单个 PBR config 大结构的耦合。

新增与修改：

- `RuntimePBRVerificationConfig.h` 新增 `RuntimePBRVerificationPassConfig`、`RuntimePBRVerificationProbeConfig` 和 `RuntimePBRVerificationDeferredConfig`。
- `RuntimePBRVerificationConfig` 现在只聚合 `passes`、`probes`、`deferred` 三个子配置。
- `RuntimeVerificationArgs.cpp` 把 mode descriptor options 写入对应子配置：渲染 pass 开关进入 `pbr.passes`，scene/light/import probes 进入 `pbr.probes`，deferred tiled light override 进入 `pbr.deferred`。
- `RuntimePBRProfileVerification.cpp` 改为按 `passes` / `probes` / `deferred` 读取配置，保留原有 profile line 和 renderer pass profile 行为。
- `RuntimePBRSceneProbeVerification.cpp` 只读取 `pbr.probes`。
- `RuntimeImportedAssetVerification.cpp` 只读取 `pbr.probes.enablePbrImportedAssetProbe`。
- 本轮继续按 `/subagents` 协作边界启动只读 sidecar agent 审查 flat field 残留；父 agent 仍负责最终集成、验证和文档。

已完成验证：

- 静态检查确认 `application/tools/engine/renderer/main.cpp` 中不再存在 `verification.pbr.enable...`、`config.enable...` 或 `pbr.enable...` 这类旧 flat field 访问。
- 静态检查确认 `RuntimePBRVerificationPassConfig`、`RuntimePBRVerificationProbeConfig`、`RuntimePBRVerificationDeferredConfig` 和 `passes` / `probes` / `deferred` 调用点可检索。
- 第一次 focused verification 命令因使用不存在的 mode `deferred-tiled-lights-custom` 被脚本拒绝，未进入构建；随后改用脚本支持的 `deferred-tiled-lights-32` 与 `deferred-tiled-lights-cutoff-005`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,deferred-transparent,deferred-material-ibl,import,deferred-tiled-lights-32,deferred-tiled-lights-cutoff-005,deferred-clustered-grid-timing,showcase-spheres,renderer-backend-registry-noop -DiscardCaptures`：构建通过；focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- PBR verification config 已完成第一层职责分组；args、profile、scene probe 和 imported asset verification 不再直接依赖一个 flat PBR config 字段列表。
- 这仍然是 verification 内部数据模型调整，不扩张 PBR 功能，也不改变现有脚本契约。
- 后续如果继续切 verification，可优先处理 light-camera rig policy 或 probe policy；pass profile、profile reporting 与 preview ownership 已在后续章节继续拆出。如果回到主线，优先继续收敛 application startup/content composition 或 renderer backend contract。

### 2026-05-30 Runtime PBR Pass Profile Verification Extraction

本轮继续上一节，把 renderer pass profile 写入从 `RuntimePBRProfileVerification` 再拆成独立小模块。该切片不改变 pass order、verification 输出字符串或 PBR 功能；目标是让 startup profile / preview / light-camera rig 与 renderer pass profile policy 分离。

新增与修改：

- 新增 `application/RuntimePBRPassProfileVerification.h/.cpp`。
- `RuntimePBRPassProfileVerification::applyRendererPassProfile(...)` 接管 forward/deferred/G-buffer/debug/tiled/clustered/GPU timing 相关 renderer frame pass profile 写入。
- `RuntimePBRProfileVerification::applyProfile(...)` 仍负责 procedural IBL、post process、runtime frame pipeline profile、PBR preview grid、light rig、camera rig 和 `PBR verification profile applied` 输出。
- `RuntimeVerificationLifecycle::reportPreparedScene(...)` 改为直接调用 `RuntimePBRPassProfileVerification::applyRendererPassProfile(...)`，prepared-scene probe 注入后不再绕回 startup profile module。
- `RuntimePBRProfileVerification.h` 删除 `applyRendererPassProfile(...)` public API；profile module 不再为了 pass profile 写入直接包含 renderer header。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 PBR pass profile verification 源文件和头文件。
- 按用户 `/subagents` 指令更新 `docs/subagents_coordination.md`：本轮 sidecar 默认只读，父 agent 保留最终集成、验证和 `worked.md` 责任；随后启动只读 sidecar `Kuhn` 审查本轮抽离残留。

已完成验证：

- 静态检查确认 `RuntimePBRProfileVerification::applyRendererPassProfile(...)` 和 profile header 中的旧 `static applyRendererPassProfile(...)` 无残留。
- 静态检查确认 `RuntimePBRPassProfileVerification` 已在 lifecycle、VS 工程和 filters 中注册，`RuntimeVerificationLifecycle::reportPreparedScene(...)` 直接调用新 pass-profile module。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,gbuffer-debug,deferred,deferred-transparent,deferred-tiled-heatmap,deferred-clustered-grid-timing,showcase-spheres,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimePBRPassProfileVerification.cpp`、`RuntimePBRProfileVerification.cpp` 和 `RuntimeVerificationLifecycle.cpp` 参与编译，focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- `git diff --check`：没有 whitespace error，仅输出 Windows 工作区 LF/CRLF 转换提示。

结论：

- Renderer pass profile policy 已从 startup profile module 中拆出，PBR verification profile 的职责边界更清晰。
- 当前 PBR verification 继续保持“稳定现有功能、降低耦合”的方向，没有扩张 PBR pass。
- Application 层后续优先处理 startup/content composition；PBR verification 侧的 profile line、preview grid、light-camera rig policy 和 environment/post-process/frame-pipeline startup profile policy 已在后续章节拆出，后续不要增加新渲染特性。

### 2026-05-30 Runtime PBR Profile Line Verification Extraction

本轮继续上一节，把 `PBR verification profile applied` 输出行从 `RuntimePBRProfileVerification` 中拆出。该切片不改变任何 profile line 文本、verification 参数或渲染行为；目标是把“启动 profile 写入”和“verification profile 摘要报告”分离。

新增与修改：

- 新增 `application/RuntimePBRProfileLineVerification.h/.cpp`。
- `RuntimePBRProfileLineVerification::reportAppliedProfile(...)` 接管 `PBR verification profile applied` 字符串拼接与 `std::cout` / `LogInfo` 输出。
- `RuntimePBRProfileVerification::applyProfile(...)` 在完成 environment、post-process、frame pipeline、preview、light rig 和 camera rig 写入后调用新 profile-line module。
- `RuntimePBRProfileVerification.cpp` 不再直接包含 `<iostream>`、`<string>` 或 `../tools/Logger/LogManager.h`，也不再直接读取 pass/deferred/renderer timing config 来组装输出行。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 PBR profile line verification 源文件和头文件。

已完成验证：

- 静态检查确认 `PBR verification profile applied`、`LogInfo`、`std::cout`、`std::to_string` 和 pass/deferred/renderer timing profile-line 读取均集中在 `RuntimePBRProfileLineVerification.cpp`。
- 静态检查确认 `RuntimePBRProfileVerification.cpp` 中不再存在 profile line 输出字符串、`reportLine(...)`、`LogInfo(...)`、`std::cout` 或 `std::to_string`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,gbuffer-debug,deferred,deferred-transparent,deferred-material-ibl,import,deferred-tiled-lights-32,deferred-clustered-grid-timing,showcase-spheres,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimePBRProfileLineVerification.cpp` 与 `RuntimePBRProfileVerification.cpp` 参与编译，focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimePBRProfileVerification` 的职责继续收窄为 startup profile 写入，不再持有 verification summary line/reporting 细节。
- profile line module 仍保持原输出契约，避免影响现有脚本与日志阅读方式。
- 后续如果继续切 PBR verification，preview grid policy 已在下一节拆出，当前优先考虑 light-camera rig policy；如果回到主线，则继续收敛 application startup/content composition。

### 2026-05-30 Runtime PBR Preview Profile Verification Extraction

本轮继续上一节，把 verification preview grid policy 从 `RuntimePBRProfileVerification` 中拆出。该切片不改变 preview grid 默认值、showcase override、minimal-scene disable 行为或验证输出；目标是继续缩小 startup profile module 的职责。

新增与修改：

- 新增 `application/RuntimePBRPreviewProfileVerification.h/.cpp`。
- `RuntimePBRPreviewProfileVerification::applyPreviewProfile(...)` 接管 `context.pbrPreviewProfile` 的默认 PBR material grid、normal map、showcase spheres override 和 engine world minimal scene disable。
- `RuntimePBRProfileVerification::applyProfile(...)` 在 renderer pass profile 后委托调用 `RuntimePBRPreviewProfileVerification::applyPreviewProfile(context, verification)`。
- `RuntimePBRProfileVerification.cpp` 不再直接写入 `pbrPreviewProfile` 字段；该 module 仍负责 environment、post-process、frame pipeline、light rig、camera rig 和对 profile-line module 的调用。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 PBR preview profile verification 源文件和头文件。

已完成验证：

- 静态检查确认 `pbrPreviewProfile`、grid/normal map/showcase override 和 minimal-scene disable 写入集中在 `RuntimePBRPreviewProfileVerification.cpp`。
- 静态检查确认 `RuntimePBRProfileVerification.cpp` 中不再直接写入 `pbrPreviewProfile` 字段；剩余 `enablePbrShowcaseSpheres` 和 `enableMinimalScene` 访问仅服务 light/camera rig。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,deferred,showcase-spheres,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimePBRPreviewProfileVerification.cpp` 与 `RuntimePBRProfileVerification.cpp` 参与编译，focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimePBRProfileVerification` 不再持有 preview grid policy，职责继续向纯 startup profile composition 收敛。
- Preview grid policy 保持原行为，现有 scene stats 中的 `pbrPreviewMeshes` / `pbrShowcaseSpheres` 仍由 focused 和 full verification 覆盖。
- Light-camera rig policy 已在下一节拆出；environment/post-process/frame-pipeline startup profile policy 已在后续章节拆出；如果回到主线，则继续收敛 application startup/content composition。

### 2026-05-30 Runtime PBR Light Camera Rig Verification Extraction

本轮继续上一节，把 verification light/camera rig policy 从 `RuntimePBRProfileVerification` 中拆出。该切片不改变默认点光源、minimal-scene 灯光、tiled-light probe、pressure-light probe、showcase camera override 或 camera near/far/apply 行为；目标是让 startup profile module 不再直接持有场景观察与灯光实验参数。

新增与修改：

- 新增 `application/RuntimePBRLightCameraRigVerification.h/.cpp`。
- `RuntimePBRLightCameraRigVerification::applyLightCameraRig(...)` 接管 `context.pbrLightRigProfile` 和 `context.pbrCameraRigProfile` 的默认写入、minimal-scene override、tiled-light override、pressure-light rig 和 showcase camera override。
- `RuntimePBRProfileVerification::applyProfile(...)` 在 renderer pass profile 与 preview profile 后委托调用 `RuntimePBRLightCameraRigVerification::applyLightCameraRig(context, verification)`。
- `RuntimePBRProfileVerification.cpp` 不再直接写入 `pbrLightRigProfile` 或 `pbrCameraRigProfile` 字段；该 module 当前只保留 environment、post-process、frame pipeline startup composition，并委托 pass/profile-line/preview/light-camera concerns。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 PBR light/camera rig verification 源文件和头文件。
- 按 `/subagents` 协作边界启动只读 sidecar `Bohr` 审查文档一致性；该 agent 未修改文件，指出当前方向中仍把 light-camera rig policy 写成未来工作的 stale wording，已由父 agent 集成修正。

已完成验证：

- 静态检查确认 `RuntimePBRLightCameraRigVerification`、`applyLightCameraRig(...)`、工程注册和 light/camera policy 写入集中在新模块。
- 静态检查确认 `RuntimePBRProfileVerification.cpp` 中不再存在 `pbrLightRigProfile`、`pbrCameraRigProfile`、`applyPressurePointLightRig`、`PBRLightRigProfile`、`PBRCameraRigProfile`、`attenuationK2` 或 `applyTo(context.camera)`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,deferred-tiled-lights-pressure,showcase-spheres,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimePBRLightCameraRigVerification.cpp` 与 `RuntimePBRProfileVerification.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Light/camera rig policy 已从 startup profile composition 中拆出，PBR verification 的剩余 profile module 进一步收敛为 environment、post-process 和 frame pipeline startup defaults。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 verification 输出契约。
- Environment/post-process/frame-pipeline startup profile policy、application startup/content composition 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；后续优先继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-30 Runtime PBR Startup Profile Verification Extraction

本轮继续上一节，把 environment、post-process 和 runtime frame pipeline startup defaults 从 `RuntimePBRProfileVerification` 中拆出。该切片不改变 procedural environment、bloom/post-process 默认值、frame pipeline pass order、verification 输出或 renderer 行为；目标是让 `RuntimePBRProfileVerification` 变成纯 profile application 编排入口。

新增与修改：

- 新增 `application/RuntimePBRStartupProfileVerification.h/.cpp`。
- `RuntimePBRStartupProfileVerification::applyStartupProfile(...)` 接管 `context.environmentProfile`、`context.postProcessSettings` 和 `context.framePipelineProfile` 的 verification startup defaults。
- `RuntimePBRProfileVerification::applyProfile(...)` 现在只按顺序调用 startup defaults、renderer pass profile、preview profile、light/camera rig 和 profile-line reporting modules。
- `RuntimePBRProfileVerification.cpp` 不再直接写入 environment、post-process 或 frame-pipeline profile 字段，也不再需要包含 `AppRuntimeContext.h`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 PBR startup profile verification 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimePBRStartupProfileVerification`、`applyStartupProfile(...)`、工程注册和 startup defaults 写入集中在新模块。
- 静态检查确认 `RuntimePBRProfileVerification.cpp` 中不再存在 `environmentProfile`、`postProcessSettings`、`framePipelineProfile`、`precomputeOnPrepare`、`proceduralSkyIntensity`、`bloomThreshold`、`sceneColorPassEnabled` 或 `passOrder` 写入残留。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,deferred,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimePBRProfileVerification.cpp` 与 `RuntimePBRStartupProfileVerification.cpp` 参与编译，四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- PBR verification profile 拆分已经形成稳定职责链：startup defaults、pass profile、preview profile、light/camera rig 和 profile line 各自独立，`RuntimePBRProfileVerification` 只保留编排职责。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 verification 输出契约。
- Application startup/content composition 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；后续应继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals，而不是继续扩张 PBR pass。

### 2026-05-30 Runtime Content Lifecycle Config Header Extraction

本轮回到 Engine runtime ownership 主线，处理 application startup/content composition 的 public header 边界。`RuntimeContentLifecycle` 已经接管 startup content composition，但它的 public header 仍直接包含 camera lifecycle、scene preparer、engine lifecycle、verification lifecycle 和 legacy experiment headers。本切片把 content startup config DTOs 拆到独立 header，让调用方只依赖配置数据和前置声明。

新增与修改：

- 新增 `application/RuntimeCameraConfig.h`，从 `RuntimeCameraLifecycle.h` 迁出 `RuntimeCameraConfig`。
- 新增 `application/RuntimeScenePrepareConfig.h`，从 `RuntimeScenePreparer.h` 迁出 `RuntimeScenePrepareConfig`。
- 新增 `application/RuntimeContentLifecycleConfig.h`，从 `RuntimeContentLifecycle.h` 迁出 `RuntimeContentLifecycleConfig`。
- `RuntimeContentLifecycle.h` 现在只包含 `RuntimeContentLifecycleConfig.h`，并以前置声明暴露 `AppRuntimeContext`、`Engine`、`LegacyExperimentRunner` 和 `RuntimeEngineLifecycleState`。
- `RuntimeContentLifecycle.cpp` 当时保留实际实现依赖；后续 renderer backend lifecycle 与 content verification lifecycle 切片已继续移走 engine lifecycle/logger/profile loader/verification lifecycle 直接依赖。
- `RuntimeContentConfigPolicy.h` 改为依赖 `RuntimeContentLifecycleConfig.h`，不再为了 config 返回值包含完整 content lifecycle public API。
- `RuntimeCameraLifecycle.h` 和 `RuntimeScenePreparer.h` 也收窄为 config header + 前置声明，具体 `AppRuntimeContext`、legacy experiment 和 scene setup pipeline 实现依赖留在 `.cpp`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 config header。

已完成验证：

- 静态检查确认 `RuntimeCameraConfig`、`RuntimeScenePrepareConfig` 和 `RuntimeContentLifecycleConfig` 的定义集中在新增 header，工程和 filters 均已注册。
- 静态检查确认 `RuntimeContentLifecycle.h` 不再直接包含 `RuntimeCameraLifecycle.h`、`RuntimeEngineLifecycle.h`、`RuntimeScenePreparer.h`、`RuntimeVerificationLifecycle.h`、`AppRuntimeContext.h` 或 legacy experiment implementation header。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationStartupLifecycle.cpp`、`RuntimeContentConfigPolicy.cpp`、`RuntimeContentLifecycle.cpp`、`RuntimeCameraLifecycle.cpp` 和 `RuntimeScenePreparer.cpp` 参与编译，四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Content lifecycle public API 不再把 implementation lifecycle / legacy experiment 依赖扩散给调用者，application startup/content composition 边界更干净。
- 该切片不改变 startup order、scene preparation、renderer backend attachment 或 verification 输出。
- RuntimeContentLifecycle prepare 内部阶段与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；后续继续沿 Engine runtime ownership 主线收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-30 Runtime Content Renderer Backend Lifecycle Extraction

本轮继续拆分 `RuntimeContentLifecycle::prepare(...)` 内部阶段，把 scene preparation 后的 renderer fail-fast gate 与 Engine-owned renderer backend attachment 迁入专用 lifecycle module。目标是减少 generic content composition 对 `RuntimeEngineLifecycle` 和 logger 的直接依赖，同时保持 startup order、renderer backend selection、verification 输出和失败返回行为不变。

新增与修改：

- 新增 `application/RuntimeContentRendererBackendLifecycle.h/.cpp`。
- `RuntimeContentRendererBackendLifecycle::attachAfterScenePreparation(...)` 现在集中检查 `context.renderer`，在 scene prepare 未创建 renderer 时输出原有 `LogError` 并返回 `false`。
- `RuntimeContentRendererBackendLifecycle::attachAfterScenePreparation(...)` 通过 `RuntimeEngineLifecycle::attachRendererBackend(...)` 完成 Engine-owned renderer backend attachment。
- `RuntimeContentLifecycle.cpp` 不再直接包含 `RuntimeEngineLifecycle.h` 或 `LogManager.h`，也不再直接调用 `RuntimeEngineLifecycle::attachRendererBackend(...)`。
- `RuntimeContentLifecycle::prepare(...)` 保留 high-level startup content 顺序：camera init、profile load、verification startup profile、scene prepare、renderer backend lifecycle attach、prepared-scene report。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 renderer backend lifecycle 源文件和头文件。
- 本轮按 `/subagents` 要求启动只读 sidecar subagent `McClintock` 审查该切片；该 agent 未修改文件，确认源码边界和工程注册无阻塞问题，并指出文档需要同步，已在本节和 engine docs 中采纳。

已完成验证：

- 静态检查确认 `RuntimeContentRendererBackendLifecycle`、`attachAfterScenePreparation(...)`、fail-fast 日志、backend attach 调用和工程注册均可检索。
- 静态检查确认 `RuntimeContentLifecycle.cpp` 中不再直接调用 `RuntimeEngineLifecycle::attachRendererBackend(...)`，也不再直接记录 scene-prepare renderer failure 日志。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeContentLifecycle.cpp` 与 `RuntimeContentRendererBackendLifecycle.cpp` 参与编译，四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeContentLifecycle` 仍负责编排 startup content composition，但 renderer backend attach/fail-fast 已变成可独立审查和继续下沉的阶段。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 renderer backend contract。
- 下一步可继续拆分 content prepare 的 profile/scene/report 子阶段，或推进 renderer backend attachment 从 application-side lifecycle 进一步靠近 Engine-level contract。

### 2026-05-30 Runtime Content Verification Lifecycle Extraction

本轮继续拆分 `RuntimeContentLifecycle::prepare(...)` 内部阶段，把 runtime profile loading、verification startup profile application 和 prepared-scene verification report 收口到 content-side verification bridge。目标是让 `RuntimeContentLifecycle` 只保留高层 startup content 顺序，不再直接依赖 `RuntimeProfileLoader` 或 `RuntimeVerificationLifecycle`。

新增与修改：

- 新增 `application/RuntimeContentVerificationLifecycle.h/.cpp`。
- `RuntimeContentVerificationLifecycle::loadStartupProfiles(...)` 统一执行 `RuntimeProfileLoader::loadAll(...)` 与 `RuntimeVerificationLifecycle::applyStartupProfile(...)`，保持原有调用顺序。
- `RuntimeContentVerificationLifecycle::reportPreparedScene(...)` 统一转发 prepared-scene verification report。
- `RuntimeContentLifecycle.cpp` 不再直接包含 `RuntimeProfileLoader.h` 或 `RuntimeVerificationLifecycle.h`。
- `RuntimeContentLifecycle::prepare(...)` 现在按高层顺序委托：camera init、content verification startup profiles、scene prepare、renderer backend lifecycle attach、content verification prepared-scene report。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 content verification lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeContentVerificationLifecycle`、`loadStartupProfiles(...)`、`reportPreparedScene(...)`、profile load、verification startup/report 和工程注册均可检索。
- 静态检查确认 `RuntimeContentLifecycle.cpp` 中不再直接出现 `RuntimeProfileLoader` 或 `RuntimeVerificationLifecycle` 依赖；剩余 report 调用只指向 `RuntimeContentVerificationLifecycle`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeContentLifecycle.cpp` 与 `RuntimeContentVerificationLifecycle.cpp` 参与编译，四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeContentLifecycle` 的直接依赖继续减少，目前只剩 camera lifecycle、scene preparer、renderer backend lifecycle 和 content verification lifecycle 这些明确阶段。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 verification 输出契约。
- Scene preparation 阶段已在下一节拆出；后续可继续把 renderer backend attachment 推向更 Engine-level 的 contract。

### 2026-05-30 Runtime Content Scene Lifecycle Extraction

本轮继续拆分 `RuntimeContentLifecycle::prepare(...)` 内部阶段，把 scene preparation 的直接调用移入 content-side scene lifecycle。目标是让 `RuntimeContentLifecycle` 只保留高层 startup content 顺序，不再直接依赖 `RuntimeScenePreparer`，同时保持 scene setup pipeline、legacy experiments、Engine World prepared scene 输出和 renderer backend attach 顺序不变。

新增与修改：

- 新增 `application/RuntimeContentSceneLifecycle.h/.cpp`。
- `RuntimeContentSceneLifecycle::prepareScene(...)` 负责转发到 `RuntimeScenePreparer::prepare(...)`。
- `RuntimeContentLifecycle.cpp` 不再直接包含 `RuntimeScenePreparer.h`，也不再直接调用 `RuntimeScenePreparer::prepare(...)`。
- `RuntimeContentLifecycle::prepare(...)` 当前只按高层阶段委托：camera init、content verification startup profiles、content scene lifecycle prepare、renderer backend lifecycle attach、content verification prepared-scene report。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 content scene lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeContentSceneLifecycle`、`prepareScene(...)`、`RuntimeScenePreparer::prepare(...)` 转发点和工程注册均可检索。
- 静态检查确认 `RuntimeContentLifecycle.cpp` 中不再直接出现 `RuntimeScenePreparer` 依赖；剩余 scene prepare 调用只指向 `RuntimeContentSceneLifecycle`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeContentLifecycle.cpp` 与 `RuntimeContentSceneLifecycle.cpp` 参与编译，四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeContentLifecycle` 现在已收敛为高层 startup content sequence，不再直接依赖 scene preparer、profile loader、verification lifecycle、engine lifecycle 或 logger。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 verification 输出契约。
- Camera init stage 一致性命名已在下一节拆出；后续可推进 renderer backend attachment 从 application-side lifecycle 进一步靠近 Engine-level contract。

### 2026-05-30 Runtime Content Camera Lifecycle Extraction

本轮继续统一 content startup stage 的命名和边界，把 camera initialization 的直接调用移入 content-side camera lifecycle。目标是让 `RuntimeContentLifecycle` 的所有 startup content stages 都通过 `RuntimeContent*Lifecycle` 入口表达，同时保持原有 camera 创建、camera control 绑定和 cleanup 行为不变。

新增与修改：

- 新增 `application/RuntimeContentCameraLifecycle.h/.cpp`。
- `RuntimeContentCameraLifecycle::initializeCamera(...)` 负责转发到 `RuntimeCameraLifecycle::initializeDefaultCamera(...)`。
- `RuntimeContentLifecycle.cpp` 不再直接包含 `RuntimeCameraLifecycle.h`，也不再直接调用 `RuntimeCameraLifecycle::initializeDefaultCamera(...)`。
- `RuntimeContentLifecycle::prepare(...)` 当前只按高层 content stages 委托：content camera lifecycle、content verification lifecycle、content scene lifecycle、content renderer backend lifecycle、content verification prepared-scene report。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 content camera lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeContentCameraLifecycle`、`initializeCamera(...)`、`RuntimeCameraLifecycle::initializeDefaultCamera(...)` 转发点和工程注册均可检索。
- 静态检查确认 `RuntimeContentLifecycle.cpp` 中不再直接出现 `RuntimeCameraLifecycle` 或 `initializeDefaultCamera` 依赖；剩余 camera init 调用只指向 `RuntimeContentCameraLifecycle`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeContentCameraLifecycle.cpp` 与 `RuntimeContentLifecycle.cpp` 参与编译，四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeContentLifecycle` 现在已成为纯 startup content sequence coordinator，不再直接依赖 camera lifecycle、scene preparer、profile loader、verification lifecycle、engine lifecycle 或 logger。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 camera 或 verification 输出契约。
- Runtime legacy experiment lifecycle 已在下一节拆出；后续可继续推进 renderer backend attachment 更靠近 Engine-level contract，或继续收敛 scene setup / frame runner 边界。

### 2026-05-30 Runtime Legacy Experiment Lifecycle Extraction

本轮继续收敛 `RuntimeScenePreparer` 内部职责，把 legacy experiment context construction、startup enable hook 注释区和 per-frame update 迁入独立 lifecycle。目标是让 `RuntimeScenePreparer` 只负责 scene setup pipeline 和 Engine World prepared-scene stats，而 `RuntimeFrameRunner` 不再为了每帧 legacy experiment update 依赖 scene preparer。

新增与修改：

- 新增 `application/RuntimeLegacyExperimentLifecycle.h/.cpp`。
- `RuntimeLegacyExperimentLifecycle::makeContext(...)` 集中构造 `GL_EXPERIMENTS::RuntimeContext`。
- `RuntimeLegacyExperimentLifecycle::prepare(...)` 保留原有 legacy experiment startup enable hook 注释区，默认不启用任何 legacy experiment，保持 disabled 行为。
- `RuntimeLegacyExperimentLifecycle::update(...)` 集中调用 `LegacyExperimentRunner::update(...)`。
- `RuntimeScenePreparer.cpp` 不再暴露或实现 `makeLegacyExperimentContext(...)`、`prepareLegacyExperiments(...)` 或 `updateLegacyExperiments(...)`；scene prepare 完成后只委托 `RuntimeLegacyExperimentLifecycle::prepare(...)`。
- `RuntimeFrameRunner.cpp` 不再包含 `RuntimeScenePreparer.h`，也不再调用 `RuntimeScenePreparer::updateLegacyExperiments(...)`，每帧改为调用 `RuntimeLegacyExperimentLifecycle::update(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 legacy experiment lifecycle 源文件和头文件。

Subagent 审查：

- 本轮启动只读 sidecar subagent `Darwin` 审查该切片。
- `Darwin` 确认抽取方向正确，必须保持默认 `legacyExperiments.enable*` 全部注释、`RuntimeFrameRunner.cpp` 不再依赖 `RuntimeScenePreparer.h`、新增文件已注册到工程。
- `Darwin` 判断该抽取不应影响 `SceneSetupPipeline`、Engine World prepared scene、`RuntimeFramePipeline` 或 verification modes；唯一风险是文档需要把“下一步收敛 legacy / scene setup 职责”更新为已完成，本节已处理。

已完成验证：

- 静态检查确认 `RuntimeLegacyExperimentLifecycle`、`makeContext(...)`、`prepare(...)`、`update(...)`、工程注册和旧 scene preparer 调用迁移均可检索。
- 静态检查确认 `RuntimeFrameRunner.cpp`、`RuntimeScenePreparer.h` 和 `RuntimeScenePreparer.cpp` 中不再出现 `RuntimeScenePreparer::updateLegacyExperiments`、`updateLegacyExperiments`、`prepareLegacyExperiments` 或 `makeLegacyExperimentContext` 残留。
- 静态检查确认 `RuntimeLegacyExperimentLifecycle.cpp` 中没有未注释的 `legacyExperiments.enable*` 调用，默认 legacy experiment disabled 行为保持不变。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeContentSceneLifecycle.cpp`、`RuntimeFrameRunner.cpp`、`RuntimeLegacyExperimentLifecycle.cpp` 与 `RuntimeScenePreparer.cpp` 参与编译，四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeScenePreparer` 的职责进一步收窄为 scene setup pipeline、prepared-scene stats 输出和 scene setup 完成后的 lifecycle 委托。
- `RuntimeFrameRunner` 不再通过 scene preparer 更新 legacy experiments，frame runner 与 startup scene preparation 的耦合下降。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变默认 legacy experiment disabled 行为。
- Scene setup pipeline config/header 依赖已在下一节收敛；后续可继续推进 renderer backend attachment 更靠近 Engine-level contract，或继续收窄 scene setup result/report 边界。

### 2026-05-30 Scene Setup Pipeline Config Header Extraction

本轮继续收敛 scene setup public header 边界。上一轮之后 `RuntimeScenePrepareConfig.h` 仍直接包含完整 `SceneSetupPipeline.h`，这会把 `SceneSetupPipelineResult`、`SetupContext`、World-driven scene setup 与 LegacySceneWorldBuilder 相关实现依赖传播给 application runtime config 调用方。本切片把 `SceneSetupPipelineConfig` 拆成独立轻量 header，让 runtime scene prepare config 只依赖配置 DTO。

新增与修改：

- 新增 `tools/sceneSetup/SceneSetupPipelineConfig.h`，集中定义 `GL_SCENE::SceneSetupPipelineConfig`。
- `tools/sceneSetup/SceneSetupPipeline.h` 改为包含 `SceneSetupPipelineConfig.h`，自身继续负责 `SceneSetupPipelineResult`、`prepareScene(...)` 和 stats format API。
- `application/RuntimeScenePrepareConfig.h` 改为只包含 `SceneSetupPipelineConfig.h`，不再 public include 完整 `SceneSetupPipeline.h`。
- `RuntimeScenePreparer.cpp` 仍包含完整 `SceneSetupPipeline.h`，因为该 `.cpp` 需要调用 `prepareScene(...)` 并读取 `SceneSetupPipelineResult`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 config header。

已完成验证：

- 静态检查确认 `SceneSetupPipelineConfig`、新 config header、pipeline API、runtime config include 和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeScenePrepareConfig.h` 不再包含完整 `SceneSetupPipeline.h`，`SceneSetupPipelineConfig` 的 struct 定义只位于新 header。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationStartupLifecycle.cpp`、`RuntimeContentConfigPolicy.cpp`、`RuntimeContentLifecycle.cpp`、`RuntimeContentSceneLifecycle.cpp`、`RuntimeFrameRunner.cpp`、`RuntimeLegacyExperimentLifecycle.cpp`、`RuntimeScenePreparer.cpp` 与 `SceneSetupPipeline.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- runtime scene prepare config 现在只暴露 scene setup pipeline 配置 DTO，不再传播完整 scene setup pipeline API。
- scene setup pipeline 行为、Engine World prepared scene 输出、legacy default scene mirror 和 renderer backend verification 均保持不变。
- 该切片仍是降耦合，不扩张 PBR 功能。
- Scene setup report 边界已在下一节收敛；后续可继续回到 renderer backend Engine-level attachment contract，或继续清理 runtime/application composition。

### 2026-05-30 Runtime Scene Setup Report Extraction

本轮继续收窄 `RuntimeScenePreparer` 的职责，把 scene setup result 的 stdout/logger 输出和最终 renderer prepared 日志迁入独立 report 模块。目标是让 `RuntimeScenePreparer` 只负责编排 scene setup pipeline 与后续 lifecycle 委托，不再直接持有 logging/report formatting 细节。

新增与修改：

- 新增 `application/RuntimeSceneSetupReport.h/.cpp`。
- `RuntimeSceneSetupReport::reportPreparedScene(...)` 集中处理 legacy mirror、Engine World minimal scene 和 Engine World scene probe 的 setup stats 输出。
- `RuntimeSceneSetupReport::reportRendererPrepared()` 集中保留原有 renderer prepared `LogInfo(...)` 行，日志内容不变。
- `RuntimeScenePreparer.cpp` 不再包含 `<iostream>` 或 `LogManager.h`，也不再直接调用 scene setup stats formatter；scene setup 完成后只委托 report 模块输出结果。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 report 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeSceneSetupReport`、`reportPreparedScene(...)`、`reportRendererPrepared()`、scene setup format API 和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeScenePreparer.cpp` 中不再出现 `<iostream>`、`LogManager`、`std::cout`、`LogInfo` 或 scene setup stats formatter 直接调用。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeScenePreparer.cpp` 与 `RuntimeSceneSetupReport.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeScenePreparer` 现在只保留 scene setup context 构造、pipeline 调用、legacy experiment lifecycle 委托和 report 委托。
- scene setup stats 输出的 formatting/logging 细节集中到 report 模块，后续如果要替换 logger、拆 stdout 或改 verification 输出入口，不需要再改 preparer 编排逻辑。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 verification 输出契约。
- Scene setup context assembly 已在下一节拆出；后续可继续回到 renderer backend Engine-level attachment contract，或继续清理 runtime/application composition。

### 2026-05-31 Runtime Scene Setup Context Factory Extraction

本轮继续收窄 `RuntimeScenePreparer`，把 `AppRuntimeContext` 到 `GL_SCENE::SetupContext` 的字段展开迁入独立 factory。目标是让 scene preparer 不再公开 context factory API，也不再直接知道 runtime context 中每个 renderer/scene/light/profile 字段如何映射到 scene setup pipeline。

新增与修改：

- 新增 `application/RuntimeSceneSetupContextFactory.h/.cpp`。
- `RuntimeSceneSetupContextFactory::make(...)` 集中构造 `GL_SCENE::SetupContext`，保留原有字段映射和 Engine World 指针/可编辑状态引用。
- `RuntimeScenePreparer.h` 删除公开的 `makeSceneSetupContext(...)` API，不再 forward declare `GL_SCENE::SetupContext`。
- `RuntimeScenePreparer.cpp` 删除 context 字段展开、`AppRuntimeContext.h` 和 `Engine.h` 直接 include，只委托 `RuntimeSceneSetupContextFactory::make(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 context factory 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeSceneSetupContextFactory`、`make(...)`、runtime context 字段映射和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeScenePreparer.cpp/.h` 中不再出现 `makeSceneSetupContext`、`GL_SCENE::SetupContext`、`AppRuntimeContext.h` / `Engine.h` include 或 `context.` 字段访问残留。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeContentSceneLifecycle.cpp`、`RuntimeScenePreparer.cpp` 与 `RuntimeSceneSetupContextFactory.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeScenePreparer` 现在只负责 scene preparation sequence：scene setup pipeline lifecycle、legacy experiment lifecycle prepare 和 renderer prepared report。
- runtime context 字段映射被集中到 factory，后续如果 `AppRuntimeContext` 继续瘦身或 scene setup pipeline 改用更中性的 input DTO，只需要替换 factory 入口。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 scene setup pipeline 行为或 verification 输出契约。
- Scene setup pipeline execution 已在下一节拆出；后续可继续清理 runtime/application composition，或回到 renderer backend Engine-level attachment contract。

### 2026-05-31 Runtime Scene Setup Pipeline Lifecycle Extraction

本轮继续收窄 `RuntimeScenePreparer`，把 scene setup pipeline 执行、setup context factory 调用和 prepared-scene setup report 输出合并进独立 lifecycle。目标是让 `RuntimeScenePreparer` 不再直接包含完整 `SceneSetupPipeline.h`，也不再直接调用 `GL_SCENE::prepareScene(...)` 或读取 `SceneSetupPipelineResult`。

新增与修改：

- 新增 `application/RuntimeSceneSetupPipelineLifecycle.h/.cpp`。
- `RuntimeSceneSetupPipelineLifecycle::prepare(...)` 集中调用 `RuntimeSceneSetupContextFactory::make(...)`、`GL_SCENE::prepareScene(...)` 和 `RuntimeSceneSetupReport::reportPreparedScene(...)`。
- `RuntimeScenePreparer.cpp` 不再包含 `RuntimeSceneSetupContextFactory.h` 或完整 `SceneSetupPipeline.h`，只委托 `RuntimeSceneSetupPipelineLifecycle::prepare(...)`。
- `RuntimeScenePreparer.cpp` 现在只保留高层顺序：scene setup pipeline lifecycle、legacy experiment lifecycle prepare、renderer prepared report。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 pipeline lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeSceneSetupPipelineLifecycle`、context factory、`prepareScene(...)`、`SceneSetupPipelineResult`、prepared-scene setup report 和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeScenePreparer.cpp/.h` 中不再出现完整 `SceneSetupPipeline.h` include、`GL_SCENE::prepareScene(...)`、`SceneSetupPipelineResult`、`RuntimeSceneSetupContextFactory` 或 `reportPreparedScene(...)` 直接调用。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeScenePreparer.cpp` 与 `RuntimeSceneSetupPipelineLifecycle.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeScenePreparer` 当前只负责 scene preparation 高层顺序，不再直接知道 scene setup pipeline execution 的具体 API。
- scene setup pipeline 执行边界现在可单独演进，后续如果要把 setup pipeline 从 application runtime 迁向 engine-level scene bootstrap，入口已经集中。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 scene setup pipeline 行为或 verification 输出契约。
- `RuntimeScenePreparer` 包装层已在下一节删除；后续可继续清理 runtime/application composition，或回到 renderer backend Engine-level attachment contract。

### 2026-05-31 Runtime Scene Preparer Removal

本轮继续清理 runtime/application composition 中已经失去实际职责的包装层。上一轮之后 `RuntimeScenePreparer` 只剩三行高层顺序委托：scene setup pipeline lifecycle、legacy experiment lifecycle prepare 和 renderer prepared report。这个类不再持有独立状态、独立策略或独立资源所有权，因此继续保留会让调用链多一层无意义 facade。

新增与修改：

- 删除 `application/RuntimeScenePreparer.h/.cpp`。
- `RuntimeContentSceneLifecycle::prepareScene(...)` 直接按原顺序调用 `RuntimeSceneSetupPipelineLifecycle::prepare(...)`、`RuntimeLegacyExperimentLifecycle::prepare(...)` 和 `RuntimeSceneSetupReport::reportRendererPrepared()`。
- `RuntimeContentSceneLifecycle.cpp` 现在成为 content-side scene preparation sequence 的直接入口。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已移除 `RuntimeScenePreparer` 源文件和头文件注册。

已完成验证：

- 静态检查确认 live source/project 中不再存在 `RuntimeScenePreparer.h`、`RuntimeScenePreparer.cpp`、`RuntimeScenePreparer::` 或 `class RuntimeScenePreparer` 残留。
- 静态检查确认 `RuntimeContentSceneLifecycle.cpp` 直接引用 `RuntimeSceneSetupPipelineLifecycle`、`RuntimeLegacyExperimentLifecycle` 和 `RuntimeSceneSetupReport`。
- `Test-Path application\RuntimeScenePreparer.cpp` 与 `Test-Path application\RuntimeScenePreparer.h` 均返回 `False`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- content scene lifecycle 现在直接拥有 scene preparation sequence，不再通过一个无状态 wrapper 间接调用。
- scene setup pipeline、legacy experiment prepare、renderer prepared report 的执行顺序保持不变。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 verification 输出契约。
- Application startup 的 content startup bridge 已在下一节拆出；后续可以继续沿 content/startup composition 收敛无状态 facade，或回到 renderer backend Engine-level attachment contract。

### 2026-05-31 Runtime Application Content Startup Lifecycle Extraction

本轮继续收敛 application startup/content composition。上一轮后 `RuntimeApplicationStartupLifecycle` 仍直接知道 content startup 的两个细节：如何从 `RuntimeApplicationShellConfig + RuntimeWindowSnapshot` 组装 `RuntimeContentLifecycleConfig`，以及如何调用 `RuntimeContentLifecycle::prepare(...)`。这会让 application startup lifecycle 同时承担 window/graphics/editor/frame 顺序和 content startup bridge 细节。本切片把 content startup bridge 迁入独立 module。

新增与修改：

- 新增 `application/RuntimeApplicationContentStartupLifecycle.h/.cpp`。
- `RuntimeApplicationContentStartupLifecycle::prepareContent(...)` 集中接收 `RuntimeApplicationState`、`RuntimeApplicationShellConfig` 和 `RuntimeWindowSnapshot`，再调用 `RuntimeContentConfigPolicy::makeContentLifecycleConfig(...)` 与 `RuntimeContentLifecycle::prepare(...)`。
- `RuntimeApplicationStartupLifecycle.cpp` 不再包含 `RuntimeContentConfigPolicy.h` 或 `RuntimeContentLifecycle.h`，content stage 改为委托 `RuntimeApplicationContentStartupLifecycle::prepareContent(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 content startup lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationContentStartupLifecycle`、`prepareContent(...)`、content config policy、content lifecycle prepare 和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationStartupLifecycle.cpp` 中不再直接包含 `RuntimeContentConfigPolicy.h` / `RuntimeContentLifecycle.h`，也不再直接调用 `RuntimeContentConfigPolicy::...` 或 `RuntimeContentLifecycle::prepare(...)`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationContentStartupLifecycle.cpp` 与 `RuntimeApplicationStartupLifecycle.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationStartupLifecycle` 继续负责 Engine/Window/Graphics/Content/Editor/Frame 的 startup 顺序，但不再直接知道 content config policy 与 content lifecycle prepare 细节。
- content startup bridge 现在可独立审查，后续如果 content config 继续拆分，application startup lifecycle 不需要继续膨胀。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 startup 顺序、scene preparation、renderer backend attachment 或 verification 输出。
- Application startup 的 editor startup bridge 已在下一节拆出；后续可以继续收敛 application startup/frame/shutdown bridge，或回到 renderer backend Engine-level attachment contract。

### 2026-05-31 Runtime Application Editor Startup Lifecycle Extraction

本轮继续收敛 application startup composition。上一轮之后 `RuntimeApplicationStartupLifecycle` 已经不再直接组装 content startup，但它仍直接知道 editor startup 的两件事：如何从 shell config 和 window native handle 构造 editor lifecycle config，以及如何调用 `RuntimeEditorLifecycle::initialize(...)`。本切片把 editor startup bridge 拆成独立 module，让 startup lifecycle 继续保留高层顺序，但不再直接依赖 editor lifecycle implementation。

新增与修改：

- 新增 `application/RuntimeApplicationEditorStartupLifecycle.h/.cpp`。
- `RuntimeApplicationEditorStartupLifecycle::initializeEditor(...)` 集中接收 `RuntimeApplicationState`、`RuntimeApplicationShellConfig` 和 `RuntimeWindowSnapshot`，再调用 `RuntimeApplicationConfigPolicy::makeEditorLifecycleConfig(...)` 与 `RuntimeEditorLifecycle::initialize(...)`。
- `RuntimeApplicationStartupLifecycle.cpp` 不再包含 `RuntimeEditorLifecycle.h`，editor stage 改为委托 `RuntimeApplicationEditorStartupLifecycle::initializeEditor(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 editor startup lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationEditorStartupLifecycle`、`initializeEditor(...)`、editor lifecycle 初始化、editor config policy 和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationStartupLifecycle.cpp` 中不再出现 `RuntimeEditorLifecycle.h` include、`RuntimeEditorLifecycle::initialize(...)` 或 `makeEditorLifecycleConfig(...)` 直接调用。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationEditorStartupLifecycle.cpp` 与 `RuntimeApplicationStartupLifecycle.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationStartupLifecycle` 继续保留 Engine/Window/Graphics/Content/Editor/Frame reset 的高层顺序，但不再直接知道 editor lifecycle 初始化细节。
- editor startup bridge 现在可独立审查，后续如果 editor lifecycle config 继续拆分，startup lifecycle 不需要继续膨胀。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 startup 顺序、editor initialization、scene preparation 或 verification 输出。
- Application startup 的 frame reset bridge 已在下一节拆出；后续可以继续收敛 application frame bridge / shutdown bridge，或回到 renderer backend Engine-level attachment contract。

### 2026-05-31 Runtime Application Frame Startup Lifecycle Extraction

本轮继续收敛 application startup composition。上一轮之后 `RuntimeApplicationStartupLifecycle` 已经不再直接组装 content 或 editor startup，但它仍直接调用 `RuntimeFrameLifecycle::reset(state.frameLifecycle)`。这个调用虽然很小，但它让 startup lifecycle 继续直接依赖 frame lifecycle implementation。本切片把 frame reset bridge 拆成独立 module，让 startup lifecycle 只保留高层启动顺序。

新增与修改：

- 新增 `application/RuntimeApplicationFrameStartupLifecycle.h/.cpp`。
- `RuntimeApplicationFrameStartupLifecycle::resetFrameLifecycle(...)` 集中接收 `RuntimeApplicationState`，再调用 `RuntimeFrameLifecycle::reset(state.frameLifecycle)`。
- `RuntimeApplicationStartupLifecycle.cpp` 不再直接调用 `RuntimeFrameLifecycle::reset(...)`，frame reset stage 改为委托 `RuntimeApplicationFrameStartupLifecycle::resetFrameLifecycle(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 frame startup lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationFrameStartupLifecycle`、`resetFrameLifecycle(...)`、frame lifecycle reset 桥接和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationStartupLifecycle.cpp` 中不再出现 `RuntimeFrameLifecycle::reset(...)` 或 `reset(state.frameLifecycle)` 直接调用。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationFrameStartupLifecycle.cpp` 与 `RuntimeApplicationStartupLifecycle.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationStartupLifecycle` 继续保留 Engine/Window/Graphics/Content/Editor/Frame reset 的高层顺序，但不再直接知道 frame lifecycle reset implementation。
- frame startup bridge 现在可独立审查，后续如果 frame lifecycle startup/reset 继续拆分，startup lifecycle 不需要继续膨胀。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 startup 顺序、frame reset 行为、scene preparation 或 verification 输出。
- Application startup 的 engine startup bridge 已在下一节拆出；后续可以继续收敛 application frame bridge / shutdown bridge，或回到 renderer backend Engine-level attachment contract。

### 2026-05-31 Runtime Application Engine Startup Lifecycle Extraction

本轮继续收敛 application startup composition。上一轮之后 `RuntimeApplicationStartupLifecycle` 已经不再直接组装 content、editor 或 frame reset startup，但它仍直接调用 `RuntimeEngineLifecycle::initializeEngine(...)` 并从 application shell config 生成 Engine desc。本切片把 Engine startup bridge 拆成独立 module，让 startup lifecycle 继续保留高层启动顺序，但不再直接依赖 Engine lifecycle initialization implementation。

新增与修改：

- 新增 `application/RuntimeApplicationEngineStartupLifecycle.h/.cpp`。
- `RuntimeApplicationEngineStartupLifecycle::initializeEngine(...)` 集中接收 `RuntimeApplicationState` 与 `RuntimeApplicationShellConfig`，再调用 `RuntimeApplicationConfigPolicy::makeEngineDesc(...)` 和 `RuntimeEngineLifecycle::initializeEngine(...)`。
- `RuntimeApplicationStartupLifecycle.cpp` 不再直接调用 `RuntimeEngineLifecycle::initializeEngine(...)` 或 `RuntimeApplicationConfigPolicy::makeEngineDesc(...)`，Engine startup stage 改为委托 `RuntimeApplicationEngineStartupLifecycle::initializeEngine(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 engine startup lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationEngineStartupLifecycle`、`initializeEngine(...)`、Engine lifecycle initialization 桥接和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationStartupLifecycle.cpp` 中不再出现 `RuntimeEngineLifecycle::initializeEngine(...)` 或 `makeEngineDesc(...)` 直接调用。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationEngineStartupLifecycle.cpp` 与 `RuntimeApplicationStartupLifecycle.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationStartupLifecycle` 继续保留 Engine/Window/Graphics/Content/Editor/Frame reset 的高层顺序，但不再直接知道 Engine desc mapping 或 Engine lifecycle initialization implementation。
- Engine startup bridge 现在可独立审查，后续如果 Engine startup config 或 subsystem initialization 继续拆分，startup lifecycle 不需要继续膨胀。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 startup 顺序、Engine lifecycle 行为、scene preparation 或 verification 输出。
- Application startup 的 window startup bridge 已在下一节拆出；后续可以继续收敛 application graphics startup bridge、application frame bridge / shutdown bridge，或回到 renderer backend Engine-level attachment contract。

### 2026-05-31 Runtime Application Window Startup Lifecycle Extraction

本轮继续收敛 application startup composition。上一轮之后 `RuntimeApplicationStartupLifecycle` 已经不再直接组装 Engine startup，但它仍直接调用 window setup prompt、`RuntimeWindowLifecycle::initialize(...)` 和 `RuntimeWindowLifecycle::captureSnapshot()`。本切片把 window startup bridge 拆成独立 module，让 startup lifecycle 继续保留高层启动顺序，但不再直接依赖 window lifecycle initialization 或 snapshot implementation。

新增与修改：

- 新增 `application/RuntimeApplicationWindowStartupLifecycle.h/.cpp`。
- `RuntimeApplicationWindowStartupLifecycle::initializeWindow(...)` 集中接收 `RuntimeApplicationState` 与 `RuntimeApplicationShellConfig`，再调用 `RuntimeGraphicsLifecycle::reportWindowSetupPrompt()` 与 `RuntimeWindowLifecycle::initialize(...)`。
- `RuntimeApplicationWindowStartupLifecycle::captureWindowSnapshot()` 集中调用 `RuntimeWindowLifecycle::captureSnapshot()`。
- `RuntimeApplicationStartupLifecycle.cpp` 不再直接调用 `RuntimeWindowLifecycle::initialize(...)`、`RuntimeWindowLifecycle::captureSnapshot()` 或 `RuntimeGraphicsLifecycle::reportWindowSetupPrompt()`，window startup stage 和 snapshot stage 改为委托 `RuntimeApplicationWindowStartupLifecycle`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 window startup lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationWindowStartupLifecycle`、`initializeWindow(...)`、`captureWindowSnapshot()`、window lifecycle bridge 和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationStartupLifecycle.cpp` 中不再出现 `RuntimeWindowLifecycle::initialize(...)`、`RuntimeWindowLifecycle::captureSnapshot()`、`RuntimeGraphicsLifecycle::reportWindowSetupPrompt()` 或 `RuntimeWindowLifecycle.h` 直接 include。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationWindowStartupLifecycle.cpp` 与 `RuntimeApplicationStartupLifecycle.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationStartupLifecycle` 继续保留 Engine/Window/Graphics/Content/Editor/Frame reset 的高层顺序，但不再直接知道 window initialization、window callback context 或 window snapshot implementation。
- window startup bridge 现在可独立审查，后续如果 window config、callback binding 或 snapshot 继续拆分，startup lifecycle 不需要继续膨胀。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 startup 顺序、window lifecycle 行为、scene preparation 或 verification 输出。
- Application startup 的 graphics startup bridge 已在下一节拆出；后续可以继续收敛 application frame bridge / shutdown bridge，或回到 renderer backend Engine-level attachment contract。

### 2026-05-31 Runtime Application Graphics Startup Lifecycle Extraction

本轮继续收敛 application startup composition。上一轮之后 `RuntimeApplicationStartupLifecycle` 已经不再直接组装 Engine 或 Window startup，但它仍直接调用 `RuntimeGraphicsLifecycle::initializeAfterWindow(...)` 并从 application shell config 生成 graphics lifecycle config。本切片把 graphics startup bridge 拆成独立 module，让 startup lifecycle 继续保留高层启动顺序，但不再直接依赖 graphics lifecycle implementation 或 graphics config mapping。

新增与修改：

- 新增 `application/RuntimeApplicationGraphicsStartupLifecycle.h/.cpp`。
- `RuntimeApplicationGraphicsStartupLifecycle::initializeGraphics(...)` 集中接收 `RuntimeApplicationShellConfig`，再调用 `RuntimeApplicationConfigPolicy::makeGraphicsLifecycleConfig(...)` 和 `RuntimeGraphicsLifecycle::initializeAfterWindow(...)`。
- `RuntimeApplicationStartupLifecycle.cpp` 不再直接调用 `RuntimeGraphicsLifecycle::initializeAfterWindow(...)` 或 `RuntimeApplicationConfigPolicy::makeGraphicsLifecycleConfig(...)`，graphics startup stage 改为委托 `RuntimeApplicationGraphicsStartupLifecycle::initializeGraphics(...)`。
- `RuntimeApplicationStartupLifecycle.cpp` 同时移除不再需要的 `RuntimeApplicationConfigPolicy.h` 和 `RuntimeGraphicsLifecycle.h` 直接 include。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 graphics startup lifecycle 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationGraphicsStartupLifecycle`、`initializeGraphics(...)`、graphics lifecycle initialization 桥接和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationStartupLifecycle.cpp` 中不再出现 `RuntimeGraphicsLifecycle::initializeAfterWindow(...)`、`makeGraphicsLifecycleConfig(...)`、`RuntimeGraphicsLifecycle.h` 或 `RuntimeApplicationConfigPolicy.h` 直接依赖。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-scene-probe,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationGraphicsStartupLifecycle.cpp` 与 `RuntimeApplicationStartupLifecycle.cpp` 参与编译，五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationStartupLifecycle` 继续保留 Engine/Window/Graphics/Window snapshot/Content/Editor/Frame reset 的高层顺序，但不再直接知道 graphics lifecycle config mapping 或 graphics initialization implementation。
- graphics startup bridge 现在可独立审查，后续如果 graphics startup config、capability report 或 viewport initialization 继续拆分，startup lifecycle 不需要继续膨胀。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 startup 顺序、graphics lifecycle 行为、scene preparation 或 verification 输出。
- Application frame editor callback bridge 已在下一节拆出；后续可以继续收敛 application frame continue/run bridge 或 shutdown bridge，或回到 renderer backend Engine-level attachment contract。

### 2026-05-31 Runtime Application Frame Editor Callback Bridge Extraction

本轮按 `/subagents` 要求先让只读 sidecar `Lorentz` 审查后续候选。结论是 frame editor callback bridge 是当前最小且风险最低的切片：不触碰 shutdown cleanup 顺序、不扩张 renderer backend contract、不继续扩大 PBR pass。父 agent 采纳该建议，把 `RuntimeApplicationFrameLifecycle::runFrame(...)` 中 editor callback config mapping 与 callback creation 拆出。

新增与修改：

- 新增 `application/RuntimeApplicationFrameEditorCallbackBridge.h/.cpp`。
- `RuntimeApplicationFrameEditorCallbackBridge::makeFrameCallbacks(...)` 集中接收 `RuntimeApplicationState`、`RuntimeApplicationShellConfig` 和 `GLFWwindow*`，再调用 `RuntimeApplicationConfigPolicy::makeEditorLifecycleConfig(...)` 与 `RuntimeEditorLifecycle::makeFrameCallbacks(...)`。
- `RuntimeApplicationFrameLifecycle.cpp` 不再直接调用 `RuntimeEditorLifecycle::makeFrameCallbacks(...)` 或 `RuntimeApplicationConfigPolicy::makeEditorLifecycleConfig(...)`；frame loop 只向 bridge 请求 editor frame callbacks。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 frame editor callback bridge 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationFrameEditorCallbackBridge`、`makeFrameCallbacks(...)`、editor callback bridge 和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationFrameLifecycle.cpp` 中不再出现 `RuntimeEditorLifecycle::makeFrameCallbacks(...)` 或 `makeEditorLifecycleConfig(...)` 直接调用。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationFrameEditorCallbackBridge.cpp` 与 `RuntimeApplicationFrameLifecycle.cpp` 参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationFrameLifecycle` 继续保留 should-continue、window snapshot、frame config 和 frame execution 编排，但不再直接知道 editor lifecycle callback creation 的细节。
- 该切片保持 `RuntimeEditorLifecycle::makeFrameCallbacks(...)` 中按值捕获 config 的行为不变，只移动调用边界。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 frame loop、renderer backend、window snapshot 或 shutdown cleanup 顺序。
- Application frame continue/run bridge 已在下一节拆出；后续可以考虑 shutdown bridge 或 renderer backend contract continuation。

### 2026-05-31 Runtime Application Frame Continue/Run Bridge Extraction

本轮继续收敛 application frame composition。上一轮之后 editor callback creation 已从 `RuntimeApplicationFrameLifecycle` 中拆出，但该 facade 仍直接知道 frame continue config mapping、window snapshot、frame execution 参数展开和 `RuntimeFrameLifecycle` 调用。本切片把 continue 与 run 两条路径拆成两个小 bridge，让 `RuntimeApplicationFrameLifecycle` 只保留高层转发。

新增与修改：

- 新增 `application/RuntimeApplicationFrameContinueBridge.h/.cpp`。
- `RuntimeApplicationFrameContinueBridge::shouldContinue(...)` 集中接收 `RuntimeApplicationShellConfig` 与 `RuntimeApplicationState`，再调用 `RuntimeApplicationConfigPolicy::makeFrameLifecycleConfig(...)` 和 `RuntimeFrameLifecycle::shouldContinue(...)`。
- 新增 `application/RuntimeApplicationFrameRunBridge.h/.cpp`。
- `RuntimeApplicationFrameRunBridge::runFrame(...)` 集中接收 `RuntimeApplicationState` 与 `RuntimeApplicationShellConfig`，负责 window snapshot、frame lifecycle config、frame execution 参数展开和 editor callback bridge 串接。
- `RuntimeApplicationFrameLifecycle.cpp` 不再直接调用 `RuntimeFrameLifecycle::shouldContinue(...)`、`RuntimeFrameLifecycle::runFrame(...)`、`RuntimeWindowLifecycle::captureSnapshot()`、`RuntimeApplicationConfigPolicy::makeFrameLifecycleConfig(...)` 或 `RuntimeApplicationFrameEditorCallbackBridge::makeFrameCallbacks(...)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 continue/run bridge 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationFrameContinueBridge`、`RuntimeApplicationFrameRunBridge`、continue/run 桥接和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationFrameLifecycle.cpp` 中不再出现 frame lifecycle、window snapshot、frame config policy 或 editor callback bridge 的直接调用/直接 include。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationFrameContinueBridge.cpp`、`RuntimeApplicationFrameRunBridge.cpp` 与 `RuntimeApplicationFrameLifecycle.cpp` 参与编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationFrameLifecycle` 当前只保留 `shouldContinue(...)` 和 `runFrame(...)` facade 入口，不再直接知道 frame continue/run 的底层实现细节。
- continue/run bridge 仍保持原 frame loop 行为：max-frame/GL_APP update gating、window framebuffer size、Engine-owned renderer subsystem、legacy experiment update、editor UI callback gating 和 verification capture 顺序不变。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 renderer backend ownership 或 shutdown cleanup 顺序。
- Application shutdown cleanup/destroy bridge 已在下一节拆出；后续优先考虑更细 shutdown phase 拆分，或回到 renderer backend contract continuation。

### 2026-05-31 Runtime Application Shutdown Cleanup/Destroy Bridge Extraction

本轮继续收敛 application shutdown composition。`RuntimeApplicationShutdownLifecycle` 已经接管 cleanup/destroy orchestration，但它仍直接知道 renderer cleanup report、camera cleanup、runtime context detach、Engine shutdown、engine cleanup report 和 window destroy。该区域顺序敏感，因此本切片不重排步骤，只把原顺序整体迁入窄 cleanup bridge，并把 window destroy 单独迁入 destroy bridge。

新增与修改：

- 新增 `application/RuntimeApplicationShutdownCleanupBridge.h/.cpp`。
- `RuntimeApplicationShutdownCleanupBridge::cleanup(...)` 保留原 cleanup 顺序：`RuntimeEngineLifecycle::beginCleanup(...)`、renderer subsystem cleanup report、camera cleanup、runtime context detach、Engine shutdown、Engine cleanup report。
- 新增 `application/RuntimeApplicationShutdownDestroyBridge.h/.cpp`。
- `RuntimeApplicationShutdownDestroyBridge::destroy()` 集中委托 `RuntimeWindowLifecycle::destroy()`。
- `RuntimeApplicationShutdownLifecycle.cpp` 不再直接调用 `RuntimeEngineLifecycle::beginCleanup(...)`、`RuntimeVerificationLifecycle::reportRendererSubsystemCleanup(...)`、`RuntimeCameraLifecycle::cleanup(...)`、`RuntimeEngineLifecycle::detachRuntimeContext(...)`、`RuntimeEngineLifecycle::shutdownEngine(...)`、`RuntimeVerificationLifecycle::reportEngineCleanup(...)` 或 `RuntimeWindowLifecycle::destroy()`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 shutdown cleanup/destroy bridge 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationShutdownCleanupBridge`、`RuntimeApplicationShutdownDestroyBridge`、cleanup/destroy 细节迁移和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationShutdownLifecycle.cpp` 中不再出现 cleanup/destroy 细节直接调用或相关实现 header 直接 include。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationShutdownCleanupBridge.cpp`、`RuntimeApplicationShutdownDestroyBridge.cpp` 与 `RuntimeApplicationShutdownLifecycle.cpp` 参与编译，三条 focused verification mode 全部通过，renderer/world/subsystem cleanup stats 继续输出。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationShutdownLifecycle` 当前只保留 `cleanup(...)` 和 `destroy()` facade 入口，不再直接知道 shutdown cleanup/destroy 的底层实现细节。
- shutdown cleanup 的观察窗口保持不变：先 detach renderer backend 并报告 renderer subsystem cleanup，再清理 camera 和 runtime context raw pointers，随后 shutdown Engine 并报告 Engine cleanup。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 renderer backend ownership、Engine shutdown 或 window destroy 行为。
- Runtime Application Shutdown Verification Bridge、Runtime Application Shutdown Engine Bridge、Runtime Engine Lifecycle Types Header、Runtime Application State Forward Boundary、Runtime Profile State extraction、Runtime Render Resource State extraction、Runtime Camera Light State extraction、Runtime Engine Attachment State extraction 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；下一步可以继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-31 Runtime Application Shutdown Verification Bridge Extraction

本轮继续收敛更细 shutdown phase bridge。上一轮 `RuntimeApplicationShutdownCleanupBridge` 已经持有 shutdown cleanup 顺序，但它仍直接知道 verification cleanup report 的两个调用点：renderer subsystem cleanup report 与 Engine cleanup report。本切片把这两个 verification report 委托迁入独立 bridge，让 cleanup bridge 继续只表达 shutdown 顺序，而不直接依赖 `RuntimeVerificationLifecycle`。

新增与修改：

- 新增 `application/RuntimeApplicationShutdownVerificationBridge.h/.cpp`。
- `RuntimeApplicationShutdownVerificationBridge::reportRendererSubsystemCleanup(...)` 集中委托 `RuntimeVerificationLifecycle::reportRendererSubsystemCleanup(...)`。
- `RuntimeApplicationShutdownVerificationBridge::reportEngineCleanup(...)` 集中委托 `RuntimeVerificationLifecycle::reportEngineCleanup(...)`。
- `RuntimeApplicationShutdownCleanupBridge.cpp` 不再直接 include 或调用 `RuntimeVerificationLifecycle`；cleanup 顺序保持为 begin cleanup、renderer cleanup report、camera cleanup、runtime context detach、Engine shutdown、Engine cleanup report。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 shutdown verification bridge 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationShutdownVerificationBridge`、renderer/Engine cleanup report 委托和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationShutdownCleanupBridge.cpp` 中不再出现 `RuntimeVerificationLifecycle` 或 `RuntimeVerificationLifecycle.h` 直接依赖。
- 静态检查确认 cleanup 顺序仍覆盖 begin cleanup、renderer cleanup report、camera cleanup、runtime context detach、Engine shutdown、Engine cleanup report。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationShutdownCleanupBridge.cpp` 与 `RuntimeApplicationShutdownVerificationBridge.cpp` 参与编译，三条 focused verification mode 全部通过，renderer/world/subsystem cleanup stats 继续输出。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- shutdown verification report 现在有独立 application bridge，后续如果 verification cleanup report 继续拆分，不需要再修改 shutdown cleanup 顺序 bridge。
- `RuntimeApplicationShutdownCleanupBridge` 仍负责高层 cleanup 顺序，但不再直接知道 `RuntimeVerificationLifecycle`。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 renderer backend ownership、Engine shutdown、camera cleanup、runtime context detach 或 window destroy 行为。
- Runtime Application Shutdown Engine Bridge、Runtime Engine Lifecycle Types Header、Runtime Application State Forward Boundary、Runtime Profile State extraction、Runtime Render Resource State extraction、Runtime Camera Light State extraction、Runtime Engine Attachment State extraction 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；下一步可以继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-31 Runtime Application Shutdown Engine Bridge Extraction

本轮继续收敛 camera cleanup / runtime context detach / Engine shutdown phase。上一轮后 `RuntimeApplicationShutdownCleanupBridge` 已不再直接依赖 verification lifecycle，但仍直接调用 `RuntimeEngineLifecycle::beginCleanup(...)`、`RuntimeCameraLifecycle::cleanup(...)`、`RuntimeEngineLifecycle::detachRuntimeContext(...)` 和 `RuntimeEngineLifecycle::shutdownEngine(...)`。本切片把这些 engine/camera cleanup 细节迁入独立 bridge，让 cleanup bridge 只表达高层 shutdown 顺序。

新增与修改：

- 新增 `application/RuntimeApplicationShutdownEngineBridge.h/.cpp`。
- `RuntimeApplicationShutdownEngineBridge::beginCleanup(...)` 集中委托 `RuntimeEngineLifecycle::beginCleanup(...)`，并返回 application-level `RuntimeApplicationShutdownCleanupRefs`。
- `RuntimeApplicationShutdownEngineBridge::cleanupRuntimeContext(...)` 集中执行 camera cleanup 与 runtime context detach。
- `RuntimeApplicationShutdownEngineBridge::shutdownEngine(...)` 集中委托 Engine shutdown。
- `RuntimeApplicationShutdownCleanupBridge.cpp` 不再直接 include 或调用 `RuntimeCameraLifecycle` / `RuntimeEngineLifecycle`；高层顺序保持为 begin cleanup、renderer cleanup report、runtime context cleanup、Engine shutdown、Engine cleanup report。
- `RuntimeApplicationShutdownVerificationBridge` 改为接收 `RuntimeApplicationShutdownCleanupRefs`，不再暴露 `RuntimeEngineLifecycleCleanupRefs` 到 application shutdown verification API。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 shutdown engine bridge 源文件和头文件。

已完成验证：

- 静态检查确认 `RuntimeApplicationShutdownEngineBridge`、`RuntimeApplicationShutdownCleanupRefs`、camera/runtime/engine cleanup 委托和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeApplicationShutdownCleanupBridge.cpp` 与 `RuntimeApplicationShutdownVerificationBridge.*` 中不再出现 `RuntimeCameraLifecycle`、`RuntimeEngineLifecycle` 或 `RuntimeEngineLifecycleCleanupRefs` 直接依赖。
- 静态检查确认 cleanup bridge 高层顺序仍覆盖 begin cleanup、renderer cleanup report、runtime context cleanup、Engine shutdown、Engine cleanup report。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；`RuntimeApplicationShutdownCleanupBridge.cpp`、`RuntimeApplicationShutdownEngineBridge.cpp` 与 `RuntimeApplicationShutdownVerificationBridge.cpp` 参与编译，三条 focused verification mode 全部通过，renderer/world/subsystem cleanup stats 继续输出。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationShutdownCleanupBridge` 当前只保留 shutdown cleanup 的高层 phase 顺序，不再直接知道 camera cleanup、runtime context detach 或 Engine lifecycle cleanup/shutdown 实现细节。
- `RuntimeApplicationShutdownCleanupRefs` 把 shutdown report 所需 refs 保持在 application shutdown bridge 边界，避免 verification bridge 直接暴露 engine lifecycle cleanup DTO。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 renderer backend ownership、cleanup report 输出、camera cleanup、runtime context detach、Engine shutdown 或 window destroy 行为。
- Runtime Engine Lifecycle Types Header、Runtime Application State Forward Boundary、Runtime Profile State extraction、Runtime Render Resource State extraction、Runtime Camera Light State extraction、Runtime Engine Attachment State extraction 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；后续可以继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-31 Runtime Engine Lifecycle Types Header Extraction

本轮继续细化 shutdown refs / Engine lifecycle cleanup DTO 边界。上一轮后 application shutdown bridge 已不再直接依赖 engine lifecycle implementation，但 `RuntimeEngineLifecycle.h` 仍把 `RuntimeEngineLifecycleState` 与 `RuntimeEngineLifecycleCleanupRefs` 和完整 `AppRuntimeContext.h`、`Engine.h`、`AssetSubsystem.h`、`RendererSubsystem.h` 一起暴露给所有调用方。本切片把 lifecycle state / cleanup refs 拆到轻量 types header，让持有 state 的 application aggregate 不再需要包含完整 lifecycle API header。

新增与修改：

- 新增 `application/RuntimeEngineLifecycleTypes.h`。
- `RuntimeEngineLifecycleTypes.h` 集中 `RuntimeEngineLifecycleState` 与 `RuntimeEngineLifecycleCleanupRefs`，只前置声明 `GLengine::AssetSubsystem` 与 `GLengine::RendererSubsystem`。
- `RuntimeEngineLifecycle.h` 改为只包含 `RuntimeEngineLifecycleTypes.h`，并前置声明 `GLframework::AppRuntimeContext`、`GLengine::Engine` 与 `GLengine::EngineDesc`。
- `RuntimeEngineLifecycle.cpp` 显式包含 implementation 所需的 `AppRuntimeContext.h`、`Engine.h`、`AssetSubsystem.h` 与 `RendererSubsystem.h`，把完整类型依赖局部化到 `.cpp`。
- `RuntimeApplicationState.h` 改为包含 `RuntimeEngineLifecycleTypes.h`，避免为了 `RuntimeEngineLifecycleState` 持有完整 lifecycle API header。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 engine lifecycle types header。

已完成验证：

- 静态检查确认 `RuntimeEngineLifecycleTypes`、`RuntimeEngineLifecycleState`、`RuntimeEngineLifecycleCleanupRefs`、implementation 侧完整类型依赖和 VS 工程注册均可检索。
- 静态检查确认 `RuntimeEngineLifecycle.h` 中不再出现 `AppRuntimeContext.h`、`Engine.h`、`AssetSubsystem.h` 或 `RendererSubsystem.h` public include。
- 静态检查确认 `RuntimeApplicationState.h` 已改为包含 `RuntimeEngineLifecycleTypes.h`；该 header 仍因持有完整 `Engine` 与 `AppRuntimeContext` 保留对应 include。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；application startup/frame/shutdown/content/engine lifecycle 相关文件重新编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- Engine lifecycle state/cleanup refs 现在有独立轻量 header，调用方可以只依赖 DTO，不必引入完整 lifecycle API 或 Engine/subsystem implementation headers。
- `RuntimeEngineLifecycle.h` 只保留 lifecycle API surface，不再把 implementation 所需完整类型作为 public include 扩散出去。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 Engine startup、renderer backend attach、cleanup refs、context detach、Engine shutdown 或 verification cleanup 行为。
- Runtime Application State Forward Boundary、Runtime Profile State extraction、Runtime Render Resource State extraction、Runtime Camera Light State extraction、Runtime Engine Attachment State extraction 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；后续可以继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-31 Runtime Application State Forward Boundary

本轮继续收敛 `RuntimeApplicationState / AppRuntimeContext public owner boundary` 的第一层：不改变 `RuntimeApplicationState` 当前持有的 Engine、runtime context、editor/frame state 和 legacy experiment runner，也不改 startup/frame/shutdown 顺序；只把 `RuntimeApplicationState.h` 从公共 application facade headers 中隔离出去，降低编译期耦合和公共 owner 泄漏。

新增与修改：

- `RuntimeApplicationShell.h` 不再 include `RuntimeApplicationState.h`，改为前置声明 `RuntimeApplicationState` 并通过 `std::unique_ptr<RuntimeApplicationState>` 持有 state。
- `RuntimeApplicationShell.cpp` 显式 include `RuntimeApplicationState.h`，负责构造/析构隐藏的 state，并在 `makeCallbacks()` 中把 `*mState` 传给 callback binder。
- `RuntimeApplicationShell` 显式删除 copy，保留 move 构造/赋值；析构和 move 实现在 `.cpp` 中定义，避免 incomplete type 在 public header 中要求完整 state。
- `RuntimeApplicationCallbackBinder.h`、content/editor/engine/window startup bridge headers、frame continue/run/editor callback bridge headers、frame startup/application frame/shutdown facade headers、shutdown cleanup bridge header 改为前置声明 `RuntimeApplicationState` 与 `RuntimeApplicationShellConfig`。
- 对应 `.cpp` 文件显式 include `RuntimeApplicationState.h` 与 `RuntimeApplicationConfig.h`，让完整类型依赖停留在 implementation files。

已完成验证：

- 静态检查确认上述 public headers 不再直接 include `RuntimeApplicationState.h`；除 `RuntimeApplicationShell.h` 因值持有 config 仍需要 `RuntimeApplicationConfig.h` 外，其余引用式 lifecycle/bridge headers 不再直接 include config header。
- 静态检查确认 `RuntimeApplicationShell.h/.cpp` 中 hidden state ownership、构造/析构/move 和 callback binder 转发均可检索。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；callback binder、startup/frame/shutdown lifecycle/bridge 和 shell 相关文件重新编译，三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- `RuntimeApplicationState` 仍是 application runtime state aggregate，但不再作为 Shell 和 application lifecycle public headers 的强制完整类型依赖向外扩散。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 Engine startup、window startup、content preparation、frame loop、shutdown cleanup 或 verification cleanup 行为。
- Runtime Profile State extraction、Runtime Render Resource State extraction、Runtime Camera Light State extraction、Runtime Engine Attachment State extraction 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；后续可以继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-31 Runtime Profile State Extraction

本轮继续推进 `AppRuntimeContext profile/data boundary`。上一轮后 `RuntimeApplicationState` 已不再从 Shell 和 lifecycle public headers 中扩散，但 `AppRuntimeContext.h` 本身仍直接持有并暴露 frame pipeline、post-process、environment、PBR preview/light/camera profile 与各配置路径。本切片先把这些 profile/path 数据聚合到独立 `RuntimeProfileState`，让 `AppRuntimeContext` 的 profile data 成为一个明确子边界。

新增与修改：

- 新增 `application/RuntimeProfileState.h`。
- `RuntimeProfileState` 集中 `RuntimeFramePipelineProfile`、frame pipeline profile path、renderer frame pass profile path、`PostProcessSettings`、post-process settings path、`EnvironmentProfile`、environment profile path、PBR preview/light/camera profiles、PBR preview profile path 与 PBR experiment profile path。
- `AppRuntimeContext.h` 删除对 frame pipeline profile、environment profile、post-process settings、renderer frame pass profile 和 PBR scene setup profile headers 的直接 include，改为包含 `RuntimeProfileState.h` 并持有 `GL_RUNTIME::RuntimeProfileState profiles`。
- Runtime profile loader、debug panel context mapping、runtime frame pass predicates/execution、renderer backend readiness/frame plan key、scene setup context creation 和 PBR verification startup/preview/light-camera policy 访问点改为 `context.profiles.*`。
- `RuntimeProfileLoader.h` 改为前置声明 `GLframework::AppRuntimeContext`，完整 context include 移到 `.cpp`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 runtime profile state header。

已完成验证：

- 静态检查确认 application 中旧的 `context.framePipelineProfile`、`context.environmentProfile`、`context.postProcessSettings`、`context.pbrPreviewProfile`、`context.pbrLightRigProfile`、`context.pbrCameraRigProfile` 和 profile path 直接访问已迁移到 `context.profiles.*`。
- 静态检查确认 `RuntimeProfileState.h` 是 profile headers 的集中包含点，`AppRuntimeContext.h` 只通过 `RuntimeProfileState.h` 持有 profile aggregate。
- 静态检查确认 `RuntimeProfileLoader.h` 不再直接 include `AppRuntimeContext.h`，loader implementation file owns the complete include。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。

结论：

- `AppRuntimeContext` 仍是运行时上下文聚合，但 profile/path 数据已形成独立子结构，后续继续拆 context 时不需要再把 profile 字段和 render resource / scene object / camera-light 字段混在一起处理。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 profile load order、frame pass enable policy、post-process settings、environment precompute、PBR preview/light/camera verification policy 或 renderer backend readiness 行为。
- Runtime Render Resource State extraction、Runtime Camera Light State extraction、Runtime Engine Attachment State extraction 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；后续可以继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-31 Runtime Render Resource State Extraction

本轮继续推进 `AppRuntimeContext render-resource/camera-light boundary` 的 render-resource 半段。上一轮后 profile/path 数据已经进入 `context.profiles`，但 `AppRuntimeContext` 仍直接暴露 renderer、scene、frame targets、bloom、screen/runtime mesh/material、post-process pass 和 clear color。本切片先把这些渲染资源聚合到独立 `RuntimeRenderResourceState`，camera/light 留作下一片。

新增与修改：

- 新增 `application/RuntimeRenderResourceState.h`。
- `RuntimeRenderResourceState` 集中 renderer、world/screen scene、point-light/screen/skybox/move-plane/text mesh、screen/grass/CSM shadow material、frame render targets、bloom、post-process pass 和 clear color。
- `AppRuntimeContext.h` 删除这些 render resource top-level 字段，改为持有 `GL_RUNTIME::RuntimeRenderResourceState renderResources`。
- application 内 runtime frame passes、frame runner、renderer backend readiness、window resize callback、scene setup context factory、legacy experiment context factory、editor panel coordinator、Engine lifecycle attachment 和 verification report/probe 访问点改为 `context.renderResources.*`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 runtime render resource state header。

已完成验证：

- 静态检查确认 application 中旧的 `context.renderer`、`context.sceneOffScreen`、`context.sceneInScreen`、`context.frameRenderTargets`、`context.bloom`、`context.screenQuad`、`context.screenMaterial`、`context.postProcessPass`、`context.clearColor` 等 top-level AppRuntimeContext render resource 访问已迁移到 `context.renderResources.*`；`RuntimeViewport` 中保留的是 resize DTO 字段名，不属于 AppRuntimeContext。
- 静态检查确认 `RuntimeRenderResourceState.h`、`AppRuntimeContext.h`、VS 工程注册和 `context.renderResources` 访问点均可检索。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。

结论：

- `AppRuntimeContext` 的渲染资源现在有独立子边界，后续拆 camera/light 时不需要和 renderer/scene/frame-target/post-process 资源混在一起处理。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 runtime frame pass 顺序、renderer backend ownership、window resize texture sync、scene setup mapping、legacy experiment hooks 或 verification output contract。
- Runtime Camera Light State extraction、Runtime Engine Attachment State extraction 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；后续可以继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-31 Runtime Camera Light State Extraction

本轮继续推进 `AppRuntimeContext camera-light boundary`。上一轮后 render resources 已经进入 `context.renderResources`，但 `AppRuntimeContext` 仍直接暴露 camera、camera control、ambient/directional/spot lights 和 point light 集合。本切片把这些 camera/light 状态聚合到独立 `RuntimeCameraLightState`，让后续继续处理 Engine/runtime attachment 指针时不再和 camera/light 混在一起。

新增与修改：

- 新增 `application/RuntimeCameraLightState.h`。
- `RuntimeCameraLightState` 集中 `Camera*`、`CameraControl*`、ambient light、directional light、spot light 和 point light vector。
- `AppRuntimeContext.h` 删除这些 camera/light top-level 字段，改为持有 `GL_RUNTIME::RuntimeCameraLightState cameraLights`。
- application 内 camera lifecycle、input/resize callback mapping、runtime frame pass scene-color 参数、scene setup context factory、legacy experiment context factory、editor panel coordinator、renderer backend readiness、PBR camera rig application 和 profile loader 访问点改为 `context.cameraLights.*`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 runtime camera light state header。

已完成验证：

- 静态检查确认 application 中旧的 top-level `context.camera`、`context.cameracontrol`、`context.ambientLight`、`context.dirLight`、`context.spotLight`、`context.pointLights` 访问已迁移到 `context.cameraLights.*`；`RuntimeInputController` 与 `RuntimeViewport` 中保留的是局部 DTO 字段名，不属于 AppRuntimeContext。
- 静态检查确认 `RuntimeCameraLightState.h`、`AppRuntimeContext.h`、VS 工程注册和 `context.cameraLights` 访问点均可检索。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。

结论：

- `AppRuntimeContext` 的 camera/light 状态现在有独立子边界，后续拆 Engine/runtime attachment 指针时不需要和 camera/control/light 集合混在一起处理。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 camera init/cleanup 顺序、resize aspect sync、scene-color render 参数、scene setup light mapping、editor light/camera mapping、legacy experiment hooks、PBR light/camera verification policy 或 renderer backend readiness 行为。
- Runtime Engine Attachment State extraction 与 Runtime Renderer Backend Attachment Lifecycle extraction 已在后续章节接入；后续可以继续收敛 renderer backend contract 或 RendererSubsystem frame bridge internals。

### 2026-05-31 Runtime Engine Attachment State Extraction

本轮继续推进 `AppRuntimeContext engine/runtime attachment boundary`。上一轮后 render resources、camera/light 和 profiles 已进入各自子结构，但 `AppRuntimeContext` 仍直接暴露 Engine、World、AssetSubsystem、RendererSubsystem 和 engine-world editable flag。本切片把这些 runtime attachment 指针聚合到独立 `RuntimeEngineAttachmentState`，让 `AppRuntimeContext` 只保留几个明确子边界。

新增与修改：

- 新增 `application/RuntimeEngineAttachmentState.h`。
- `RuntimeEngineAttachmentState` 集中 `Engine*`、`World*`、`AssetSubsystem*`、`RendererSubsystem*` 和 `engineWorldEditable`。
- `AppRuntimeContext.h` 删除这些 engine/runtime attachment top-level 字段，改为持有 `GL_RUNTIME::RuntimeEngineAttachmentState engineAttachments`。
- application 内 Engine lifecycle attach/detach、editor panel context mapping、engine world verification、imported asset verification、scene setup context factory 和 verification report 访问点改为 `context.engineAttachments.*`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 runtime engine attachment state header。

已完成验证：

- 静态检查确认 application 中旧的 top-level `context.engine`、`context.engineWorld`、`context.assetSubsystem`、`context.rendererSubsystem`、`context.engineWorldEditable` 访问已迁移到 `context.engineAttachments.*`。
- 静态检查确认 `RuntimeEngineAttachmentState.h` 是 Engine/World/Subsystem forward declarations 的集中包含点，`AppRuntimeContext.h` 不再直接声明这些 engine runtime types。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。

结论：

- `AppRuntimeContext` 现在主要由 `renderResources`、`cameraLights`、`engineAttachments` 和 `profiles` 四个子边界组成，不再直接暴露大量 runtime top-level 字段。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 Engine startup/shutdown、World editable gate、AssetSubsystem registry、RendererSubsystem attach/cleanup、scene setup mapping 或 verification output contract。
- Runtime Renderer Backend Attachment Lifecycle extraction 已在下一节接入；后续可以继续收敛 renderer backend contract 或推进 RendererSubsystem frame bridge internals。

### 2026-05-31 Runtime Renderer Backend Attachment Lifecycle Extraction

本轮继续推进 `engine-level renderer/backend ownership`。上一轮后 `AppRuntimeContext` 的 engine attachment 状态已经聚合，但 `RuntimeEngineLifecycle.cpp` 仍直接知道 renderer backend catalog、factory、selection 和 attachment desc 组装细节。本切片把 renderer backend attach 细节拆到独立 `RuntimeRendererBackendAttachmentLifecycle`，让 Engine lifecycle 只保留对外 attach API 和 subsystem 空指针 gate。

新增与修改：

- 新增 `application/RuntimeRendererBackendAttachmentLifecycle.h/.cpp`。
- `RuntimeRendererBackendAttachmentLifecycle::attachToRendererSubsystem(...)` 集中执行 renderer 绑定、backend key selection、backend factory 创建、attachment desc 组装和 `RendererSubsystem::setFrameExecutor(...)`。
- `RuntimeEngineLifecycle::attachRendererBackend(...)` 保留原有对外接口，但实现改为委托 attachment lifecycle；`RuntimeEngineLifecycle.cpp` 不再直接 include `RuntimeRendererBackendCatalog.h` 或 `RuntimeRendererBackendFactory.h`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 runtime renderer backend attachment lifecycle 源文件和 header。

已完成验证：

- 静态检查确认 catalog/factory、selection、attachment desc 和 `setFrameExecutor(...)` 细节集中在 `RuntimeRendererBackendAttachmentLifecycle.cpp`；`RuntimeEngineLifecycle.cpp` 只调用新 lifecycle。
- 静态检查确认新模块、VS 工程注册和 `RuntimeEngineLifecycle` 委托入口均可检索。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。

结论：

- Engine lifecycle 现在不再直接承担 runtime renderer backend selection/factory/attachment desc 细节，renderer backend attach 边界更接近独立 application-to-engine bridge。
- 该切片仍是降耦合，不扩张 PBR 功能，也不改变 backend registry key、runtime pipeline backend、no-op backend、RendererSubsystem ownership、cleanup detach 或 verification output contract。
- 后续可以继续收敛 renderer backend contract 或推进 RendererSubsystem frame bridge internals。

### 2026-05-31 RendererSubsystem Frame Bridge Stats Header Extraction

本轮继续推进 `RendererSubsystem frame bridge internals` 的低风险切片。上一轮后 runtime renderer backend attachment 细节已经从 `RuntimeEngineLifecycle` 中拆出，但 `RendererSubsystem.h` 仍直接承载 `RendererSubsystemFrameBridgeStats` 的完整字段列表和 `<string>` 依赖。本切片先把 stats DTO 拆到独立 header，让 subsystem public header 更接近行为 API + DTO 依赖组合，而不是继续堆积诊断字段。

新增与修改：

- 新增 `engine/RendererSubsystemFrameBridgeStats.h`。
- `RendererSubsystemFrameBridgeStats` 完整迁入该 header，字段名、默认值和 namespace 保持不变。
- `RendererSubsystem.h` 删除 stats 结构体定义和直接 `<string>` include，改为 include `RendererSubsystemFrameBridgeStats.h`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 engine header，filter 归入 `include\GLengine`。

已完成验证：

- 静态检查确认只有 `engine/RendererSubsystemFrameBridgeStats.h` 定义 `RendererSubsystemFrameBridgeStats`，`RendererSubsystem.h` 只 include 新 header 且不再直接 include `<string>`。
- 静态检查确认 `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新 header，filter 为 `include\GLengine`。
- `RuntimeVerificationReport` 和 `EngineDiagnosticsPanel` 仍通过 `RendererSubsystem::getFrameBridgeStats()` 观察同一 stats contract，不需要改输出字段或 UI 字段。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是纯 DTO/header 边界拆分，不改变 RendererSubsystem lifecycle、frame bridge call path、backend ownership、verification contract 或 PBR pass。
- 后续可以继续收敛 RendererSubsystem frame bridge internals，例如把 frame intent/result refresh 逻辑继续拆成更小的 engine-side frame bridge state/adapter。

### 2026-05-31 RendererSubsystem Frame Bridge State Extraction

本轮继续推进 `RendererSubsystem frame bridge internals`。上一轮已把 stats DTO 字段列表从 `RendererSubsystem.h` 中移出，但 `RendererSubsystem.cpp` 仍直接写 `RendererSubsystemFrameBridgeStats` 的 counters、backend metadata、frame intent/result 和 Engine time/delta。本切片新增 `RendererSubsystemFrameBridgeState`，把 stats mutation policy 继续下沉到 engine-side frame bridge state，`RendererSubsystem` 保留生命周期、renderer/backend 持有和 frame dispatch。

新增与修改：

- 新增 `engine/RendererSubsystemFrameBridgeState.h/.cpp`。
- `RendererSubsystemFrameBridgeState` 持有 `RendererSubsystemFrameBridgeStats`，集中 `record*` counters、frame intent/result application、backend metadata refresh、renderer observation refresh 和 Engine time/delta refresh。
- `RendererSubsystem.h` 改为持有 `RendererSubsystemFrameBridgeState`，不再直接持有 `RendererSubsystemFrameBridgeStats`。
- `RendererSubsystem.cpp` 删除直接 stats 字段写入和本地 backend state formatter，改为调用 `mFrameBridgeState.record... / apply... / refresh...`；`RendererSubsystem::getFrameBridgeStats()` 合同保持不变。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 frame bridge state `.cpp` 与 header，filter 分别为 `cppfile\GLengine` 和 `include\GLengine`。

已完成验证：

- 静态检查确认 `RendererSubsystem.cpp` 不再出现 `mFrameBridgeStats`，stats 字段写入集中到 `RendererSubsystemFrameBridgeState.cpp`。
- 静态检查确认新 state `.cpp/.h` 已注册到 VS 工程与 filters。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是纯 frame bridge state/diagnostics 写入策略拆分，不改变 renderer backend ownership、frame bridge call order、runtime frame pipeline adapter、verification output 字段或 PBR pass。
- 后续可继续沿 RendererSubsystem frame bridge internals 拆分 backend dispatch / frame execution state，或回到 renderer backend contract。

### 2026-05-31 RendererSubsystem Backend Slot Extraction

本轮继续推进 `RendererSubsystem frame bridge internals`。上一轮已把 frame bridge stats mutation policy 下沉到 `RendererSubsystemFrameBridgeState`，但 `RendererSubsystem` 仍直接持有 backend `unique_ptr`、attachment metadata，并承担 attachment desc normalization、ready 判断和 attach/detach 差异检测。本切片新增 `RendererSubsystemBackendSlot`，把这些 backend slot state 下沉到独立 engine-side object，`RendererSubsystem` 保留 lifecycle、renderer 观察和 frame dispatch facade。

新增与修改：

- 新增 `engine/RendererSubsystemBackendSlot.h/.cpp`。
- `RendererSubsystemBackendSlot` 集中持有 `std::unique_ptr<RendererBackend>` 与 `RendererBackendAttachmentDesc`，封装 `setBackend(...)`、`getBackend()`、`hasBackend()`、`isBackendReady()` 和 attachment desc normalization。
- `RendererSubsystemBackendSlotChange` 明确表达本次 backend set 操作是否发生 detach/attach，`RendererSubsystem` 只把该 change 转交给 `RendererSubsystemFrameBridgeState` 记录 lifecycle counters。
- `RendererSubsystem.h` 改为持有 `RendererSubsystemBackendSlot mBackendSlot`，不再直接持有 `std::unique_ptr<RendererBackend>` 或 `RendererBackendAttachmentDesc` 成员。
- `RendererSubsystem.cpp` 的 public API 保持不变：`setFrameExecutor(...)`、`clearFrameExecutor()`、`getFrameExecutor()`、`hasFrameExecutor()` 和 frame dispatch 仍是外部入口，但内部委托 backend slot。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 backend slot `.cpp` 与 header，filter 分别为 `cppfile\GLengine` 和 `include\GLengine`。

已完成验证：

- 静态检查确认 `RendererSubsystem.h/.cpp` 不再出现旧 `mFrameExecutor` 或 `mFrameExecutorAttachment` 成员。
- 静态检查确认 `RendererSubsystemBackendSlot`、`RendererSubsystemBackendSlotChange`、VS 工程注册、subagent 协作文档和 `recordRendererBackendAttachmentChange(...)` 调用点均可检索。
- 只读 sidecar subagent `Euler` 审计本轮 backend slot extraction，未修改文件，确认 attach/detach counters、attachment desc normalization、cleanup detach 语义、no-op backend mode 和 VS 工程注册没有阻塞问题。
- 采纳 `Euler` 的低风险建议：删除未使用的 `RendererSubsystemBackendSlot::clear()`，避免未来绕过 `RendererSubsystem::clearFrameExecutor()` 时漏记 lifecycle counters。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 backend slot state / ownership 边界拆分，不改变 runtime frame pipeline adapter、renderer backend public API、verification output 字段、cleanup detach stats 或 PBR pass。
- RendererSubsystem Backend Slot Snapshot extraction 已在下一节接入；后续可以继续把 `RendererSubsystem` 中 frame dispatch / backend execution 细节拆成更明确的 execution bridge，或者继续推进 renderer backend contract；当前仍不建议扩张 PBR 功能。

### 2026-05-31 RendererSubsystem Backend Slot Snapshot Extraction

本轮继续推进 `RendererSubsystem backend slot snapshot`。上一轮 `RendererSubsystemBackendSlot` 已经接管 backend pointer storage、attachment metadata 和 ready 判断，但 `RendererSubsystemFrameBridgeState` 仍通过 raw `RendererBackend*` 读取 backend key / ready 状态。本切片新增 backend slot snapshot，让 frame bridge state 只消费只读 DTO，不再直接探测 backend 对象。

新增与修改：

- 在 `engine/RendererSubsystemBackendSlot.h` 新增 `RendererSubsystemBackendSlotSnapshot`，集中 `attached`、`ready`、`backendKey` 和 normalized `RendererBackendAttachmentDesc`。
- `RendererSubsystemBackendSlot::captureSnapshot()` 负责生成 backend slot snapshot，并把 backend key fallback、ready 判断和 detached metadata fallback 留在 slot 内部。
- `RendererSubsystemFrameBridgeState::refreshRendererBackendStats(...)` 与 `refreshFrameBridgeStats(...)` 改为接收 `RendererSubsystemBackendSlotSnapshot`，不再接收 raw `RendererBackend*` 或 attachment desc pair。
- `RendererSubsystem.cpp` 在 backend attach 与 frame bridge stats refresh 时传入 `mBackendSlot.captureSnapshot()`。

已完成验证：

- 静态检查确认 `RendererSubsystemFrameBridgeState.h/.cpp` 不再直接出现 `RendererBackend*` probing、`getBackendKey()` 或本地 `isRendererBackendReady(...)` helper。
- 静态检查确认 `RendererSubsystemBackendSlotSnapshot`、`captureSnapshot()` 和 `backendSnapshot` 调用点均可检索。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 backend observation DTO 边界拆分，不改变 `RendererSubsystem` public API、backend ownership、frame dispatch、runtime/no-op backend behavior、cleanup detach stats、verification output 字段或 PBR pass。
- 后续可以继续把 `RendererSubsystem` 中 frame dispatch / backend execution 细节拆成更明确的 execution bridge，或者转回 renderer backend contract 命名与 API 收敛。

### 2026-05-31 RendererSubsystem Frame Execution Bridge Extraction

本轮继续推进 `RendererSubsystem frame execution bridge`。上一轮后 backend observation 已经通过 `RendererSubsystemBackendSlotSnapshot` 下沉到只读 DTO，但 `RendererSubsystem::renderFrameBridge(...)` 仍直接决定是否调用 backend 并直接执行 `backend->renderFrame(...)`。本切片新增 `RendererSubsystemFrameExecutionBridge`，把 backend frame execution / default frame result generation 从 subsystem facade 中拆出。

新增与修改：

- 新增 `engine/RendererSubsystemFrameExecutionBridge.h/.cpp`。
- 新增 `RendererSubsystemFrameExecutionResult`，明确表达 backend readiness、frame 是否实际执行，以及返回的 `RendererFrameResult`。
- `RendererSubsystemFrameExecutionBridge::executeFrame(...)` 负责基于 backend pointer 与 ready flag 执行 backend frame，backend 不可用时返回默认 frame result。
- `RendererSubsystem::renderFrameBridge(...)` 保留 frame bridge call counter、begin/end lifecycle、ready/not-ready frame counter 和 exception path；实际 backend 调用改为委托 execution bridge。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 已注册新增 execution bridge `.cpp` 与 header，filter 分别为 `cppfile\GLengine` 和 `include\GLengine`。

已完成验证：

- 静态检查确认 `RendererSubsystem.cpp` 不再直接调用 `backend->renderFrame(...)`，实际调用集中在 `RendererSubsystemFrameExecutionBridge.cpp`。
- 静态检查确认 `RendererSubsystemFrameExecutionBridge`、`RendererSubsystemFrameExecutionResult`、`mFrameExecutionBridge` 和 VS 工程注册均可检索。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 frame execution dispatch 边界拆分，不改变 renderer backend public API、backend ownership、frame bridge stats 字段、ready/not-ready counter 语义、runtime/no-op backend behavior、cleanup detach stats 或 PBR pass。
- `RendererSubsystem` 当前仍保留 subsystem lifecycle、begin/end frame bridge、stats counter coordination 和 public facade；后续建议继续收敛 renderer backend contract 命名/API，或选择下一个小型 Engine runtime boundary，避免继续扩张 PBR pass。

### 2026-05-31 RendererSubsystem Renderer Backend API Naming Cleanup

本轮继续推进 `renderer backend contract naming/API cleanup`。上一轮 execution bridge 已经让 backend frame execution 从 `RendererSubsystem` 中移出，但 public API、diagnostics 和 verification 仍以旧 `FrameExecutor` 命名作为主语。本切片把 live application 调用迁到 `RendererBackend` 命名，同时保留旧 `FrameExecutor` wrapper 和输出字段作为兼容层。

新增与修改：

- `RendererSubsystem` 新增 `setRendererBackend(...)`、`clearRendererBackend()`、`getRendererBackend()` 和 `hasRendererBackend()`。
- `setFrameExecutor(...)`、`clearFrameExecutor()`、`getFrameExecutor()` 和 `hasFrameExecutor()` 保留为 compatibility wrapper，内部转发到新的 renderer backend API。
- `RuntimeRendererBackendAttachmentLifecycle` 改为调用 `RendererSubsystem::setRendererBackend(...)`。
- `RuntimeFrameRunner` 改为通过 `hasRendererBackend()` 判断是否进入 Engine-owned renderer backend path。
- `RuntimeEngineLifecycle::beginCleanup(...)` 改为通过 `clearRendererBackend()` 清理 backend。
- `RendererSubsystemFrameBridgeStats` 新增 `rendererBackendAttached` 与 `rendererBackendFrameCallCount`；旧 `frameExecutorAttached` / `frameExecutorCallCount` 保留并与新字段同步，保证既有 verification 输出兼容。
- `RuntimeVerificationReport` 的 runtime renderer subsystem / renderer backend contract / cleanup stats 均新增 `rendererBackendAttached` 和 `rendererBackendFrameCalls` 字段。
- `EngineDiagnosticsPanel` 的 UI 标签改为 Renderer Backend Attached / Renderer Backend Frame Calls。
- `tools/verify_pbr.ps1` 新增对 `rendererBackendAttached`、`rendererBackendFrameCalls` 和 cleanup/backend detach 的断言，同时保留旧 `frameExecutor*` 断言。

已完成验证：

- 静态检查确认 application live code 不再直接调用 `setFrameExecutor(...)`、`clearFrameExecutor()` 或 `hasFrameExecutor()`；旧 API 只保留在 `RendererSubsystem` compatibility wrapper 中。
- 静态检查确认 `rendererBackendAttached`、`rendererBackendFrameCalls`、`setRendererBackend(...)`、`hasRendererBackend()` 和 `clearRendererBackend()` 调用点均可检索。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 renderer backend contract 命名与诊断字段收敛，不改变 backend ownership、frame execution bridge、runtime/no-op backend behavior、cleanup detach stats 或 PBR pass。
- 旧 `FrameExecutor` 命名仍作为兼容层存在；后续可继续收敛剩余 compatibility alias / stats 字段，或选择下一个小型 Engine runtime boundary。

### 2026-05-31 RendererSubsystem Renderer Backend Public Compatibility API Removal

本轮继续推进 `renderer backend compatibility cleanup`。上一轮已经把 live application code 迁到 `RendererBackend` 命名，但 `RendererSubsystem` public API 里仍保留 `setFrameExecutor(...)` / `hasFrameExecutor()` 等 wrapper，`RendererBackend.h` 里也仍保留旧 DTO / executor alias。本切片删除这些未被 live code 使用的公共兼容入口，仅保留 verification 输出中的旧 `frameExecutor*` 字段作为外部日志兼容。

新增与修改：

- `RendererBackend.h` 删除 `RendererFrameExecutor`、`RendererSubsystemFrameIntent` 和 `RendererSubsystemFrameResult` compatibility aliases。
- `RendererSubsystem.h/.cpp` 删除 `setFrameExecutor(...)`、`clearFrameExecutor()`、`getFrameExecutor()` 和 `hasFrameExecutor()` public compatibility wrappers。
- `RendererSubsystemFrameBridgeState.h/.cpp` 删除未使用的 `recordFrameExecutorCall()` wrapper。
- `RendererSubsystem` public renderer backend API 现在只暴露 `setRendererBackend(...)`、`clearRendererBackend()`、`getRendererBackend()` 和 `hasRendererBackend()`。
- `RendererSubsystemFrameBridgeStats` 与 verification report 仍保留 `frameExecutorAttached` / `frameExecutorCalls`，并由 renderer backend 字段同步，避免一次性破坏既有 verification/log parser。

已完成验证：

- 静态检查确认 engine/application/tools live code 中不再出现 `RendererFrameExecutor`、`RendererSubsystemFrameIntent`、`RendererSubsystemFrameResult`、`setFrameExecutor(...)`、`clearFrameExecutor()`、`getFrameExecutor()`、`hasFrameExecutor()` 或 `recordFrameExecutorCall()`。
- 静态检查确认旧 `frameExecutor*` 名称只保留在 stats/report/script compatibility 输出与断言中。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 public renderer backend API cleanup，不改变 backend ownership、frame execution bridge、runtime/no-op backend behavior、verification 输出兼容字段、cleanup detach stats 或 PBR pass。
- 后续可以继续把旧 `frameExecutor*` verification 字段逐步降级为 legacy-only 输出，或者切换到下一个小型 Engine runtime boundary；当前仍不建议扩张 PBR 功能。

### 2026-05-31 RendererSubsystem Frame Executor Stats Internal Compatibility Removal

本轮继续推进 `renderer backend compatibility cleanup`。上一轮已经删除旧 `FrameExecutor` public API / aliases，但 `RendererSubsystemFrameBridgeStats` 内部仍保存 `frameExecutorAttached` / `frameExecutorCallCount` 两份旧状态。本切片删除 Engine 内部旧 stats 字段，只在 verification/report 输出中保留旧字段名，并直接从 `rendererBackendAttached` / `rendererBackendFrameCallCount` 派生，降低内部命名债务。

新增与修改：

- `RendererSubsystemFrameBridgeStats` 删除 `frameExecutorAttached` 与 `frameExecutorCallCount`，Engine 内部 frame bridge stats 只保留 `rendererBackendAttached` 与 `rendererBackendFrameCallCount`。
- `RendererSubsystemFrameBridgeState` 删除旧字段同步逻辑，不再维护并行 `frameExecutor*` 状态。
- `RuntimeVerificationReport` 继续输出 `frameExecutorAttached`、`frameExecutorCalls` 和 `rendererFrameExecutorAttached` 兼容字段，但其值改为从 renderer-backend stats 派生。
- `tools/verify_pbr.ps1` 断言保持不变，用来证明旧输出字段仍兼容既有 log/parser。

已完成验证：

- 静态检查确认 `engine/` 下不再出现 `frameExecutorAttached`、`frameExecutorCallCount`、`frameExecutorCalls` 或 `rendererFrameExecutorAttached`。
- 静态检查确认旧 `frameExecutor*` 名称只保留在 `RuntimeVerificationReport.cpp` 文本输出和 `verify_pbr.ps1` parser/assertion 中。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 renderer backend stats internal compatibility cleanup，不改变 backend ownership、frame execution bridge、runtime/no-op backend behavior、verification 输出兼容字段名、cleanup detach stats 或 PBR pass。
- 旧 `frameExecutor*` 当前已经不是 Engine 内部状态，只是 verification/log compatibility 输出；后续可以选择最终移除这些旧输出字段，或转入下一个小型 Engine runtime boundary。

### 2026-05-31 RendererSubsystem Frame Executor Verification Output Removal

本轮继续完成 `renderer backend compatibility cleanup`。上一轮已经让旧 `frameExecutor*` 不再作为 Engine 内部状态存在，但 live verification/report 仍保留旧输出字段名作为兼容层。本切片移除这些旧 live 输出字段和脚本断言，让 renderer backend contract 只暴露 `rendererBackend*` 语义。

新增与修改：

- `RuntimeVerificationReport` 删除 `frameExecutorAttached`、`frameExecutorCalls` 和 `rendererFrameExecutorAttached` 输出字段。
- `tools/verify_pbr.ps1` 删除旧 `frameExecutor*` parser/assertion，改为只基于 `rendererBackendAttached`、`rendererBackendFrameCalls`、`rendererBackendReady`、backend key/state/ownership/registry、ready/not-ready frame counters 和 cleanup detach counters 验证 renderer backend contract。
- `Runtime renderer subsystem stats`、`Runtime renderer backend contract stats`、cleanup stats 和 engine subsystem cleanup stats 均不再输出旧 executor 字段。

已完成验证：

- 静态检查确认 `engine/application/tools` live code 中不再出现 `frameExecutorAttached`、`frameExecutorCallCount`、`frameExecutorCalls`、`rendererFrameExecutorAttached`、`RendererFrameExecutor` 或旧 `setFrameExecutor(...)` API 名称。
- 静态检查确认 `rendererBackendAttached`、`rendererBackendFrameCalls`、`rendererBackendFrameCallCount` 和 renderer backend cleanup 断言仍可检索。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 renderer backend verification output cleanup，不改变 backend ownership、frame execution bridge、runtime/no-op backend behavior、cleanup detach stats 或 PBR pass。
- 旧 `FrameExecutor` 命名已经从 live engine/application/tools contract 中清除；后续建议转入下一个小型 Engine runtime boundary，而不是继续扩张 PBR 功能。

### 2026-05-31 Runtime Renderer Backend Verification Report Formatter Extraction

本轮转入下一个小型 Engine runtime boundary。当前 `RuntimeVerificationReport` 仍直接拼接 renderer subsystem / renderer backend contract / cleanup verification 字符串，导致通用 runtime report 继续承载 renderer backend 字段格式细节。本切片把这些字符串格式化函数拆到独立 application 模块，保持 `RuntimeVerificationReport` 只负责报告触发和 `reportLine(...)` 输出。

新增与修改：

- 新增 `RuntimeRendererBackendVerificationReport.h/.cpp`，集中格式化 `Runtime renderer subsystem stats`、`Runtime renderer backend contract stats`、`Runtime renderer subsystem cleanup stats` 和 `Runtime renderer backend contract cleanup stats` 四类行。
- `RuntimeVerificationReport.cpp` 改为调用 formatter 函数获取完整字符串，仍由原 `reportLine(...)` 统一写 stdout/logger，避免改变日志副作用。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `.cpp/.h`，filter 分别保持在 `cppfile\Application` 和 `include\Application`。
- 只读 sidecar `James` 审计本切片，确认四个 verification 行前缀、字段名、cleanup detach 字段和 `verify_pbr.ps1` parser contract 必须保持稳定；本实现按该边界完成，没有让新模块接管日志输出。

已完成验证：

- 静态检查确认新增 formatter、项目注册和 `RuntimeVerificationReport` 调用点均可检索。
- 静态检查确认 `engine/application/tools` live code 仍不包含旧 `FrameExecutor` / `frameExecutor*` API 与输出字段。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过，renderer backend contract 与 cleanup 输出保持原字段。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime verification report formatter boundary extraction，不改变 renderer backend ownership、frame execution bridge、runtime/no-op backend behavior、verification 输出合同、cleanup detach stats 或 PBR pass。
- 后续仍建议继续选择小型 Engine runtime boundary，优先降 runtime/application 组合层耦合；不建议回到 PBR pass 扩张。

### 2026-05-31 Runtime Engine Verification Report Formatter Extraction

本轮继续推进 runtime verification report 边界拆分。上一轮已经把 renderer backend report 行格式化迁出，但 `RuntimeVerificationReport` 仍直接拼接 Engine lifecycle snapshot、subsystem summary、tick health、Engine World cleanup 和 Engine subsystem cleanup 输出。本切片新增 Engine verification report formatter，让 `RuntimeVerificationReport` 只负责捕获当前 runtime/Engine 指针状态并输出 formatter 返回的字符串。

新增与修改：

- 新增 `RuntimeEngineVerificationReport.h/.cpp`，集中格式化 `Runtime engine lifecycle snapshot stats`、`Runtime engine subsystem summary stats`、`Runtime engine tick stats`、`Runtime subsystem health stats`、`Runtime engine world cleanup stats` 和 `Runtime engine subsystem cleanup stats`。
- 新增 `RuntimeEngineWorldCleanupReportFields` 与 `RuntimeEngineSubsystemCleanupReportFields`，把 runtime context 指针状态、asset registry 数量和 renderer stats 指针作为明确 DTO 传入 formatter。
- `RuntimeVerificationReport.cpp` 删除本地 `EngineRunMode`、subsystem summary 和 tick count 字符串格式化 helper，改为调用 `RuntimeEngineVerificationReport`；`reportLine(...)` 输出职责保持不变。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `.cpp/.h`，filter 分别保持在 `cppfile\Application` 和 `include\Application`。

已完成验证：

- 静态检查确认新增 Engine formatter、项目注册和 `RuntimeVerificationReport` 调用点均可检索。
- Focused verification 第一次通过时发现一个 `size_t` 聚合初始化 warning，已修正为 `std::size_t{ 0 }`，随后 focused verification 重新通过且 warning 消失。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过，Engine runtime / cleanup report 行保持原字段。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime engine verification report formatter boundary extraction，不改变 Engine lifecycle、World/subsystem cleanup 行为、renderer backend contract、verification 输出合同或 PBR pass。
- 后续建议继续沿 application runtime report/lifecycle 组合层做小步降耦合，优先保持可验证边界，而不是扩张 PBR 功能。

### 2026-05-31 Runtime Verification Frame Capture Lifecycle Extraction

本轮继续推进 verification lifecycle 降耦合。当前 `RuntimeVerificationLifecycle::captureFrameIfNeeded(...)` 直接编排 framebuffer capture、runtime Engine/renderer report 和 PBR renderer stats report，使 generic verification lifecycle 继续依赖 capture/report/PBR stats 细节。本切片把 capture-frame 编排迁入独立 lifecycle 模块，`RuntimeVerificationLifecycle` 保持 public facade 不变。

新增与修改：

- 新增 `RuntimeVerificationFrameCaptureLifecycle.h/.cpp`，集中处理 verification frame capture gate、`RuntimeVerificationCapture::captureDefaultFramebuffer(...)`、`RuntimeVerificationReport::reportRenderedFrameRuntimeStats(...)`、`RuntimePBRRendererStatsVerification::reportRenderedFrame(...)` 和 `captureWritten` 状态更新。
- `RuntimeVerificationLifecycle.cpp` 的 `captureFrameIfNeeded(...)` 改为委托 `RuntimeVerificationFrameCaptureLifecycle`，同时移除对 `RuntimeVerificationCapture`、`RuntimePBRRendererStatsVerification` 以及 Engine/subsystem implementation headers 的直接依赖。
- `RuntimeVerificationLifecycle` public API 不变，frame capture 调用方不需要改动。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `.cpp/.h`，filter 分别保持在 `cppfile\Application` 和 `include\Application`。

已完成验证：

- 静态检查确认新增 lifecycle、调用点、VS 工程注册，以及 capture/report/PBR stats 依赖已集中到新模块。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过，capture 后的 runtime Engine/renderer/PBR stats report 仍保持原输出。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime verification frame capture lifecycle boundary extraction，不改变 verification trigger、capture 输出、runtime report、PBR renderer stats report、cleanup stats 或 PBR pass。
- 后续建议继续把 `RuntimeVerificationLifecycle` 中 prepared-scene 或 cleanup 编排按同样方式拆成小模块，持续降低 generic lifecycle 对具体 verification domain 的依赖。

### 2026-05-31 Runtime Verification Prepared Scene Lifecycle Extraction

本轮继续沿 `RuntimeVerificationLifecycle` 做 application verification lifecycle 降耦合。上一轮已经把 capture-frame 编排拆出，但 `reportPreparedScene(...)` 仍直接串联 Engine World probe、PBR scene probe、imported asset probe、renderer pass profile 和 prepared-scene stats。该切片把 prepared-scene verification orchestration 迁入独立 lifecycle 模块，保持 `RuntimeVerificationLifecycle` 作为 public facade。

新增与修改：

- 新增 `RuntimeVerificationPreparedSceneLifecycle.h/.cpp`，集中处理 `config.enabled` gate、Engine World probe、PBR scene probe、imported asset probe、renderer pass profile refresh、`PBR verification scene stats` 和 `Engine world prepared scene stats` report 顺序。
- `RuntimeVerificationLifecycle.cpp` 的 `reportPreparedScene(...)` 改为委托 `RuntimeVerificationPreparedSceneLifecycle`，并移除对 Engine World/PBR scene probe/import/pass-profile/prepared-scene stats 模块的直接 include。
- `RuntimeVerificationLifecycle` public API 不变，content verification lifecycle 调用方不需要改动。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `.cpp/.h`，filter 分别保持在 `cppfile\Application` 和 `include\Application`。
- 只读 sidecar `Mendel` 审计本切片，确认这是低风险拆分，关键约束是保持 prepared-scene 调用/输出顺序、VS 工程注册和 verification parser contract 不变；本实现按该边界完成。

已完成验证：

- 静态检查确认新增 lifecycle、调用点、VS 工程注册，以及 prepared-scene domain 依赖已集中到新模块。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,import,deferred-emissive,deferred-material-ibl,deferred-alpha-mask,deferred-texture-set,showcase-spheres,engine-world-scene-probe,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；十条 focused verification mode 全部通过，覆盖 PBR probes、import probe、showcase、Engine World probes 和 no-op backend。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime verification prepared-scene lifecycle boundary extraction，不改变 scene probe 创建、renderer pass profile、prepared-scene stats、runtime renderer backend contract、cleanup stats 或 PBR pass。
- 后续建议继续把 `RuntimeVerificationLifecycle` 中 cleanup report 或 startup profile 编排拆成小模块，持续降低 generic verification lifecycle 对具体 verification domain 的依赖。

### 2026-05-31 Runtime Verification Cleanup Lifecycle Extraction

本轮继续沿 `RuntimeVerificationLifecycle` 做 verification lifecycle 降耦合。上一轮 prepared-scene 编排已经拆出，但 cleanup 阶段仍直接调用 `RuntimeVerificationReport` 输出 renderer subsystem cleanup、renderer backend contract cleanup、Engine World cleanup 和 Engine subsystem cleanup。该切片把 cleanup report 编排迁入独立 lifecycle 模块，保持 `RuntimeVerificationLifecycle` public facade 不变。

新增与修改：

- 新增 `RuntimeVerificationCleanupLifecycle.h/.cpp`，集中处理 cleanup report 的 `config.enabled` gate、renderer subsystem null gate、renderer subsystem cleanup report、Engine World cleanup report 和 Engine subsystem cleanup report。
- `RuntimeVerificationLifecycle.cpp` 的 `reportRendererSubsystemCleanup(...)` 与 `reportEngineCleanup(...)` 改为委托 `RuntimeVerificationCleanupLifecycle`，并移除对 `RuntimeVerificationReport.h` 的直接 include。
- `RuntimeVerificationLifecycle` public API 不变，shutdown verification bridge 调用方不需要改动。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `.cpp/.h`，filter 分别保持在 `cppfile\Application` 和 `include\Application`。
- 本轮没有启动新的 sidecar：该切片写域小且没有独立 sidecar 写域，按 parent-owned 方式完成实现、验证和文档记录。

已完成验证：

- 静态检查确认新增 cleanup lifecycle、调用点、VS 工程注册，以及 cleanup report 依赖已集中到新模块。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过，覆盖 runtime renderer cleanup、no-op backend cleanup、Engine World cleanup 和 scene package cleanup。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime verification cleanup lifecycle boundary extraction，不改变 cleanup ordering、runtime context detach state、renderer backend cleanup stats、Engine World cleanup stats、Engine subsystem cleanup stats 或 PBR pass。
- 后续建议继续处理 `RuntimeVerificationLifecycle::applyStartupProfile(...)`，把 startup profile gate 和 PBR profile application 再拆成一个小型 lifecycle boundary。

### 2026-05-31 Runtime Verification Startup Profile Lifecycle Extraction

本轮继续收敛 `RuntimeVerificationLifecycle` 的具体 domain 依赖。cleanup、prepared-scene 和 capture-frame 编排已经拆出后，`applyStartupProfile(...)` 仍直接持有 `RuntimePBRProfileVerification` 依赖。本切片把 startup profile gate 与 PBR profile application 委托迁入独立 lifecycle 模块，让 generic verification lifecycle 继续向 facade 收敛。

新增与修改：

- 新增 `RuntimeVerificationStartupProfileLifecycle.h/.cpp`，集中处理 `config.enabled` gate 和 `RuntimePBRProfileVerification::applyProfile(...)` 调用。
- `RuntimeVerificationLifecycle.cpp` 的 `applyStartupProfile(...)` 改为委托 `RuntimeVerificationStartupProfileLifecycle`，并移除对 `RuntimePBRProfileVerification.h` 的直接 include。
- `RuntimeVerificationLifecycle` public API 不变，content verification lifecycle 调用方不需要改动。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `.cpp/.h`，filter 分别保持在 `cppfile\Application` 和 `include\Application`。
- 本轮没有启动新的 sidecar：该切片写域小且没有独立 sidecar 写域，按 parent-owned 方式完成实现、验证和文档记录。

已完成验证：

- 静态检查确认新增 startup profile lifecycle、调用点、VS 工程注册，以及 PBR profile 依赖已集中到新模块。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,deferred-tiled-lights-pressure-timing,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过，覆盖默认 profile、pressure timing profile、Engine World minimal profile 和 no-op backend profile 路径。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime verification startup profile lifecycle boundary extraction，不改变 startup profile defaults、renderer pass profile、preview/light/camera profile、profile applied 输出、renderer backend contract 或 PBR pass。
- 后续建议继续收敛 `RuntimeVerificationLifecycle` public header 依赖，或把 `shouldStopAfterFrames(...)` 提成小型 stop policy；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Verification Stop Policy Extraction

本轮继续收敛 `RuntimeVerificationLifecycle` facade。startup profile、prepared-scene、frame capture 和 cleanup 编排已经拆出后，facade 内部只剩 `shouldStopAfterFrames(...)` 的 max-frame 判断。本切片把 verification stop condition 提成独立 policy，保持外部调用点不变。

新增与修改：

- 新增 `RuntimeVerificationStopPolicy.h/.cpp`，集中处理 `config.enabled && renderedFrameCount >= config.maxFrames` 的停止判断。
- `RuntimeVerificationLifecycle.cpp` 的 `shouldStopAfterFrames(...)` 改为委托 `RuntimeVerificationStopPolicy`，`RuntimeVerificationLifecycle` public API 不变，`RuntimeFrameLifecycle` 调用方不需要改动。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `.cpp/.h`，filter 分别保持在 `cppfile\Application` 和 `include\Application`。
- 本轮没有启动新的 sidecar：该切片写域小且没有独立 sidecar 写域，按 parent-owned 方式完成实现、验证和文档记录。

已完成验证：

- 静态检查确认新增 stop policy、调用点、VS 工程注册，以及 `maxFrames` 判断已集中到新模块。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,deferred-tiled-lights-pressure-timing,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过，覆盖默认 2 帧、pressure timing 5 帧、Engine World 和 no-op backend。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime verification stop policy boundary extraction，不改变 verification max-frame 语义、frame loop、capture timing、cleanup timing、renderer backend contract 或 PBR pass。
- 后续建议继续收敛 `RuntimeVerificationLifecycle` public header 依赖，优先处理 forward declaration/header include boundary；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Verification Lifecycle Header Forward Boundary

本轮继续收敛 `RuntimeVerificationLifecycle` facade 的 public header 依赖。经过前几轮拆分后，facade 只需要通过引用暴露 `AppRuntimeContext` 与 `RuntimeVerificationConfig`，不需要在 public header 中包含完整定义。本切片去掉这两个传递 include，让调用方按自身数据所有权显式包含配置头。

新增与修改：

- `RuntimeVerificationLifecycle.h` 移除 `AppRuntimeContext.h` 与 `RuntimeVerificationConfig.h` include，改为 forward declare `GLframework::AppRuntimeContext` 和 `GL_RUNTIME::RuntimeVerificationConfig`。
- `RuntimeFrameLifecycle.h` 因为值成员 `RuntimeVerificationConfig verification{}` 需要完整类型，显式 include `RuntimeVerificationConfig.h`，不再依赖 `RuntimeVerificationLifecycle.h` 的传递 include。
- `RuntimeVerificationLifecycle` public API、调用方入口、stop policy、startup profile、prepared-scene、frame capture 和 cleanup delegate 均不变。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 `RuntimeVerificationLifecycle.h` 只剩 forward declarations，`RuntimeFrameLifecycle.h` 显式持有 `RuntimeVerificationConfig.h`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过，覆盖 default runtime frame path、Engine World minimal path 和 no-op backend path。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime verification facade header boundary cleanup，不改变 runtime behavior、verification config data model、frame loop、renderer backend contract 或 PBR pass。
- 后续建议继续检查 `RuntimeFrameLifecycle.h` 的 public header include surface，优先减少 frame lifecycle 对 runtime runner / legacy experiment runner 的传递依赖。

### 2026-05-31 Runtime Frame Callbacks Header Extraction

本轮继续收敛 `RuntimeFrameLifecycle.h` 的 public include surface。此前 `RuntimeFrameLifecycle.h` 为了暴露 `RuntimeFrameCallbacks` 间接包含 `RuntimeFrameRunner.h`，同时把 `AppRuntimeContext.h`、`LegacyExperimentRunner.h` 和 verification lifecycle implementation header 带入 frame lifecycle public header。本切片把 callback DTO 拆成独立轻量头，让 frame lifecycle header 只保留必要值类型和 forward declarations。

新增与修改：

- 新增 `RuntimeFrameCallbacks.h`，单独定义 `RuntimeFrameCallbacks` 和 `renderUi` callback。
- `RuntimeFrameRunner.h` 改为包含 `RuntimeFrameCallbacks.h`，并 forward declare `AppRuntimeContext` 与 `LegacyExperimentRunner`，不再传递 include context 或 legacy experiment runner 完整头。
- `RuntimeFrameLifecycle.h` 改为包含 `RuntimeFrameCallbacks.h`、`RuntimeFrameClock.h` 和 `RuntimeVerificationConfig.h`，并 forward declare `AppRuntimeContext`、`LegacyExperimentRunner`、`Engine` 和 `RendererSubsystem`；runner 与 verification lifecycle 依赖迁到 `.cpp`。
- `RuntimeApplicationFrameEditorCallbackBridge.h` 与 `RuntimeEditorLifecycle.h` 改为包含 `RuntimeFrameCallbacks.h`，不再为了返回 callback DTO 依赖 `RuntimeFrameRunner.h`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `RuntimeFrameCallbacks.h`。

已完成验证：

- 静态检查确认 callback DTO 可检索，`RuntimeFrameLifecycle.h` 不再 include `RuntimeFrameRunner.h`、`AppRuntimeContext.h`、`RuntimeVerificationLifecycle.h` 或 `LegacyExperimentRunner.h`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime frame callback DTO / header surface cleanup，不改变 frame loop、editor callback 行为、legacy experiment update、renderer backend contract、verification capture 或 PBR pass。
- 后续建议继续检查 `RuntimeFrameRunner.h` / `RuntimeFrameLifecycle.h` 是否可以进一步拆分 frame config 或 frame state 类型，但优先保持小切片和全量 verification。

### 2026-05-31 Runtime Frame Lifecycle Types Header Extraction

本轮继续收敛 `RuntimeFrameLifecycle.h` 的 public include surface。上一轮已把 frame callbacks 拆成轻量 DTO，本轮把 `RuntimeFrameLifecycleConfig` 与 `RuntimeFrameLifecycleState` 从 lifecycle facade header 中拆出，让 lifecycle header 只暴露行为入口和引用参数。

新增与修改：

- 新增 `RuntimeFrameLifecycleTypes.h`，集中定义 `RuntimeFrameLifecycleConfig` 和 `RuntimeFrameLifecycleState`。
- `RuntimeFrameLifecycle.h` 不再 include `RuntimeFrameClock.h` 或 `RuntimeVerificationConfig.h`，只 forward declare `RuntimeFrameLifecycleConfig` / `RuntimeFrameLifecycleState` 并保留 `RuntimeFrameCallbacks.h`。
- `RuntimeFrameLifecycle.cpp` 显式 include `RuntimeFrameLifecycleTypes.h`，继续访问 frame clock、verification config 和 frame state 字段。
- `RuntimeApplicationConfigPolicy.h` 与 `RuntimeApplicationState.h` 改为 include `RuntimeFrameLifecycleTypes.h`，因为它们分别按值返回/持有 frame lifecycle config/state。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `RuntimeFrameLifecycleTypes.h`。

已完成验证：

- 静态检查确认 config/state 定义只在 `RuntimeFrameLifecycleTypes.h`，`RuntimeFrameLifecycle.h` 只保留 forward declarations。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime frame lifecycle config/state DTO / header surface cleanup，不改变 frame loop、fixed delta policy、verification stop/capture、editor callback、renderer backend contract 或 PBR pass。
- 后续建议继续检查 `RuntimeFrameRunner.h` 的 frame config DTO 是否也应拆成轻量 types header，或继续推进 application frame bridge 的头文件边界。

### 2026-05-31 Runtime Application Frame Editor Callback Bridge Header Boundary

本轮按 `/subagents` 协作模式启动只读 sidecar `Ptolemy` 审查下一步边界。`Ptolemy` 建议先做更小的 application frame bridge header cleanup，而不是立刻拆 `RuntimeFrameRunnerTypes.h`：`RuntimeFrameConfig` 当前没有泄漏到 bridge public header，优先清理 editor callback bridge 的 callback DTO 传递 include 风险更低。

新增与修改：

- `RuntimeApplicationFrameEditorCallbackBridge.h` 移除 `RuntimeFrameCallbacks.h` include，改为 forward declare `RuntimeFrameCallbacks`。
- `RuntimeApplicationFrameEditorCallbackBridge.cpp` 显式 include `RuntimeFrameCallbacks.h`，把完整 callback DTO 依赖局部化到 implementation。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。
- `Ptolemy` 为只读 sidecar，未修改文件；父 agent 接受其最小写域建议并本地完成实现、验证和文档记录。

已完成验证：

- 静态检查确认 bridge header 不再 include callback 完整定义，`.cpp` 显式 include `RuntimeFrameCallbacks.h`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 application frame editor callback bridge public header boundary cleanup，不改变 editor callback 执行、GUI gating、legacy experiment ticking、frame clock delta、verification capture timing、renderer backend contract 或 PBR pass。
- 后续建议继续选择 `RuntimeFrameRunner` config DTO header extraction；该切片需要新增 runner types header 并注册 VS project/filter。

### 2026-05-31 Runtime Frame Runner Types Header Extraction

本轮继续收敛 `RuntimeFrameRunner.h` 的 public include/type surface。上一轮确认 `RuntimeFrameConfig` 没有泄漏到 application bridge public header，但它仍作为字段列表定义在 runner facade header 中。本切片把 runner frame config DTO 拆到独立 types header，让 runner header 只暴露 run entry、callback 默认参数所需的 callback DTO 和 config forward declaration。

新增与修改：

- 新增 `RuntimeFrameRunnerTypes.h`，集中定义 `RuntimeFrameConfig`。
- `RuntimeFrameRunner.h` 移除 `RuntimeFrameConfig` 字段定义，改为 forward declare `RuntimeFrameConfig`；同时移除不再需要的 `Engine` / `RendererSubsystem` forward declarations。
- `RuntimeFrameRunner.cpp` 和 `RuntimeFrameLifecycle.cpp` 显式 include `RuntimeFrameRunnerTypes.h`，分别用于访问 config 字段和构造 frame config。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `RuntimeFrameRunnerTypes.h` 到 `include\Application`。

已完成验证：

- 静态检查确认 `RuntimeFrameConfig` 的真实定义只在 `RuntimeFrameRunnerTypes.h`，`RuntimeFrameRunner.h` 只保留 forward declaration。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime frame runner config DTO / header surface cleanup，不改变 frame loop、editor callback execution、legacy experiment ticking、renderer backend bridge fallback、verification capture timing、renderer backend contract 或 PBR pass。
- 后续建议继续选择 application frame run/continue bridge header boundary cleanup，进一步减少 application frame facade 的 public include surface。

### 2026-05-31 Runtime Application Frame Bridge Implementation Include Cleanup

本轮检查 application frame continue/run bridge header boundary。`RuntimeApplicationFrameContinueBridge.h`、`RuntimeApplicationFrameRunBridge.h` 与 `RuntimeApplicationFrameLifecycle.h` 已经只使用 forward declarations，没有 public include 泄漏；因此本轮选择继续收敛 implementation include surface，把不访问字段的 facade `.cpp` 和已经通过 policy header 间接获得 config 类型的 bridge `.cpp` 中的冗余完整 include 移除。

新增与修改：

- `RuntimeApplicationFrameLifecycle.cpp` 移除 `RuntimeApplicationConfig.h` 与 `RuntimeApplicationState.h` include，只保留 frame facade 对 continue/run bridge 的转发依赖。
- `RuntimeApplicationFrameContinueBridge.cpp` 移除冗余 `RuntimeApplicationConfig.h` include；该文件仍显式 include `RuntimeApplicationState.h`，因为需要访问 `state.frameLifecycle`。
- `RuntimeApplicationFrameRunBridge.cpp` 移除冗余 `RuntimeApplicationConfig.h` include；该文件仍显式 include `RuntimeApplicationState.h`，因为需要展开 runtime、engine、engine lifecycle、legacy experiments 和 frame lifecycle。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 frame facade `.cpp` 只保留 bridge headers，continue/run bridge `.cpp` 只保留实际字段访问需要的 state include 和 policy/lifecycle/window dependencies。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 application frame bridge implementation include cleanup，不改变 shouldContinue/runFrame 转发、editor callback construction、legacy experiment ticking、frame clock delta、verification capture timing、renderer backend contract 或 PBR pass。
- 后续建议检查 shutdown bridge 或 startup bridge 的 implementation/header include surface，继续做小切片降耦合。

### 2026-05-31 Runtime Application Shutdown Bridge Implementation Include Cleanup

本轮继续检查 application shutdown bridge 的 header/implementation include surface。`RuntimeApplicationShutdownLifecycle.h`、`RuntimeApplicationShutdownCleanupBridge.h`、`RuntimeApplicationShutdownDestroyBridge.h`、`RuntimeApplicationShutdownEngineBridge.h` 与 `RuntimeApplicationShutdownVerificationBridge.h` 已经主要通过 forward declarations 暴露 API；可收敛点集中在 facade/cleanup implementation 的冗余完整 config/state include。

新增与修改：

- `RuntimeApplicationShutdownLifecycle.cpp` 移除 `RuntimeApplicationConfig.h` 与 `RuntimeApplicationState.h` include；该 facade `.cpp` 只转发到 cleanup/destroy bridge，不访问 config/state 字段。
- `RuntimeApplicationShutdownCleanupBridge.cpp` 移除冗余 `RuntimeApplicationConfig.h` include；该文件仍显式 include `RuntimeApplicationState.h`，因为 cleanup 顺序需要访问 `state.runtime` 和 `state.engine`。
- `RuntimeApplicationShutdownVerificationBridge.cpp` 继续保留 `RuntimeApplicationConfig.h`，因为它读取 `config.verification` 并把 verification config 传给 `RuntimeVerificationLifecycle`。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 shutdown facade `.cpp` 只依赖 cleanup/destroy bridge headers，cleanup bridge `.cpp` 不再依赖完整 config，verification bridge `.cpp` 保留真正需要的 config include。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 application shutdown bridge implementation include cleanup，不改变 shutdown cleanup ordering、camera cleanup、runtime context detach、Engine shutdown、renderer/Engine cleanup verification output、renderer backend contract cleanup 或 PBR pass。
- 后续建议继续选择 startup bridge implementation/header include surface cleanup，保持小切片降耦合。

### 2026-05-31 Runtime Application Startup Bridge Include Surface Cleanup

本轮继续检查 application startup facade 与各 startup bridge 的 header/implementation include surface。`RuntimeApplicationStartupLifecycle.h`、content/editor/engine/frame/window startup bridge headers 已经主要使用 forward declarations；可收敛点集中在 startup facade `.cpp`、content/editor/engine startup implementation 的冗余完整 config/state include，以及 `RuntimeApplicationGraphicsStartupLifecycle.h` 对完整 shell config header 的 public include。

新增与修改：

- `RuntimeApplicationStartupLifecycle.cpp` 移除 `RuntimeApplicationConfig.h` 与 `RuntimeApplicationState.h` include；该 facade `.cpp` 只负责编排 engine/window/graphics/content/editor/frame startup bridge，不访问 config/state 字段。
- `RuntimeApplicationContentStartupLifecycle.cpp` 移除冗余 `RuntimeApplicationConfig.h` include；仍保留 `RuntimeApplicationState.h`，因为需要访问 runtime、engine、engine lifecycle 和 legacy experiment state。
- `RuntimeApplicationEditorStartupLifecycle.cpp` 移除冗余 `RuntimeApplicationConfig.h` 与 `RuntimeApplicationState.h` include；该 bridge 不访问 state 字段，config 完整依赖由 policy header 局部承载。
- `RuntimeApplicationEngineStartupLifecycle.cpp` 移除冗余 `RuntimeApplicationConfig.h` include；仍保留 `RuntimeApplicationState.h`，因为需要访问 runtime、engine 和 engine lifecycle state。
- `RuntimeApplicationGraphicsStartupLifecycle.h` 移除 `RuntimeApplicationConfig.h` public include，改为 forward declare `RuntimeApplicationShellConfig`。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 startup facade `.cpp` 只保留 startup bridge headers，graphics startup header 只 forward declare shell config，仍访问字段的 window/content/engine/frame bridge 保留必要完整 state/config/window dependencies。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 application startup bridge include surface cleanup，不改变 Engine startup、window initialization、graphics initialization、content preparation、editor initialization、frame lifecycle reset、renderer backend contract 或 PBR pass。
- 后续建议继续选择 `RuntimeApplicationConfigPolicy` header include surface cleanup 或 shutdown cleanup refs type boundary cleanup，保持小切片降耦合。

### 2026-05-31 Runtime Application Config Policy Header Include Surface Cleanup

本轮继续收敛 `RuntimeApplicationConfigPolicy.h` 的 public include surface。该 policy header 只需要声明从 shell config 到 Engine/frame/editor/graphics lifecycle config 的映射函数；完整的 shell config、lifecycle config DTO 与 `EngineDesc` 字段访问都只发生在 `.cpp` 或具体调用点中。因此本轮把 policy header 改成纯前置声明边界，避免所有 include policy 的 startup/frame bridge implementation 间接获得完整 config/lifecycle/EngineContext 依赖。

新增与修改：

- `RuntimeApplicationConfigPolicy.h` 移除 `RuntimeApplicationConfig.h`、`RuntimeEditorLifecycle.h`、`RuntimeFrameLifecycleTypes.h`、`RuntimeGraphicsLifecycle.h` 与 `EngineContext.h` public include，改为 forward declare `RuntimeApplicationShellConfig`、`RuntimeFrameLifecycleConfig`、`RuntimeEditorLifecycleConfig`、`RuntimeGraphicsLifecycleConfig` 和 `GLengine::EngineDesc`。
- `RuntimeApplicationConfigPolicy.cpp` 显式 include 完整 config/lifecycle/EngineContext headers，因为这里实际读取 shell config 字段并构造返回 DTO。
- `RuntimeApplicationEngineStartupLifecycle.cpp` 显式 include `EngineContext.h`，因为该调用点直接消费 `makeEngineDesc(...)` 的返回临时对象，需要 `EngineDesc` 完整定义。
- 其它 policy 调用点已经通过对应 lifecycle/state header 获得返回类型完整定义，本轮不额外扩大 include。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 `RuntimeApplicationConfigPolicy.h` 只保留 forward declarations，完整 headers 集中在 `RuntimeApplicationConfigPolicy.cpp` 与需要完整 `EngineDesc` 的 engine startup implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 RuntimeApplicationConfigPolicy header include surface cleanup，不改变 Engine desc mapping、frame lifecycle config mapping、editor lifecycle config mapping、graphics lifecycle config mapping、startup/frame bridge 行为、renderer backend contract 或 PBR pass。
- 后续建议继续选择 `RuntimeContentConfigPolicy` header include surface cleanup 或 shutdown cleanup refs type boundary cleanup，保持小切片降耦合。

### 2026-05-31 Runtime Content Config Policy Header Include Surface Cleanup

本轮继续收敛 `RuntimeContentConfigPolicy.h` 的 public include surface。该 policy header 只需要声明从 shell config 到 content lifecycle config 的映射函数；完整 shell config 字段访问和 content lifecycle config 构造都发生在 `.cpp` 中，显式消费返回临时对象的调用点负责获得返回类型完整定义。因此本轮把 content config policy header 改成 forward declaration boundary，避免所有 include policy 的 startup implementation 间接获得完整 shell/content config 依赖。

新增与修改：

- `RuntimeContentConfigPolicy.h` 移除 `RuntimeApplicationConfig.h` 与 `RuntimeContentLifecycleConfig.h` public include，改为 forward declare `RuntimeApplicationShellConfig` 与 `RuntimeContentLifecycleConfig`。
- `RuntimeContentConfigPolicy.cpp` 显式 include 完整 shell/content config headers，因为这里实际读取 shell config 字段并构造 content lifecycle config。
- `RuntimeApplicationContentStartupLifecycle.cpp` 负责显式 include `RuntimeContentLifecycleConfig.h`，因为它直接消费 `makeContentLifecycleConfig(...)` 的返回临时对象。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 `RuntimeContentConfigPolicy.h` 只保留 forward declarations，完整 headers 集中在 `RuntimeContentConfigPolicy.cpp` 和显式消费返回临时对象的 content startup 调用点。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 RuntimeContentConfigPolicy header include surface cleanup，不改变 content lifecycle config mapping、camera config、scene setup policy、verification scene flags、renderer backend key、content preparation、renderer backend contract 或 PBR pass。
- 后续建议继续选择 shutdown cleanup refs type boundary cleanup 或 runtime content lifecycle config/data boundary cleanup，保持小切片降耦合。

### 2026-05-31 Runtime Content Lifecycle Header Config Forward Boundary Cleanup

本轮继续收敛 `RuntimeContentLifecycle.h` 的 public include surface。`RuntimeContentLifecycle::prepare(...)` 只通过 `const RuntimeContentLifecycleConfig&` 接收 content startup config，因此 lifecycle facade header 不需要暴露完整 `RuntimeContentLifecycleConfig` 字段定义。完整 DTO 依赖应留在读取字段的 `.cpp` 和构造/消费临时返回值的 application content startup 调用点。

新增与修改：

- `RuntimeContentLifecycle.h` 移除 `RuntimeContentLifecycleConfig.h` public include，改为 forward declare `RuntimeContentLifecycleConfig`。
- `RuntimeContentLifecycle.cpp` 显式 include `RuntimeContentLifecycleConfig.h`，因为 implementation 需要访问 `config.camera`、`config.verification`、`config.scene` 和 `config.rendererBackendKey`。
- `RuntimeApplicationContentStartupLifecycle.cpp` 显式 include `RuntimeContentLifecycleConfig.h`，因为该调用点直接消费 `RuntimeContentConfigPolicy::makeContentLifecycleConfig(...)` 的返回临时对象。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 `RuntimeContentLifecycle.h` 只保留 config forward declaration，完整 config DTO include 位于 lifecycle implementation 与 content startup 调用点。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 RuntimeContentLifecycle header config forward boundary cleanup，不改变 camera initialization、startup profile load、verification startup profile、scene preparation、renderer backend attachment、prepared-scene reporting、renderer backend contract 或 PBR pass。
- 后续建议继续选择 shutdown cleanup refs type boundary cleanup 或 runtime content sub-lifecycle header include cleanup，保持小切片降耦合。

### 2026-05-31 Runtime Content Camera/Scene Lifecycle Header Config Forward Boundary Cleanup

本轮继续收敛 content sub-lifecycle public header include surface。`RuntimeContentCameraLifecycle::initializeCamera(...)` 与 `RuntimeContentSceneLifecycle::prepareScene(...)` 都只通过 `const&` 接收 config DTO，因此这两个 sub-lifecycle facade headers 不需要 public include 完整 config headers。完整 DTO 依赖应留在实际转发调用的 `.cpp` 和聚合 config 持有方。

新增与修改：

- `RuntimeContentCameraLifecycle.h` 移除 `RuntimeCameraConfig.h` public include，改为 forward declare `RuntimeCameraConfig`。
- `RuntimeContentCameraLifecycle.cpp` 显式 include `RuntimeCameraConfig.h`，保持 camera config 完整定义位于 implementation。
- `RuntimeContentSceneLifecycle.h` 移除 `RuntimeScenePrepareConfig.h` public include，改为 forward declare `RuntimeScenePrepareConfig`。
- `RuntimeContentSceneLifecycle.cpp` 显式 include `RuntimeScenePrepareConfig.h`，保持 scene prepare config 完整定义位于 implementation。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 camera/scene content sub-lifecycle headers 只保留 config forward declarations，完整 config DTO includes 位于对应 `.cpp` 与 `RuntimeContentLifecycleConfig.h`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 RuntimeContent Camera/Scene sub-lifecycle header config forward cleanup，不改变 camera initialization、scene preparation sequence、legacy experiment prepare、renderer prepared report、renderer backend contract 或 PBR pass。
- 后续建议继续选择 shutdown cleanup refs type boundary cleanup 或 runtime content verification/renderer backend sub-lifecycle header include cleanup，保持小切片降耦合。

### 2026-05-31 Runtime Application Shutdown Cleanup Refs Canonicalization

本轮继续处理上一轮建议中的 shutdown cleanup refs type boundary cleanup。`RuntimeEngineLifecycle::beginCleanup(...)` 已经返回 `RuntimeEngineLifecycleCleanupRefs`，application shutdown 层此前又定义了一份结构相同的 `RuntimeApplicationShutdownCleanupRefs` 并在 bridge 中做字段复制。这会让 cleanup refs 出现两个 owner 名称，后续继续拆 shutdown verification 或 Engine lifecycle 时容易产生 DTO 漂移。

新增与修改：

- `RuntimeApplicationShutdownEngineBridge.h` 移除 application-local `RuntimeApplicationShutdownCleanupRefs` 定义，改为 forward declare `RuntimeEngineLifecycleCleanupRefs`。
- `RuntimeApplicationShutdownEngineBridge::beginCleanup(...)` 直接返回 `RuntimeEngineLifecycle::beginCleanup(...)` 的 canonical refs，不再做字段复制。
- `RuntimeApplicationShutdownVerificationBridge.h/.cpp` 改为接收 `RuntimeEngineLifecycleCleanupRefs`，完整 type include 局部化到需要访问 `assetSubsystem` / `rendererSubsystem` 字段的 `.cpp`。
- `RuntimeApplicationShutdownCleanupBridge.cpp` 改为持有 `RuntimeEngineLifecycleCleanupRefs`，并显式 include `RuntimeEngineLifecycleTypes.h`。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认旧 `RuntimeApplicationShutdownCleanupRefs` 已完全移除，shutdown cleanup / verification bridge 统一使用 `RuntimeEngineLifecycleCleanupRefs`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 shutdown cleanup、engine 和 verification bridge 改动文件。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 shutdown cleanup refs canonicalization，不改变 renderer backend cleanup、camera cleanup、runtime context detach、Engine shutdown、cleanup verification report 或 PBR pass。
- 后续建议继续选择 runtime content verification/renderer backend sub-lifecycle header include cleanup，或继续清理 `RuntimeEngineLifecycle.h` 与 shutdown bridge header 的 type include surface；保持小切片降耦合。

### 2026-05-31 Runtime Engine Lifecycle Header Type Include Surface Cleanup

本轮继续上一轮建议中的 `RuntimeEngineLifecycle.h` type include surface cleanup。`RuntimeEngineLifecycle.h` 是 Engine lifecycle facade header，当前只需要声明行为入口；`RuntimeEngineLifecycleState` 与 `RuntimeEngineLifecycleCleanupRefs` 的完整字段定义已经集中在 `RuntimeEngineLifecycleTypes.h`，因此 facade header 不需要继续 public include types header。完整 type 依赖应留在实现文件、按值持有 state 的 application state，以及需要读取 cleanup refs 字段的 shutdown bridge implementation。

新增与修改：

- `RuntimeEngineLifecycle.h` 移除 `RuntimeEngineLifecycleTypes.h` public include，改为 forward declare `RuntimeEngineLifecycleState` 与 `RuntimeEngineLifecycleCleanupRefs`。
- `RuntimeEngineLifecycle.cpp` 显式 include `RuntimeEngineLifecycleTypes.h`，因为 implementation 需要访问 state 字段并构造 cleanup refs。
- `RuntimeApplicationState.h` 继续 include `RuntimeEngineLifecycleTypes.h`，因为它按值持有 `RuntimeEngineLifecycleState`。
- shutdown cleanup / verification bridge implementation 继续 include `RuntimeEngineLifecycleTypes.h`，因为它们持有或读取 `RuntimeEngineLifecycleCleanupRefs`。
- 本轮没有新增 `.cpp/.h` 文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 `RuntimeEngineLifecycle.h` 只保留 lifecycle state/cleanup refs forward declarations，完整 `RuntimeEngineLifecycleTypes.h` include 位于 implementation 或按值持有/访问字段的调用点。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 `RuntimeEngineLifecycle.cpp` 以及依赖该 header 的 startup、shutdown、content renderer backend 调用点。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine lifecycle facade header type include surface cleanup，不改变 Engine initialization、renderer backend attachment、begin cleanup、runtime context detach、Engine shutdown、cleanup verification report 或 PBR pass。
- 后续建议继续选择 runtime content verification/renderer backend sub-lifecycle header include cleanup，或进一步评估 renderer backend attachment lifecycle header 的 `<string>` public include surface；保持小切片降耦合。

### 2026-05-31 Runtime Renderer Backend Key String View Boundary Cleanup

本轮继续上一轮建议中的 renderer backend attachment lifecycle `<string>` public include surface cleanup。renderer backend key 在 attachment/catalog/registry 路径中只是只读选择 key，不需要每层 API 都表达为 `const std::string&`。本切片把只读 key 入参收敛为 `std::string_view`，保留 config DTO、registry selection 和 attachment desc 中的 `std::string` 持久化字段，避免改变输出、所有权或存储语义。

新增与修改：

- `RuntimeContentRendererBackendLifecycle.h/.cpp` 的 `attachAfterScenePreparation(...)` 改为接收 `std::string_view rendererBackendKey`。
- `RuntimeEngineLifecycle.h/.cpp` 的 `attachRendererBackend(...)` 改为接收 `std::string_view rendererBackendKey`。
- `RuntimeRendererBackendAttachmentLifecycle.h/.cpp` 的 `attachToRendererSubsystem(...)` 改为接收 `std::string_view rendererBackendKey`。
- `RuntimeRendererBackendCatalog.h/.cpp` 的 `isRegisteredBackendKey(...)` 与 `resolveBackendSelection(...)` 改为接收 `std::string_view`。
- `RendererBackendRegistry.h/.cpp` 的 key 查询、默认 key、selection resolve 和 attachment desc helper 改为接收 `std::string_view`，在生成 `RendererBackendSelection` / `RendererBackendAttachmentDesc` 时仍写入 `std::string` 字段。
- `RuntimeApplicationConfig.h` 与 `RuntimeContentLifecycleConfig.h` 中的 `rendererBackendKey` 配置字段保持 `std::string`，不改变 profile/config 存储模型。
- 本轮没有新增或删除源文件，因此不需要修改 `text2.vcxproj` 或 `text2.vcxproj.filters`。

已完成验证：

- 静态检查确认 renderer backend key 传递路径不再使用 `const std::string&` 入参，配置 DTO 的持久化字段仍保留 `std::string`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 registry、catalog、attachment lifecycle、content renderer backend lifecycle、Engine lifecycle 和相关 startup/shutdown call sites。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 renderer backend key read-only boundary cleanup，不改变 backend registry selection、fallback default key、runtime/no-op backend attachment、renderer backend contract report、cleanup report 或 PBR pass。
- 后续建议继续选择 runtime content verification sub-lifecycle header include cleanup，或进入更小的 renderer backend catalog include surface cleanup；保持小切片降耦合。

### 2026-05-31 Renderer Backend Registry Types Header Extraction

本轮继续上一轮建议中的 renderer backend catalog include surface cleanup。`RendererBackendRegistry.h` 与 runtime backend catalog 只需要 registry metadata DTO，不应该为了 `RendererBackendRegistration`、`RendererBackendSelection` 或 `RendererBackendAttachmentDesc` 依赖完整 `RendererBackend` interface header。本切片把 registry metadata 类型拆到独立轻量头，保留 `RendererBackend.h` 只表达后端接口和 frame intent/result contract。

新增与修改：

- 新增 `engine/RendererBackendRegistryTypes.h`，集中定义 `RendererBackendAttachmentDesc`、`RendererBackendRegistration` 与 `RendererBackendSelection`。
- `RendererBackend.h` 移除 registry/attachment metadata DTO 定义，只保留 `RendererBackend` interface、`RendererFrameIntent` 与 `RendererFrameResult`。
- `RendererBackendRegistry.h` 改为 include `RendererBackendRegistryTypes.h`，不再为了 registry metadata 依赖完整 `RendererBackend.h`。
- `RuntimeRendererBackendCatalog.h` 移除对 `RendererBackend.h` 的 public include，只保留 registry header。
- `RuntimeRendererBackendFactory.h` 改为 forward declare `RendererBackend` 与 `RendererBackendSelection`，降低 factory header 对 engine renderer backend interface/registry header 的传递依赖。
- `RendererSubsystem.h` 与 `RendererSubsystemBackendSlot.h` 显式 include `RendererBackendRegistryTypes.h`，因为这些 header 的 public API/slot snapshot 直接使用 `RendererBackendAttachmentDesc`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `engine\RendererBackendRegistryTypes.h`，放入 `include\GLengine` 分组。
- 本轮使用一个只读 advisory subagent 审计 include surface、factory forward declaration、VS 项目注册和验证风险；subagent 没有改文件，主线程负责实现、验证和文档。

已完成验证：

- 静态检查确认 metadata DTO 只在 `RendererBackendRegistryTypes.h` 定义；`RendererBackendRegistry.h` 不再 include `RendererBackend.h`；`RuntimeRendererBackendCatalog.h` 不再 include `RendererBackend.h`；factory header 只保留前置声明。
- 静态检查确认 `RuntimeRendererBackendAttachmentLifecycle.cpp` 的 factory 返回值 call site 通过 `RendererSubsystem.h` 获得完整 `RendererBackend` 定义，不触发 `std::unique_ptr<RendererBackend>` incomplete type 析构问题。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 registry、catalog、factory、subsystem backend slot 和 renderer subsystem 头拆分。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 default backend 与 no-op backend 仍输出 `Runtime renderer backend contract stats`、`Runtime renderer backend contract cleanup stats`、`rendererBackendRegistryCount=2`、`runtime-frame-pipeline-adapter` 与 `test-noop-renderer-backend` registry key。

结论：

- 这是 renderer backend registry metadata header extraction，不改变 renderer backend selection、backend creation、attachment ownership、frame execution、cleanup report、verification contract 或 PBR pass。
- 后续建议继续选择 renderer backend catalog/factory implementation include cleanup，或回到 runtime content verification sub-lifecycle header include cleanup；保持 Engine runtime boundary 小步降耦合，不扩张 PBR 功能。

### 2026-05-31 Runtime Renderer Backend Keys Header Extraction

本轮继续上一轮建议中的 renderer backend catalog/factory implementation include cleanup。`RuntimeApplicationConfig.h` 只需要默认 renderer backend key，`RuntimeVerificationArgs.cpp` 只需要 no-op backend key，`RuntimeRendererBackendFactory.cpp` 只需要比较 selection key 来创建具体 backend；这些路径不需要通过 `RuntimeRendererBackendCatalog.h` 拉入 registry API。本切片把 backend key 常量抽成轻量 header，让 config、verification args 和 factory implementation 不再依赖 catalog/registry。

新增与修改：

- 新增 `RuntimeRendererBackendKeys.h`，集中提供 `runtimeFramePipelineBackendKey()`、`testNoOpBackendKey()` 与 `defaultBackendKey()` constexpr key helper。
- `RuntimeApplicationConfig.h` 改为 include `RuntimeRendererBackendKeys.h`，`rendererBackendKey` 默认值不再依赖 `RuntimeRendererBackendCatalog.h`。
- `RuntimeVerificationArgs.cpp` 改为使用 `RuntimeRendererBackendKeys::testNoOpBackendKey()`，不再为了 no-op mode key include catalog。
- `RuntimeRendererBackendFactory.cpp` 改为使用 `RuntimeRendererBackendKeys` 比较 concrete backend key，不再 include catalog；并显式 include `RendererBackendRegistryTypes.h`，因为 implementation 读取 `RendererBackendSelection` 字段。
- `RuntimeRendererBackendCatalog.cpp` 改为使用同一个 key header 填充 registry registration 与 default fallback，保持 catalog public API 兼容。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `application\RuntimeRendererBackendKeys.h`，放入 `include\Application` 分组。

已完成验证：

- 静态检查确认 `RuntimeApplicationConfig.h`、`RuntimeVerificationArgs.cpp` 与 `RuntimeRendererBackendFactory.cpp` 不再引用 `RuntimeRendererBackendCatalog`，catalog include 只保留在 attachment lifecycle 与 catalog implementation。
- 第一次 focused verification 构建失败，原因是 factory `.cpp` 移除 catalog include 后只剩 header forward declaration，读取 `RendererBackendSelection` 字段需要完整定义；已通过在 `RuntimeRendererBackendFactory.cpp` 显式 include `RendererBackendRegistryTypes.h` 修复。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：修复后构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 default backend 与 no-op backend 仍输出 runtime/no-op `rendererBackendKey`、`rendererBackendRegistryKey`、`rendererBackendRegistryCount=2` 与 cleanup contract 行。

结论：

- 这是 renderer backend key constants header extraction，不改变 backend registry selection、runtime/no-op backend creation、attachment ownership、renderer backend contract report、cleanup report 或 PBR pass。
- 后续建议继续选择 runtime content verification sub-lifecycle header include cleanup，或继续收敛 application config header 的 remaining heavy config includes；保持 Engine runtime boundary 小步降耦合，不扩张 PBR 功能。

### 2026-05-31 Runtime Window Lifecycle Types Header Extraction

本轮继续收敛 `RuntimeApplicationConfig.h` 的 remaining heavy includes。`RuntimeApplicationShellConfig` 只需要按值持有 `RuntimeWindowConfig`，而旧的 `RuntimeWindowLifecycle.h` 同时定义 window config/snapshot/callback DTO 和 window lifecycle 行为，并且 public include `AppRuntimeContext.h`。这会让 application config header 为一个 window config 字段间接拉入完整 runtime context。本切片把 window lifecycle DTO 拆到轻量 types header，让 config/header-only snapshot consumers 不再依赖 window lifecycle implementation boundary。

新增与修改：

- 新增 `RuntimeWindowLifecycleTypes.h`，集中定义 `RuntimeWindowConfig`、`RuntimeWindowSnapshot` 与 `RuntimeWindowCallbackContext`，并只 forward declare `GLFWwindow` 与 `GLframework::AppRuntimeContext`。
- `RuntimeWindowLifecycle.h` 移除 `AppRuntimeContext.h` include，改为 include `RuntimeWindowLifecycleTypes.h`，只保留 window lifecycle 行为入口。
- `RuntimeWindowLifecycle.cpp` 显式 include `AppRuntimeContext.h`，因为 implementation 需要访问 camera/light 和 render resource 字段。
- `RuntimeApplicationConfig.h` 改为 include `RuntimeWindowLifecycleTypes.h`，不再为了 `RuntimeWindowConfig` 依赖完整 `RuntimeWindowLifecycle.h`。
- `RuntimeApplicationWindowStartupLifecycle.h` 改为 include `RuntimeWindowLifecycleTypes.h`，保留 `RuntimeWindowSnapshot` 返回值完整定义；对应 `.cpp` 显式 include `RuntimeWindowLifecycle.h` 调用 initialize/capture。
- `RuntimeApplicationContentStartupLifecycle.cpp` 与 `RuntimeApplicationEditorStartupLifecycle.cpp` 改为 include `RuntimeWindowLifecycleTypes.h`，因为它们只读取 snapshot 字段，不调用 window lifecycle 行为。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `application\RuntimeWindowLifecycleTypes.h`，放入 `include\Application` 分组。

已完成验证：

- 静态检查确认 `RuntimeApplicationConfig.h` 与只读 snapshot consumers 使用 `RuntimeWindowLifecycleTypes.h`；`AppRuntimeContext.h` 只保留在 `RuntimeWindowLifecycle.cpp`，完整 runtime context 依赖局部化到 implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 window startup、content/editor startup、frame run、shutdown destroy 和 window lifecycle implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime window lifecycle DTO header extraction，不改变 window initialization、resize/input callbacks、window snapshot capture、startup/content/editor/frame/shutdown order、renderer backend contract 或 PBR pass。
- 后续建议继续选择 Runtime Frame Clock Config Header extraction，移除 `RuntimeApplicationConfig.h` 对 `RuntimeFrameClock.h` / `<chrono>` 的间接依赖；或继续 runtime content verification sub-lifecycle header cleanup。

### 2026-05-31 Runtime Frame Clock Config Header Extraction

本轮继续收敛 `RuntimeApplicationConfig.h` 的 remaining heavy includes。`RuntimeApplicationShellConfig` 只需要按值持有 `RuntimeFrameClockConfig`，不需要完整 `RuntimeFrameClock` 行为类，也不应该为了一个 frame clock 配置字段间接拉入 `<chrono>`。本切片把 frame clock config DTO 拆到轻量 types header，让 application config 只依赖数据定义。

新增与修改：

- 新增 `RuntimeFrameClockTypes.h`，集中定义 `RuntimeFrameClockConfig`。
- `RuntimeFrameClock.h` 改为 include `RuntimeFrameClockTypes.h`，只保留 `RuntimeFrameClock` 行为类和 `std::chrono::steady_clock` 存储。
- `RuntimeApplicationConfig.h` 改为 include `RuntimeFrameClockTypes.h`，不再为了 `RuntimeFrameClockConfig` 依赖完整 `RuntimeFrameClock.h` 或 `<chrono>`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `application\RuntimeFrameClockTypes.h`，放入 `include\Application` 分组。

已完成验证：

- 静态检查确认 `RuntimeApplicationConfig.h` 只 include `RuntimeFrameClockTypes.h`；`<chrono>` 只保留在 `RuntimeFrameClock.h`；`RuntimeFrameLifecycleTypes.h` 仍因为按值持有 `RuntimeFrameClock` 而依赖完整 clock 行为头。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 `RuntimeFrameClock.cpp`、frame lifecycle 和 backend contract 关键路径。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime frame clock config DTO header extraction，不改变 fixed delta 规则、verification frame delta、frame loop timing、renderer backend contract、cleanup report 或 PBR pass。
- 后续建议继续选择 Runtime Frame Lifecycle Config/State Header Split，让只需要 `RuntimeFrameLifecycleConfig` 的路径不再被 `RuntimeFrameLifecycleState` 的 `RuntimeFrameClock` 值成员拖入 `<chrono>`；或继续 runtime content verification sub-lifecycle header include cleanup。

### 2026-05-31 Runtime Frame Lifecycle Config/State Header Split

本轮继续上一轮的 frame lifecycle include boundary cleanup。`RuntimeFrameLifecycleTypes.h` 同时定义 `RuntimeFrameLifecycleConfig` 与 `RuntimeFrameLifecycleState`，而 state 按值持有 `RuntimeFrameClock`，会通过 `RuntimeFrameClock.h` 拉入 `<chrono>`。只生成或传递 frame lifecycle config 的路径不应该因此依赖 state/clock 行为。本切片把 frame lifecycle config 与 state 拆到独立头，旧 types 头保留为兼容聚合入口。

新增与修改：

- 新增 `RuntimeFrameLifecycleConfig.h`，集中定义 `RuntimeFrameLifecycleConfig`，只依赖 `RuntimeFrameClockTypes.h` 与 `RuntimeVerificationConfig.h`。
- 新增 `RuntimeFrameLifecycleState.h`，集中定义 `RuntimeFrameLifecycleState`，只在按值持有 `RuntimeFrameClock` 的 state 路径引入完整 `RuntimeFrameClock.h`。
- `RuntimeFrameLifecycleTypes.h` 改为兼容聚合头，只 include `RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h`。
- `RuntimeApplicationConfigPolicy.cpp` 改为 include `RuntimeFrameLifecycleConfig.h`，不再为了构造 frame lifecycle config 依赖 state/clock 行为头。
- `RuntimeApplicationState.h` 改为 include `RuntimeFrameLifecycleState.h`，因为 application state 按值持有 frame lifecycle state。
- `RuntimeFrameLifecycle.cpp` 显式 include config/state 具体头。
- `RuntimeApplicationFrameContinueBridge.cpp` 与 `RuntimeApplicationFrameRunBridge.cpp` 显式 include `RuntimeFrameLifecycleConfig.h`，因为它们按值接收 `RuntimeApplicationConfigPolicy::makeFrameLifecycleConfig(...)` 的返回对象。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `application\RuntimeFrameLifecycleConfig.h` 与 `application\RuntimeFrameLifecycleState.h`，放入 `include\Application` 分组。

已完成验证：

- 静态检查确认 `RuntimeApplicationConfigPolicy.cpp` 只 include config 头；`RuntimeApplicationState.h` 才 include state 头；`RuntimeFrameLifecycleState.h` 是 frame lifecycle state 路径中唯一引入完整 `RuntimeFrameClock.h` 的新头；`RuntimeFrameLifecycleTypes.h` 只作为聚合兼容头保留。
- 第一次 focused verification 构建失败，原因是 frame continue/run bridge 调用 `makeFrameLifecycleConfig(...)` 并按值接收返回对象，调用点需要完整 `RuntimeFrameLifecycleConfig` 定义；已通过在两个 bridge `.cpp` 显式 include `RuntimeFrameLifecycleConfig.h` 修复。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：修复后构建通过；三条 focused verification mode 全部通过；编译覆盖 frame continue/run bridge、frame startup 和 frame lifecycle implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime frame lifecycle config/state header split，不改变 fixed-delta verification timing、frame continue condition、frame runner dispatch、editor callback gating、verification capture timing、renderer backend contract、cleanup report 或 PBR pass。
- 后续建议继续选择 Runtime Application State Forward Boundary Cleanup，继续降低 `RuntimeApplicationState.h` 的 public include 面；或回到 runtime content verification sub-lifecycle header include cleanup。

### 2026-05-31 Runtime Editor Lifecycle Config/State Header Split

本轮继续 Runtime Application State Forward Boundary Cleanup。`RuntimeApplicationState.h` 需要按值持有 `RuntimeEditorLifecycleState`，但旧的 `RuntimeEditorLifecycle.h` 同时定义 editor config/state 和 editor lifecycle 行为入口，并且为了 state 直接 include `EditorPanels.h`。这让 application state 为一个 editor state 字段间接依赖 editor lifecycle 行为头和 frame callback 返回类型。本切片把 editor lifecycle config/state 拆到独立头，让 application state 只依赖 editor state 数据定义。

新增与修改：

- 新增 `RuntimeEditorLifecycleConfig.h`，集中定义 `RuntimeEditorLifecycleConfig`，只 forward declare `GLFWwindow`。
- 新增 `RuntimeEditorLifecycleState.h`，集中定义 `RuntimeEditorLifecycleState`，把 `EditorPanels.h` 依赖局部化到真正按值持有 selection/transaction state 的路径。
- `RuntimeEditorLifecycle.h` 移除 config/state 定义与 `EditorPanels.h` include，只 forward declare `RuntimeEditorLifecycleConfig` / `RuntimeEditorLifecycleState` 并保留 editor lifecycle 行为入口。
- `RuntimeEditorLifecycle.cpp` 显式 include config/state 具体头。
- `RuntimeApplicationState.h` 改为 include `RuntimeEditorLifecycleState.h`，不再为了 `RuntimeEditorLifecycleState` 依赖完整 editor lifecycle 行为头。
- `RuntimeApplicationConfigPolicy.cpp` 改为 include `RuntimeEditorLifecycleConfig.h`，不再为了构造 editor lifecycle config 依赖 editor lifecycle 行为头。
- `RuntimeApplicationEditorStartupLifecycle.cpp` 与 `RuntimeApplicationFrameEditorCallbackBridge.cpp` 显式 include `RuntimeEditorLifecycleConfig.h`，因为它们按值消费 `makeEditorLifecycleConfig(...)` 的返回对象。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `application\RuntimeEditorLifecycleConfig.h` 与 `application\RuntimeEditorLifecycleState.h`，放入 `include\Application` 分组。

已完成验证：

- 静态检查确认 `RuntimeApplicationState.h` 只 include `RuntimeEditorLifecycleState.h`；`RuntimeApplicationConfigPolicy.cpp` 只 include `RuntimeEditorLifecycleConfig.h`；真正调用 editor lifecycle 行为的 startup/callback bridge `.cpp` 保留 `RuntimeEditorLifecycle.h`。
- 静态检查确认 `EditorPanels.h` 不再通过 `RuntimeEditorLifecycle.h` 传递，而是只由 `RuntimeEditorLifecycleState.h` 承载。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 application state/editor startup/frame editor callback bridge/editor lifecycle implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime editor lifecycle config/state header split，不改变 GUI 初始化、editor panel callback、selection/transaction state、frame callback gating、renderer backend contract、cleanup report 或 PBR pass。
- 后续建议继续选择 Runtime Application State Engine/Context Include Boundary Cleanup，继续审计 `RuntimeApplicationState.h` 中仍按值持有的 Engine、runtime context、Engine lifecycle state 和 legacy experiment runner；或回到 runtime content verification sub-lifecycle header include cleanup。

### 2026-05-31 Runtime Application State Engine Owner Boundary Cleanup

本轮继续 Runtime Application State Engine/Context Include Boundary Cleanup 的第一步。`RuntimeApplicationState.h` 仍按值持有 `GLengine::Engine`，因此该 state 头必须 include 完整 `Engine.h`，并间接把 Engine context、World、subsystem 等实现细节暴露给所有 application state 消费者。本切片只把 Engine owner 改为 state 内部 `std::unique_ptr`，通过 out-of-line 构造/析构和 `engine()` accessor 保持 Engine 生命周期由 application state 拥有，同时把完整 `Engine.h` 依赖局部化到 `.cpp`。

新增与修改：

- 新增 `RuntimeApplicationState.cpp`，负责 include 完整 `Engine.h`、构造 `GLengine::Engine`、默认析构 state，并提供 `RuntimeApplicationState::engine()` / `const engine()` accessor。
- `RuntimeApplicationState.h` 移除 `../engine/Engine.h` include，改为 forward declare `GLengine::Engine`，并把 Engine owner 存成第一个私有 `std::unique_ptr<GLengine::Engine>` 成员，保持 Engine 仍在其他 state 字段之后析构。
- `RuntimeApplicationEngineStartupLifecycle.cpp`、`RuntimeApplicationContentStartupLifecycle.cpp`、`RuntimeApplicationFrameRunBridge.cpp`、`RuntimeApplicationShutdownEngineBridge.cpp` 与 `RuntimeApplicationShutdownCleanupBridge.cpp` 从 `state.engine` 改为 `state.engine()`，不改变初始化、内容准备、frame run、shutdown 或 cleanup report 的执行顺序。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `application\RuntimeApplicationState.cpp`，放入 `cppfile\Application` 分组。

已完成验证：

- 静态检查确认 `RuntimeApplicationState.h` 不再 include `Engine.h`；完整 `Engine.h` 只出现在 `RuntimeApplicationState.cpp` 和原有 Engine 实现/工程注册路径；旧 `state.engine` 字段访问已替换为 `state.engine()`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 `RuntimeApplicationState.cpp`、engine startup、content startup、frame run、shutdown cleanup 和 shutdown engine bridge。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 runtime context Engine attachment、Engine lifecycle snapshot、renderer backend contract、cleanup、fixed delta、minimal scene 与 editor-create 关键行仍然存在。

结论：

- 这是 application runtime state Engine owner boundary cleanup，不改变 Engine 初始化/关闭、runtime context attach/detach、Engine-owned subsystem、frame runner、legacy experiment tick、renderer backend contract、cleanup report 或 PBR pass。
- 后续建议继续 Runtime Application State Context/Legacy Owner Boundary Cleanup，继续审计 `RuntimeApplicationState.h` 中仍按值持有的 `AppRuntimeContext` 和 `LegacyExperimentRunner`；或切回 runtime content verification sub-lifecycle header include cleanup。

### 2026-05-31 Runtime Application State Legacy Runner Owner Boundary Cleanup

本轮继续上一轮的 application state owner boundary cleanup。`RuntimeApplicationState.h` 仍按值持有 `LegacyExperimentRunner`，而 `LegacyExperimentRunner.h` 是当前 application state 头里最重的 legacy implementation dependency：它会传递 renderer、scene、object、geometry、texture、materials、mesh、assimp loader、tools 和 light 等旧实验实现细节。本切片只把 legacy runner owner 改为 state 内部 `std::unique_ptr`，通过 out-of-line 构造/析构和 `legacyExperiments()` accessor 保持 application state ownership，同时把完整 legacy 实验实现头局部化到 `RuntimeApplicationState.cpp`。

新增与修改：

- `RuntimeApplicationState.h` 移除 `../tools/legacyExperiments/LegacyExperimentRunner.h` include，改为 forward declare `GL_EXPERIMENTS::LegacyExperimentRunner`。
- `RuntimeApplicationState.h` 把 legacy runner owner 存成私有 `std::unique_ptr<GL_EXPERIMENTS::LegacyExperimentRunner>`，并提供 `legacyExperiments()` / `const legacyExperiments()` accessor。
- `RuntimeApplicationState.cpp` 显式 include `LegacyExperimentRunner.h`，负责构造 legacy runner，并实现 accessor。
- `RuntimeApplicationContentStartupLifecycle.cpp` 与 `RuntimeApplicationFrameRunBridge.cpp` 从 `state.legacyExperiments` 改为 `state.legacyExperiments()`，content prepare 和 frame update 仍向 content/frame lifecycle 传入同一个 legacy runner。

已完成验证：

- 静态检查确认 `RuntimeApplicationState.h` 不再 include `LegacyExperimentRunner.h`；完整 legacy implementation header 只保留在 `RuntimeApplicationState.cpp` 和 `RuntimeLegacyExperimentLifecycle.cpp`。
- 静态检查确认旧 `state.legacyExperiments` 字段式传参已替换为 `state.legacyExperiments()`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 state 构造、content startup、frame run 和 legacy mirror verification path。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 `legacyWorldMirror`、runtime context Engine attachment、Engine lifecycle snapshot、renderer backend contract、cleanup、fixed delta、minimal scene 与 editor-create 关键行仍然存在。

结论：

- 这是 application runtime state legacy runner owner boundary cleanup，不改变 legacy experiment prepare/update、content lifecycle、frame runner、Engine lifecycle、renderer backend contract、cleanup report 或 PBR pass。
- 后续建议继续 Runtime Application State Context Header Boundary Cleanup，审计 `RuntimeApplicationState.h` 中仍按值持有的 `AppRuntimeContext`；或切回 runtime content verification sub-lifecycle header include cleanup。

### 2026-05-31 Runtime Application State Runtime Context Owner Boundary Cleanup

本轮完成上一轮建议的 `AppRuntimeContext` owner boundary。`RuntimeApplicationState.h` 之前仍按值持有 `AppRuntimeContext`，因此所有包含 application state 的调用点都会传递 render resources、camera/light state、engine attachment state 和 runtime profile state 的完整聚合头。本切片只把 runtime context 的构造/析构移到 `RuntimeApplicationState.cpp`，通过 `runtime()` accessor 保持 application state ownership，并保留原有成员销毁顺序：Engine 仍最后析构，runtime context 仍位于 engine lifecycle state 之后、editor lifecycle state 之前。

新增与修改：

- `RuntimeApplicationState.h` 移除 `AppRuntimeContext.h` include，改为 forward declare `GLframework::AppRuntimeContext`。
- `RuntimeApplicationState.h` 把 runtime context owner 存成私有 `std::unique_ptr<GLframework::AppRuntimeContext>`，并提供 `runtime()` / `const runtime()` accessor。
- `RuntimeApplicationState.cpp` 显式 include `AppRuntimeContext.h`，负责构造 runtime context，并实现 accessor。
- `RuntimeApplicationContentStartupLifecycle.cpp`、`RuntimeApplicationEngineStartupLifecycle.cpp`、`RuntimeApplicationFrameEditorCallbackBridge.cpp`、`RuntimeApplicationFrameRunBridge.cpp`、`RuntimeApplicationShutdownCleanupBridge.cpp`、`RuntimeApplicationShutdownEngineBridge.cpp` 与 `RuntimeApplicationWindowStartupLifecycle.cpp` 从 `state.runtime` / `&state.runtime` 改为 `state.runtime()` / `&state.runtime()`，不改变 window callbacks、Engine attach/detach、content startup、editor callbacks、frame run 或 shutdown cleanup 顺序。
- 启动并关闭只读 sidecar `Newton`：它确认本切片方向、8 个字段式 runtime context 调用点、无需新增 project registration，并报告无文件编辑。

已完成验证：

- 静态检查确认 `RuntimeApplicationState.h` 不再 include `AppRuntimeContext.h`；完整 runtime context 依赖局部化到 `RuntimeApplicationState.cpp`。
- 静态检查确认旧 `state.runtime` / `&state.runtime` 字段式访问已替换为 `state.runtime()` / `&state.runtime()`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 `RuntimeApplicationState.cpp` 和所有 runtime context 调用桥接点。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 runtime context Engine attach/detach、renderer backend contract、cleanup、fixed delta、legacy mirror、minimal scene 与 editor-create 关键行仍然存在。

结论：

- 这是 application runtime state context owner boundary cleanup，不改变 runtime context 数据模型、Engine lifecycle、window callback context、content preparation、editor frame callback、frame runner、legacy experiment update、renderer backend contract、cleanup report 或 PBR pass。
- 后续建议继续 Runtime Application State Lifecycle State Include Boundary Audit，评估 `RuntimeApplicationState.h` 中 remaining engine/editor/frame lifecycle state 头依赖；或切回 runtime content verification sub-lifecycle header include cleanup。

### 2026-05-31 Runtime Application State Editor Lifecycle State Owner Boundary Cleanup

本轮继续上一轮的 lifecycle state include boundary audit。`RuntimeEngineLifecycleTypes.h` 已经是轻量 forward-declaration DTO，当前不是优先问题；`RuntimeEditorLifecycleState.h` 仍会把 `EditorPanels.h` 传入 `RuntimeApplicationState.h`，而实际只有 frame editor callback bridge 需要访问 editor lifecycle state。本切片把 editor lifecycle state 改为 application state 内部 owner，并通过 `editorLifecycle()` accessor 暴露给唯一调用点。

新增与修改：

- `RuntimeApplicationState.h` 移除 `RuntimeEditorLifecycleState.h` include，改为 forward declare `RuntimeEditorLifecycleState`。
- `RuntimeApplicationState.h` 把 editor lifecycle state owner 存成私有 `std::unique_ptr<RuntimeEditorLifecycleState>`，并提供 `editorLifecycle()` / `const editorLifecycle()` accessor。
- `RuntimeApplicationState.cpp` 显式 include `RuntimeEditorLifecycleState.h`，负责构造 editor lifecycle state，并实现 accessor。
- `RuntimeApplicationFrameEditorCallbackBridge.cpp` 从 `state.editorLifecycle` 改为 `state.editorLifecycle()`，editor frame callback construction 行为不变。

已完成验证：

- 静态检查确认 `RuntimeApplicationState.h` 不再 include `RuntimeEditorLifecycleState.h` 或传递 `EditorPanels.h`；完整 editor state 依赖局部化到 `RuntimeApplicationState.cpp` 和 editor lifecycle implementation 路径。
- 静态检查确认旧 `state.editorLifecycle` 字段式访问已替换为 `state.editorLifecycle()`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 state 构造和 frame editor callback bridge。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 runtime context Engine attach/detach、renderer backend contract、cleanup、fixed delta、legacy mirror、minimal scene 与 editor-create 关键行仍然存在。

结论：

- 这是 application runtime state editor lifecycle state owner boundary cleanup，不改变 GUI 初始化、editor panel callback、selection/transaction state 数据模型、frame callback gating、renderer backend contract、cleanup report 或 PBR pass。
- 后续建议继续 Runtime Application State Frame Lifecycle State Owner Boundary Cleanup，移出 `RuntimeFrameLifecycleState.h` 对 `RuntimeFrameClock.h` / `<chrono>` 的传递依赖；或切回 runtime content verification sub-lifecycle header include cleanup。

### 2026-05-31 Runtime Application State Frame Lifecycle State Owner Boundary Cleanup

本轮继续 `RuntimeApplicationState` 的剩余 include surface cleanup。`RuntimeFrameLifecycleState.h` 按值持有 `RuntimeFrameClock`，会通过 `RuntimeFrameClock.h` 把 `<chrono>` 传入 `RuntimeApplicationState.h`，而 application state 头本身只需要拥有 frame lifecycle state，不需要暴露 clock 行为细节。本切片把 frame lifecycle state 改为 application state 内部 owner，并通过 `frameLifecycle()` accessor 暴露给 frame startup/continue/run bridge。

新增与修改：

- `RuntimeApplicationState.h` 移除 `RuntimeFrameLifecycleState.h` include，改为 forward declare `RuntimeFrameLifecycleState`。
- `RuntimeApplicationState.h` 把 frame lifecycle state owner 存成私有 `std::unique_ptr<RuntimeFrameLifecycleState>`，并提供 `frameLifecycle()` / `const frameLifecycle()` accessor。
- `RuntimeApplicationState.cpp` 显式 include `RuntimeFrameLifecycleState.h`，负责构造 frame lifecycle state，并实现 accessor。
- `RuntimeApplicationFrameStartupLifecycle.cpp`、`RuntimeApplicationFrameContinueBridge.cpp` 与 `RuntimeApplicationFrameRunBridge.cpp` 从 `state.frameLifecycle` 改为 `state.frameLifecycle()`，frame reset、continue check 和 frame run 行为不变。

已完成验证：

- 静态检查确认 `RuntimeApplicationState.h` 不再 include `RuntimeFrameLifecycleState.h`，也不再通过该头传递 `RuntimeFrameClock.h` / `<chrono>`。
- 静态检查确认旧 `state.frameLifecycle` 字段式访问已替换为 `state.frameLifecycle()`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 frame startup/continue/run bridge 和 state 构造路径。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 runtime context Engine attach/detach、renderer backend contract、cleanup、fixed delta、legacy mirror、minimal scene 与 editor-create 关键行仍然存在。

结论：

- 这是 application runtime state frame lifecycle state owner boundary cleanup，不改变 frame reset、max-frame continue、frame clock fixed delta、frame runner dispatch、verification capture timing、renderer backend contract、cleanup report 或 PBR pass。
- 后续建议继续 Runtime Application State Engine Lifecycle State Accessor Boundary Cleanup，评估是否把仍公开的 `engineLifecycle` 也改为 accessor；或切回 runtime content verification sub-lifecycle header include cleanup。

### 2026-05-31 Runtime Application State Engine Lifecycle State Accessor Boundary Cleanup

本轮继续 `RuntimeApplicationState` 的最后一个公开 lifecycle state 字段清理。`RuntimeEngineLifecycleTypes.h` 本身已经是轻量 DTO 头，但 `RuntimeApplicationState.h` 不需要暴露 engine lifecycle state 的字段；真正需要完整字段的只有 frame run 中读取 `rendererSubsystem` 和 shutdown cleanup refs 的 by-value implementation。本切片把 engine lifecycle state 改为 application state 内部 owner，并通过 `engineLifecycle()` accessor 暴露给 startup/content/frame/shutdown bridge。

新增与修改：

- `RuntimeApplicationState.h` 移除 `RuntimeEngineLifecycleTypes.h` include，改为 forward declare `RuntimeEngineLifecycleState`。
- `RuntimeApplicationState.h` 把 engine lifecycle state owner 存成私有 `std::unique_ptr<RuntimeEngineLifecycleState>`，并提供 `engineLifecycle()` / `const engineLifecycle()` accessor。
- `RuntimeApplicationState.cpp` 显式 include `RuntimeEngineLifecycleTypes.h`，负责构造 engine lifecycle state，并实现 accessor。
- `RuntimeApplicationContentStartupLifecycle.cpp`、`RuntimeApplicationEngineStartupLifecycle.cpp`、`RuntimeApplicationFrameRunBridge.cpp` 与 `RuntimeApplicationShutdownEngineBridge.cpp` 从 `state.engineLifecycle` 改为 `state.engineLifecycle()`。
- `RuntimeApplicationFrameRunBridge.cpp` 显式 include `RuntimeEngineLifecycleTypes.h`，因为该文件读取 `rendererSubsystem` 字段。
- `RuntimeApplicationShutdownEngineBridge.cpp` 显式 include `RuntimeEngineLifecycleTypes.h`，因为该文件按值定义/返回 `RuntimeEngineLifecycleCleanupRefs`。

已完成验证：

- 静态检查确认 `RuntimeApplicationState.h` 不再 include `RuntimeEngineLifecycleTypes.h`，只 forward declare `RuntimeEngineLifecycleState`。
- 静态检查确认旧 `state.engineLifecycle` 字段式访问已替换为 `state.engineLifecycle()`。
- 首次 focused build 暴露 `RuntimeApplicationShutdownEngineBridge.cpp` 的真实完整类型依赖：该文件定义 by-value cleanup refs 返回值，不能再依赖 `RuntimeApplicationState.h` 间接 include；已通过显式 include `RuntimeEngineLifecycleTypes.h` 修正。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：修正后构建通过；三条 focused verification mode 全部通过；编译覆盖 `RuntimeApplicationState.cpp`、content startup、engine startup、frame run 和 shutdown engine bridge。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 runtime context Engine attach/detach、Engine lifecycle snapshot、renderer backend contract、cleanup、fixed delta、legacy mirror、minimal scene 与 editor-create 关键行仍然存在。

结论：

- 这是 application runtime state engine lifecycle state accessor boundary cleanup，不改变 Engine initialization、subsystem capture、renderer backend attachment、frame renderer subsystem handoff、shutdown cleanup refs、cleanup report 或 PBR pass。
- 后续建议继续 runtime content verification sub-lifecycle header include cleanup，或做 RuntimeApplicationState accessor call-site include audit；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime Content Verification Sub-Lifecycle Facade Dependency Cleanup

本轮执行上一轮建议的 runtime content verification sub-lifecycle cleanup。复查后确认 `RuntimeContentVerificationLifecycle.h` 已经是干净的 forward declaration boundary：它只前置声明 `AppRuntimeContext` 与 `RuntimeVerificationConfig`，没有传递 `RuntimeProfileLoader`、`RuntimeVerificationLifecycle` 或完整 config/context 依赖。因此本轮不改 public header，而是清理 implementation 里对 generic `RuntimeVerificationLifecycle` facade 的直接依赖，让 content verification stage 直接委托更小的 verification sub-lifecycle 模块。

新增与修改：

- `RuntimeContentVerificationLifecycle.cpp` 移除 `RuntimeVerificationLifecycle.h` include。
- `RuntimeContentVerificationLifecycle.cpp` 新增 `RuntimeVerificationStartupProfileLifecycle.h` 与 `RuntimeVerificationPreparedSceneLifecycle.h` include。
- `RuntimeContentVerificationLifecycle::loadStartupProfiles(...)` 保持先 `RuntimeProfileLoader::loadAll(context)`，再改为调用 `RuntimeVerificationStartupProfileLifecycle::applyStartupProfile(context, config)`。
- `RuntimeContentVerificationLifecycle::reportPreparedScene(...)` 改为调用 `RuntimeVerificationPreparedSceneLifecycle::reportPreparedScene(context, config)`。

已完成验证：

- 静态检查确认 `RuntimeContentVerificationLifecycle.h` 仍只保留 forward declarations。
- 静态检查确认 `RuntimeContentVerificationLifecycle.cpp` 不再依赖 `RuntimeVerificationLifecycle` facade，完整依赖只剩 profile loader、startup profile sub-lifecycle 和 prepared-scene sub-lifecycle。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 `RuntimeContentVerificationLifecycle.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 PBR scene stats、Engine World minimal/package、legacy mirror、runtime Engine attach/detach、renderer backend contract、cleanup 和 fixed delta 关键输出仍然存在。

结论：

- 这是 runtime content verification sub-lifecycle facade dependency cleanup，不改变 runtime profile load 顺序、startup profile application、prepared-scene probes/report order、renderer backend contract、cleanup report 或 PBR pass。
- 后续建议继续 RuntimeApplicationState accessor call-site include audit，或做 RuntimeVerificationLifecycle remaining facade call-site audit；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime Application Shutdown Verification Bridge Cleanup Lifecycle Direct Dependency Cleanup

本轮继续上一轮建议的 `RuntimeVerificationLifecycle` remaining facade call-site audit。复查后确认 shutdown verification bridge 只负责 cleanup 阶段的 renderer subsystem cleanup report 与 Engine cleanup report，不需要再经过 generic `RuntimeVerificationLifecycle` facade；更窄的依赖应该直接指向 `RuntimeVerificationCleanupLifecycle`。frame stop/capture 路径仍属于 frame lifecycle 运行时控制，本轮不混入该职责。

新增与修改：

- `RuntimeApplicationShutdownVerificationBridge.cpp` 移除 `RuntimeVerificationLifecycle.h` include，改为 include `RuntimeVerificationCleanupLifecycle.h`。
- `RuntimeApplicationShutdownVerificationBridge::reportRendererSubsystemCleanup(...)` 改为直接调用 `RuntimeVerificationCleanupLifecycle::reportRendererSubsystemCleanup(...)`。
- `RuntimeApplicationShutdownVerificationBridge::reportEngineCleanup(...)` 改为直接调用 `RuntimeVerificationCleanupLifecycle::reportEngineCleanup(...)`。
- `RuntimeApplicationShutdownVerificationBridge.h` public API 不变，shutdown cleanup bridge 的调用顺序不变。
- `/subagents` 已重新确认可用；本轮未启动 sidecar，因为写集集中且 verification 是主线关键路径，协作边界记录为 parent-owned、无 delegated write scope。

已完成验证：

- 静态检查确认 `RuntimeApplicationShutdownVerificationBridge.cpp` 不再依赖 `RuntimeVerificationLifecycle` facade。
- 静态检查确认剩余外部 `RuntimeVerificationLifecycle::` 调用只在 `RuntimeFrameLifecycle.cpp` 的 `shouldStopAfterFrames(...)` 与 `captureFrameIfNeeded(...)`，本轮刻意保留给后续 frame stop/capture facade audit。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 shutdown verification bridge cleanup report 路径。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 PBR scene stats、Engine World minimal/package、legacy mirror、runtime Engine attach/detach、renderer backend contract、cleanup 和 fixed delta 关键输出仍然存在。

结论：

- 这是 shutdown verification bridge cleanup lifecycle direct dependency cleanup，不改变 renderer subsystem cleanup report、Engine cleanup report、cleanup ordering、runtime context detach、renderer backend contract 或 PBR pass。
- 后续建议继续 RuntimeVerificationLifecycle frame stop/capture facade call-site audit，或做 RuntimeApplicationState accessor call-site include audit；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime Verification Lifecycle Facade Removal

本轮继续上一轮留下的 frame stop/capture facade call-site audit。复查后确认 `RuntimeVerificationLifecycle` 已经只是无状态转发 facade：startup profile、prepared-scene report、frame capture、cleanup report 和 stop policy 都已拆到更窄的 sub-lifecycle/policy 模块。最后两个外部调用点位于 `RuntimeFrameLifecycle.cpp`，分别对应 max-frame stop condition 与 capture-frame report。本切片先让 frame lifecycle 直接依赖更窄模块，再删除无调用的 generic verification facade。

新增与修改：

- `RuntimeFrameLifecycle.cpp` 移除 `RuntimeVerificationLifecycle.h` include，改为 include `RuntimeVerificationStopPolicy.h` 与 `RuntimeVerificationFrameCaptureLifecycle.h`。
- `RuntimeFrameLifecycle::shouldContinue(...)` 改为直接调用 `RuntimeVerificationStopPolicy::shouldStopAfterFrames(...)`。
- `RuntimeFrameLifecycle::runFrame(...)` 改为直接调用 `RuntimeVerificationFrameCaptureLifecycle::captureFrameIfNeeded(...)`。
- 删除无外部调用者的 `RuntimeVerificationLifecycle.h/.cpp` facade。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 移除 `RuntimeVerificationLifecycle.h/.cpp` 注册。
- 本轮没有启动新的 sidecar：写域集中在 frame lifecycle、dead facade 和 VS project registration，且验证为主线关键路径，协作文档记录为 parent-owned、无 delegated write scope。

已完成验证：

- 静态检查确认 `application`、`engine`、`tools`、`main.cpp`、`text2.vcxproj` 与 `text2.vcxproj.filters` 中不再存在 `RuntimeVerificationLifecycle` 引用。
- 静态检查确认 `RuntimeVerificationLifecycle.h/.cpp` 已从工作树移除。
- 静态检查确认 `RuntimeFrameLifecycle.cpp` 直接调用 `RuntimeVerificationStopPolicy` 与 `RuntimeVerificationFrameCaptureLifecycle`，对应 sub-lifecycle/policy 仍在工程和 filters 中注册。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 frame stop/capture direct dependency 与 project registration removal。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 PBR scene stats、Engine World minimal/package、legacy mirror、runtime Engine attach/detach、renderer backend contract、cleanup 和 fixed delta 关键输出仍然存在。

结论：

- 这是 runtime verification lifecycle facade removal，不改变 frame stop condition、capture timing、runtime report 输出、PBR renderer stats report、cleanup report、renderer backend contract 或 PBR pass。
- 后续建议继续 RuntimeApplicationState accessor call-site include audit，或做 RuntimeApplicationConfig/runtime config header include surface audit；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime Application Editor Startup State Parameter and Callback Binder Include Cleanup

本轮继续 `RuntimeApplicationState accessor call-site include audit`。复查后确认 `RuntimeApplicationState.h` 当前已经是 owner/accessor forward boundary：Engine、runtime context、editor/frame/engine lifecycle state 和 legacy runner 都通过 `std::unique_ptr` 与 accessor 暴露，真正需要完整 state 类型的是调用 accessor 或构造 state 的 bridge/implementation。第一批安全清理点集中在 editor startup 和 callback binder：editor startup 初始化不读取 state，callback binder 只捕获并转发 state/config 引用，不需要完整 config/state header。

新增与修改：

- `RuntimeApplicationEditorStartupLifecycle.h` 移除 `RuntimeApplicationState` forward declaration，`initializeEditor(...)` 不再接收 `RuntimeApplicationState&`。
- `RuntimeApplicationEditorStartupLifecycle.cpp` 同步移除未使用的 state 参数，仍只根据 shell config 与 window snapshot 组装 editor lifecycle config。
- `RuntimeApplicationStartupLifecycle.cpp` 调用改为 `RuntimeApplicationEditorStartupLifecycle::initializeEditor(config, window)`。
- `RuntimeApplicationCallbackBinder.cpp` 移除冗余 `RuntimeApplicationConfig.h` 与 `RuntimeApplicationState.h` include；该文件只在 lambda 中转发已由调用方提供的引用。
- 实际调用 `state.runtime()`、`state.engine()`、`state.engineLifecycle()`、`state.editorLifecycle()`、`state.frameLifecycle()` 或 `state.legacyExperiments()` 的 bridge 仍显式 include `RuntimeApplicationState.h`，本轮不做过度隐藏。
- 已按 `/subagents` 要求更新 `docs/subagents_coordination.md`，当前主线 source/docs 写集由 parent 拥有；只读 sidecar `Huygens` 正在审计下一步 `RuntimeApplicationConfig` include surface，不能改文件，结果只作为后续切片建议。

已完成验证：

- 静态检查确认 editor startup lifecycle 不再暴露或接收 `RuntimeApplicationState`。
- 静态检查确认 callback binder 不再 include 完整 `RuntimeApplicationConfig.h` / `RuntimeApplicationState.h`。
- 静态检查确认剩余 `RuntimeApplicationState.h` include 都位于构造 state 或实际调用 state accessor 的 application bridge/implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-editor-create,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 callback binder、editor startup lifecycle 与 startup lifecycle 调用点。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 runtime Engine attach/detach、renderer backend contract、cleanup、fixed delta、legacy mirror、minimal scene、editor-create 和 PBR scene stats 关键输出仍然存在。

结论：

- 这是 application runtime state accessor call-site include cleanup，不改变 editor startup config mapping、callback binding、window snapshot、runtime frame callbacks、renderer backend contract 或 PBR pass。
- 后续建议优先根据 `Huygens` 只读审计结果选择一个小型 `RuntimeApplicationConfig/runtime config header include surface audit` 切片；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime Application Config Include Surface Follow-up Cleanup

本轮按 `Huygens` 只读审计结果执行最小 config include surface follow-up。目标不是继续扩大架构范围，而是把已经确认安全的 config header 传递依赖再下沉一层：frame editor callback bridge implementation 不读 config 字段，只把 shell config 引用交给 config policy；runner public header 只声明 by-value API，不需要向所有入口调用方传递完整 `RuntimeApplicationConfig.h`。

新增与修改：

- `RuntimeApplicationFrameEditorCallbackBridge.cpp` 移除冗余 `RuntimeApplicationConfig.h` include，保留 `RuntimeApplicationConfigPolicy.h`、`RuntimeApplicationState.h`、editor lifecycle config、editor lifecycle 和 callback DTO 依赖。
- `RuntimeApplicationRunner.h` 改为 forward declare `RuntimeApplicationShellConfig`，public API 保持 `static int run(RuntimeApplicationShellConfig config)` 不变。
- `RuntimeApplicationRunner.cpp` 显式 include `RuntimeApplicationConfig.h`，因为 function definition 按值接收并移动 config，不能依赖 header 传递 include。
- `RuntimeApplicationEntry.cpp` 不需要变化：它通过 `RuntimeVerificationArgs.h` 获得 by-value config 构造入口，通过 runner header 看到 runner API。

已完成验证：

- 静态检查确认 `RuntimeApplicationFrameEditorCallbackBridge.cpp` 与 `RuntimeApplicationRunner.h` 不再 include `RuntimeApplicationConfig.h`。
- 静态检查确认 `RuntimeApplicationRunner.h` 只 forward declare `RuntimeApplicationShellConfig`，`RuntimeApplicationRunner.cpp` 是 runner 路径的完整 config include 所有者。
- 静态检查确认 frame editor callback bridge 仍通过 `RuntimeApplicationConfigPolicy::makeEditorLifecycleConfig(config, window)` 生成 editor lifecycle config，没有读取 shell config 字段。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 `RuntimeApplicationEntry.cpp`、`RuntimeApplicationFrameEditorCallbackBridge.cpp` 与 `RuntimeApplicationRunner.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime application config include surface cleanup，不改变 application entry、runner ownership、editor callback config mapping、frame callbacks、renderer backend contract 或 PBR pass。
- 后续建议继续沿 `RuntimeApplicationConfig` 做同类小型 include surface audit，但必须遵守 `Huygens` 已确认的边界：`RuntimeApplicationShell.h`、`RuntimeVerificationArgs.h`、实际读取 config 字段的 policy/implementation 和 aggregate config DTO 的完整 include 不应强行移除。

### 2026-05-31 Runtime Verification Args Public Header Config Forward Boundary Cleanup

本轮继续 `RuntimeApplicationConfig` include surface audit。复查直接 include 后确认 policy/window/shutdown verification/shell/runner implementation 都确实需要完整 shell config：它们要么读取字段，要么按值存储/定义函数。唯一可继续下沉的是 `RuntimeVerificationArgs.h`：public header 只声明 `makeShellConfigFromArguments(...)`，不构造、不析构、不访问 `RuntimeApplicationShellConfig` 字段；完整 config 应由参数解析 implementation 和实际调用表达式所在的 entry implementation 显式持有。

新增与修改：

- `RuntimeVerificationArgs.h` 移除 `RuntimeApplicationConfig.h` include，改为 forward declare `RuntimeApplicationShellConfig`。
- `RuntimeVerificationArgs.cpp` 显式 include `RuntimeApplicationConfig.h`，因为这里定义 `makeShellConfigFromArguments(...)`、构造 config 并写入 verification/window/backend 字段。
- `RuntimeApplicationEntry.cpp` 显式 include `RuntimeApplicationConfig.h`，因为它把 `makeShellConfigFromArguments(...)` 的 by-value 返回值直接传给 `RuntimeApplicationRunner::run(...)`。
- `RuntimePBRVerificationArgs.h` 继续作为兼容 header include `RuntimeVerificationArgs.h`，但不再通过它传递完整 application config header。

已完成验证：

- 静态检查确认 `RuntimeVerificationArgs.h` 与 `RuntimePBRVerificationArgs.h` 不再 include `RuntimeApplicationConfig.h`。
- 静态检查确认 `RuntimeVerificationArgs.cpp` 与 `RuntimeApplicationEntry.cpp` 显式持有完整 config include，覆盖 by-value 构造/返回/调用所需完整类型。
- 静态检查确认剩余 `RuntimeApplicationConfig.h` 直接 include 只保留在 VS project/filter 注册、policy implementation、entry implementation、runner implementation、shell by-value owner、shutdown verification implementation、window startup implementation、content config policy implementation 和 verification args implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过；编译覆盖 `RuntimeApplicationEntry.cpp`、`RuntimePBRVerificationArgs.cpp` 与 `RuntimeVerificationArgs.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime verification args public header config forward boundary cleanup，不改变 CLI argument parsing、verification profile defaults、application entry、runner ownership、renderer backend selection、renderer backend contract 或 PBR pass。
- 后续建议如果继续 `RuntimeApplicationConfig` include audit，应优先停在当前边界；剩余直接 include 基本都有 by-value storage/definition 或字段读取依据。下一步更适合转向另一个 header surface，例如 runtime verification config/public args split 或 application startup/frame/shutdown bridge 的剩余 implementation include audit。

### 2026-05-31 Runtime PBR Verification Args Compatibility Facade Removal

本轮继续 runtime verification config/public args split。复查后确认 `RuntimePBRVerificationArgs.h/.cpp` 已经只是兼容空壳：header 只 include `RuntimeVerificationArgs.h`，cpp 只 include 自身并写一行注释；源码中没有任何外部 include 或调用，只有 VS project/filter 注册。上一轮 `RuntimeVerificationArgs.h` 已经不再传递完整 application config，因此继续保留 PBR-specific args facade 只会保留误导性的旧命名边界。

新增与修改：

- 删除 `RuntimePBRVerificationArgs.h`。
- 删除 `RuntimePBRVerificationArgs.cpp`。
- `text2.vcxproj` 移除 `RuntimePBRVerificationArgs.cpp` 与 `RuntimePBRVerificationArgs.h` 注册。
- `text2.vcxproj.filters` 移除对应 filter 注册。
- `RuntimeVerificationArgs.h/.cpp` 保持 runtime verification args 的唯一入口，不改变 CLI argument parsing 或 verification mode table。

已完成验证：

- 静态检查确认 `application`、`main.cpp`、`tools`、`engine`、`text2.vcxproj` 与 `text2.vcxproj.filters` 中不再存在 `RuntimePBRVerificationArgs` 引用。
- 文件存在性检查确认 `RuntimePBRVerificationArgs.h/.cpp` 已从工作树移除。
- 静态检查确认 `RuntimeVerificationArgs.cpp` 和 `RuntimeApplicationEntry.cpp` 仍显式持有完整 `RuntimeApplicationConfig.h`，`RuntimeVerificationArgs.h` 仍只 forward declare shell config。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 dead PBR-specific verification args compatibility facade removal，不改变 runtime verification args parser、verification defaults、mode selection、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续清理类似无外部调用者的 compatibility façade，或转向 application bridge implementation include audit；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime Camera/Scene Prepare Config Consumer Header Forward Boundary Cleanup

本轮先复查旧 compatibility facade 和 application bridge include surface。确认旧 PBR args 兼容层已删除，剩余 `RuntimePBR*` 多为真实 verification module/config，不适合继续为了命名清理而删除；同时 application bridge 的剩余完整 config/state include 大多对应字段读取、返回值完整类型或 state accessor 调用。因此本轮转向计划内的 `runtime content lifecycle config/data boundary cleanup`，只清理通过 `const Runtime*Config&` 接收配置的 consumer header。

新增与修改：

- `RuntimeCameraLifecycle.h` 不再 include `RuntimeCameraConfig.h`，改为 forward declare `RuntimeCameraConfig`；`RuntimeCameraLifecycle.cpp` 显式 include 完整 config，因为实现读取 camera 参数字段。
- `RuntimeLegacyExperimentLifecycle.h` 不再 include `RuntimeScenePrepareConfig.h`，改为 forward declare `RuntimeScenePrepareConfig`；`RuntimeLegacyExperimentLifecycle.cpp` 显式 include 完整 config，保持 legacy experiment prepare owner 的局部依赖。
- `RuntimeSceneSetupContextFactory.h` 不再 include `RuntimeScenePrepareConfig.h`，改为 forward declare；`RuntimeSceneSetupContextFactory.cpp` 显式 include 完整 config，因为 setup context construction 读取 width/height/texture path。
- `RuntimeSceneSetupPipelineLifecycle.h` 不再 include `RuntimeScenePrepareConfig.h`，改为 forward declare；`RuntimeSceneSetupPipelineLifecycle.cpp` 显式 include 完整 config，因为 implementation 读取 `sceneSetupPipeline`。
- 本轮没有启动新的 sidecar：写域集中在四组 content/camera/scene setup consumer header 和 implementation，验证覆盖路径明确，协作文档记录为 parent-owned、无 delegated write scope。

已完成验证：

- 静态检查确认四个 consumer header 中只保留 `RuntimeCameraConfig` / `RuntimeScenePrepareConfig` 前置声明。
- 静态检查确认完整 `RuntimeCameraConfig.h` / `RuntimeScenePrepareConfig.h` include 已下沉到对应 `.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 camera lifecycle、legacy experiment lifecycle、scene setup context factory 和 scene setup pipeline lifecycle。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 PBR scene stats、Engine World minimal/package、runtime Engine attach/detach、renderer backend contract、cleanup 和 fixed delta 关键输出仍然存在。

结论：

- 这是 runtime content config consumer header boundary cleanup，不改变 camera 初始化参数、legacy experiment gate、scene setup context construction、scene setup pipeline、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续 `runtime content lifecycle config/data boundary cleanup` 的剩余 consumer header include audit，或转向 shutdown cleanup refs type boundary cleanup；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime Engine Lifecycle State/Cleanup Refs Header Split

本轮先继续 runtime content lifecycle config/data boundary audit，确认 `RuntimeCameraConfig.h` / `RuntimeScenePrepareConfig.h` 的 public include 只剩 `RuntimeContentLifecycleConfig.h`，这是按值聚合 DTO，不能改为前置声明。随后转向计划内的 shutdown cleanup refs type boundary cleanup：`RuntimeEngineLifecycleTypes.h` 同时承载 engine lifecycle state 与 cleanup refs，导致 frame run、state owner 和 shutdown cleanup/report 路径共享一个过宽类型头。

新增与修改：

- 新增 `RuntimeEngineLifecycleState.h`，只承载 `RuntimeEngineLifecycleState`。
- 新增 `RuntimeEngineLifecycleCleanupRefs.h`，只承载 `RuntimeEngineLifecycleCleanupRefs`。
- 删除旧 `RuntimeEngineLifecycleTypes.h`，避免继续保留 state/cleanup refs 聚合宽头。
- `RuntimeApplicationFrameRunBridge.cpp` 与 `RuntimeApplicationState.cpp` 改为 include `RuntimeEngineLifecycleState.h`。
- `RuntimeApplicationShutdownCleanupBridge.cpp`、`RuntimeApplicationShutdownEngineBridge.cpp` 与 `RuntimeApplicationShutdownVerificationBridge.cpp` 改为 include `RuntimeEngineLifecycleCleanupRefs.h`。
- `RuntimeEngineLifecycle.cpp` 同时 include state 与 cleanup refs 两个窄头，因为 implementation 同时读写 state 并构造 cleanup refs。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 移除旧宽头注册并新增两个窄头注册。

已完成验证：

- 静态检查确认 `application`、`engine`、`tools`、`main.cpp`、`text2.vcxproj` 与 `text2.vcxproj.filters` 中不再存在 `RuntimeEngineLifecycleTypes` 引用。
- 文件存在性检查确认旧 `RuntimeEngineLifecycleTypes.h` 已移除，新 `RuntimeEngineLifecycleState.h` 与 `RuntimeEngineLifecycleCleanupRefs.h` 存在。
- 静态检查确认 frame/state 路径只 include state 窄头，shutdown cleanup/report 路径只 include cleanup refs 窄头，engine lifecycle implementation 持有两个完整类型依赖。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 frame run、shutdown cleanup、shutdown verification 和 engine lifecycle implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 PBR scene stats、Engine World minimal/package、runtime Engine attach/detach、renderer backend contract、cleanup 和 fixed delta 关键输出仍然存在。

结论：

- 这是 Runtime Engine lifecycle type boundary split，不改变 Engine initialize/attach/detach/shutdown、renderer backend cleanup、cleanup report、frame run renderer subsystem access、Engine World verification 或 PBR pass。
- 后续建议继续沿 header surface 做小切片：优先审计 RuntimeApplicationState accessor call-site 的剩余 type include，或继续 shutdown/application bridge 的实现 include surface；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime Frame Lifecycle Types Compatibility Aggregator Removal

本轮继续 header surface cleanup。复查 frame lifecycle 类型边界后确认：`RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h` 已经是 canonical 窄头，源码和工具中没有任何 include `RuntimeFrameLifecycleTypes.h`；旧聚合头只剩 VS project/filter 注册，会继续传递一个过时的“config/state 聚合入口”概念。

新增与修改：

- 删除 `RuntimeFrameLifecycleTypes.h`。
- `text2.vcxproj` 移除 `RuntimeFrameLifecycleTypes.h` 注册。
- `text2.vcxproj.filters` 移除对应 filter 注册。
- 保留 `RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h` 作为 frame lifecycle config/state 的唯一窄头入口。

已完成验证：

- 静态检查确认 `application`、`engine`、`tools`、`main.cpp`、`text2.vcxproj` 与 `text2.vcxproj.filters` 中不再存在 `RuntimeFrameLifecycleTypes` 引用。
- 文件存在性检查确认旧聚合头已移除，`RuntimeFrameLifecycleConfig.h` 与 `RuntimeFrameLifecycleState.h` 仍存在。
- 静态检查确认 frame lifecycle facade、frame lifecycle implementation、application frame bridge、application state 和 VS project/filter 仍直接依赖 config/state 窄头。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,renderer-backend-registry-noop -DiscardCaptures`：构建通过；三条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 PBR scene stats、Engine World minimal/package、runtime Engine attach/detach、renderer backend contract、cleanup 和 fixed delta 关键输出仍然存在。

结论：

- 这是 dead compatibility aggregator removal，不改变 frame lifecycle reset/run/continue、frame clock、verification capture、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续沿窄 header surface 推进：优先审计 `RuntimeApplicationState` accessor call-site 的剩余 type include，或继续 application/shutdown bridge implementation include surface；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime Application Public Header Include Boundary Cleanup

本轮按上一轮建议审计 `RuntimeApplicationState` accessor call-site。复查后确认真正 include `RuntimeApplicationState.h` 的 `.cpp` 基本都在函数体内调用 `state.runtime()`、`state.engine()`、`state.engineLifecycle()`、`state.editorLifecycle()`、`state.frameLifecycle()` 或 `state.legacyExperiments()`，这些调用点需要完整 state header，不能为了减少 include 而改成不透明转发。随后转向相邻 application public-header / implementation include surface，清理两个可证明的 public header 传递依赖和一个 shutdown implementation 冗余 include。

新增与修改：

- `RuntimeApplicationCallbackBinder.h` 移除 `RuntimeBootstrapper.h` include，改为 forward declare `RuntimeBootstrapperCallbacks`。
- `RuntimeApplicationCallbackBinder.cpp` 显式 include `RuntimeBootstrapper.h`，因为这里定义 `makeCallbacks(...)` 并构造 callback DTO。
- `RuntimeApplicationShell.cpp` 显式 include `RuntimeBootstrapper.h`，因为 `RuntimeApplicationShell::makeCallbacks()` 按值返回 `RuntimeBootstrapperCallbacks`。
- `RuntimeApplicationWindowStartupLifecycle.h` 移除 `RuntimeWindowLifecycleTypes.h` include，改为 forward declare `RuntimeWindowSnapshot`。
- `RuntimeApplicationStartupLifecycle.cpp` 显式 include `RuntimeWindowLifecycleTypes.h`，因为 startup implementation 按值接收 window snapshot 并继续传给 content/editor startup。
- `RuntimeApplicationShutdownVerificationBridge.cpp` 移除未使用的 `RuntimeApplicationShutdownEngineBridge.h` include；该文件只需要 config、cleanup refs 和 cleanup verification lifecycle。

已完成验证：

- 静态检查确认 `RuntimeApplicationCallbackBinder.h` 只 forward declare `RuntimeBootstrapperCallbacks`，完整 `RuntimeBootstrapper.h` 依赖下沉到 callback binder implementation 和 shell implementation。
- 静态检查确认 `RuntimeApplicationWindowStartupLifecycle.h` 只 forward declare `RuntimeWindowSnapshot`，完整 window lifecycle types 依赖下沉到 startup implementation。
- 静态检查确认 `RuntimeApplicationShutdownVerificationBridge.cpp` 不再 include shutdown engine bridge。
- 静态检查确认实际 accessor 调用点仍保留完整 `RuntimeApplicationState.h` include，没有把需要完整类型的访问改成隐式传递。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-editor-create,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 callback binder、shell、startup lifecycle、window startup lifecycle 和 shutdown verification bridge。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 PBR scene stats、Engine World minimal/editor-create/package、runtime Engine attach/detach、renderer backend contract、cleanup 和 fixed delta 关键输出仍然存在。

结论：

- 这是 application public header include boundary cleanup，不改变 bootstrapper callback 绑定、application startup order、window snapshot capture、shutdown cleanup reporting、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续做 application/shutdown bridge implementation include audit，或转向下一组 verification/report public header 的 forward boundary cleanup；当前不建议继续扩张 PBR pass。

### 2026-05-31 Runtime PBR Verification Config Include Boundary Cleanup

本轮继续 verification/report public header forward boundary audit。复查后确认 PBR/verification public headers 大多已经只保留前置声明；可继续收窄的是两个 PBR verification implementation：`RuntimePBRPreparedSceneStatsVerification.cpp` 的 `RuntimeVerificationConfig` 参数未使用，`RuntimePBRProfileVerification.cpp` 只把 config 作为 const 引用转发给 startup/pass/preview/light/profile-line 子模块。两者不需要完整 `RuntimeVerificationConfig.h`。

新增与修改：

- `RuntimePBRPreparedSceneStatsVerification.cpp` 移除 `RuntimeVerificationConfig.h` include，继续通过 header 前置声明保留未命名 `const RuntimeVerificationConfig&` 参数。
- `RuntimePBRProfileVerification.cpp` 移除 `RuntimeVerificationConfig.h` include，只保留 PBR profile 子模块 headers；完整 config 依赖仍在实际读取字段的 pass/preview/light/profile-line implementations。

已完成验证：

- 静态检查确认两个目标 implementation 不再 include `RuntimeVerificationConfig.h`。
- 静态检查确认 `RuntimePBRPreparedSceneStatsVerification.cpp` 不读取 config 字段，只输出 prepared scene stats；`RuntimePBRProfileVerification.cpp` 只把 const config 引用转发给子模块。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 `RuntimePBRPreparedSceneStatsVerification.cpp` 与 `RuntimePBRProfileVerification.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 PBR scene stats、Engine World minimal/package、runtime Engine attach/detach、renderer backend contract、cleanup 和 fixed delta 关键输出仍然存在。

结论：

- 这是 PBR verification implementation include boundary cleanup，不改变 PBR startup profile application、prepared scene stats、profile line输出、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续 verification/report implementation include audit，优先找“只转发 const config/context 引用、不读取字段”的 implementation；当前不建议继续扩张 PBR pass。

### 2026-05-31 Subagent Boundary Restart and Next Slice Audit

用户要求可以考虑使用多 agent，但必须先写清楚边界和共享沟通文档，再自行恢复 active goal。当前 goal 仍为 active，因此本轮不创建新 goal、不重置历史，只按原 objective 继续推进。

本轮已先更新 `docs/subagents_coordination.md`：

- 新增 `Subagent Restart Protocol`，明确 parent 先写边界、sidecar 默认只读、只有显式 disjoint write scope 才能改文件。
- 当前 active round 切到 `Subagent Boundary Restart and Next Slice Audit`。
- `imgui.ini` 继续作为无关本地状态，parent 与 sidecar 都不得触碰。
- sidecar 结论必须通过共享格式汇报，parent 验证、整合并写入 `worked.md` 后才算接受。

已启动两个只读 sidecar：

- `Harvey`：审计 verification/report implementation include surface，目标是找下一步最安全的降耦合候选。
- `Galileo`：审计 application/startup/frame/shutdown bridge implementation include surface，目标是找不改变生命周期顺序的最小 cleanup 候选。

结论：

- 这是 goal continuation 和 subagent coordination restart，不是代码行为变更。
- parent 下一步会基于 sidecar 结果选择一个最小切片，本地完成实现、验证和文档更新。

### 2026-05-31 Runtime Editor Lifecycle Callback DTO Header Boundary Cleanup

两个只读 sidecar 均已返回：

- `Harvey` 审计 verification/report include surface，未找到安全的“直接删 include”候选；建议后续可考虑把 `EngineSubsystemLifecycleSummary` / `EngineLifecycleSnapshot` 从 `Engine.h` 拆成轻量 engine header，但该方向涉及 engine header 与 project registration，风险高于本轮目标。
- `Galileo` 审计 application startup/frame/shutdown bridge include surface，确认多数 include 因 DTO 字段访问、state accessor、config 字段读取或按值返回需要保留；唯一低风险候选是 `RuntimeEditorLifecycle.h` 只为返回值声明包含 `RuntimeFrameCallbacks.h`。

本轮接受 `Galileo` 的更小切片：

- `RuntimeEditorLifecycle.h` 移除 `RuntimeFrameCallbacks.h` include，改为 forward declare `RuntimeFrameCallbacks`。
- `RuntimeEditorLifecycle.cpp` 显式 include `RuntimeFrameCallbacks.h`，因为函数定义内部构造并填充 callback DTO。
- `RuntimeApplicationFrameEditorCallbackBridge.cpp` 原本已 include 完整 callback DTO，因此调用 `RuntimeEditorLifecycle::makeFrameCallbacks(...)` 的完整类型需求不变。

已完成验证：

- 静态检查确认 `RuntimeEditorLifecycle.h` 只 forward declare `RuntimeFrameCallbacks`，完整 DTO include 位于 `RuntimeEditorLifecycle.cpp` 与 frame editor callback bridge implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-editor-create,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 `RuntimeEditorLifecycle.cpp`、editor startup 和 frame editor callback bridge。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。
- 日志检查确认 PBR scene stats、Engine World minimal/editor-create、runtime Engine attach/detach、renderer backend contract、cleanup 和 fixed delta 关键输出仍然存在。

结论：

- 这是 editor lifecycle public header include boundary cleanup，不改变 GUI 初始化、editor panel callback 创建、startup/frame/shutdown 顺序、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议优先评估 `Harvey` 提到的 `EngineLifecycleSnapshot` type header extraction，或继续做低风险 application/shutdown bridge include audit；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Engine Lifecycle Snapshot Type Header Extraction

本轮按上一轮 `Harvey` 的较大候选继续推进，但保持切片只覆盖 Engine lifecycle snapshot DTO 边界，不改 Engine 行为、不扩张 PBR。审计确认：`EngineLifecycleSnapshot` / `EngineSubsystemLifecycleSummary` 定义在 `Engine.h`，导致纯 formatter implementation `RuntimeEngineVerificationReport.cpp` 为了读取快照字段而包含完整 Engine owner header。

新增与修改：

- 新增 `engine/EngineLifecycleSnapshot.h`，集中定义 `EngineSubsystemLifecycleSummary` 与 `EngineLifecycleSnapshot`。
- `Engine.h` 移除 snapshot DTO 定义，改为 include `EngineLifecycleSnapshot.h`，继续暴露 `Engine::captureLifecycleSnapshot()`。
- `RuntimeEngineVerificationReport.cpp` 不再 include 完整 `Engine.h`，改为 include `EngineLifecycleSnapshot.h`；report formatter 只依赖 snapshot DTO 与 renderer backend stats DTO。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `EngineLifecycleSnapshot.h` 到 `include\GLengine`。

已完成验证：

- 静态检查确认 snapshot/summary 结构只在 `EngineLifecycleSnapshot.h` 定义，`RuntimeEngineVerificationReport.cpp` 已改为 include 轻量 snapshot header。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 `Engine.cpp`、`RuntimeEngineVerificationReport.cpp`、`RuntimeVerificationReport.cpp` 和 `EngineDiagnosticsPanel.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine lifecycle snapshot DTO header extraction，不改变 `Engine::captureLifecycleSnapshot()` 输出、diagnostics、runtime engine verification report、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续评估是否需要把 `EngineRunMode` 从 `EngineContext.h` 中拆成更小枚举头，或回到 application/shutdown bridge implementation include audit；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Engine Run Mode Header Extraction

本轮继续上一轮的后续方向。审计确认：`EngineLifecycleSnapshot.h` 只需要 `EngineRunMode`，但此前为了这个枚举包含完整 `EngineContext.h`；而 `EngineContext.h` 还定义 `EngineDesc` 并包含 `<string>`。因此 snapshot/report 路径仍被不必要地牵连到完整 Engine context/desc 数据模型。

新增与修改：

- 新增 `engine/EngineRunMode.h`，单独承载 `EngineRunMode` 枚举。
- `EngineContext.h` 移除 `EngineRunMode` 枚举定义，改为 include `EngineRunMode.h`，继续承载 `EngineDesc` 与 `EngineContext`。
- `EngineLifecycleSnapshot.h` 不再 include `EngineContext.h`，改为 include `EngineRunMode.h`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `EngineRunMode.h` 到 `include\GLengine`。

已完成验证：

- 静态检查确认 `EngineRunMode` 只在 `EngineRunMode.h` 定义；`EngineDesc` / `EngineContext` 仍只在 `EngineContext.h` 定义；`EngineLifecycleSnapshot.h` 只依赖 `EngineRunMode.h`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 `RuntimeApplicationConfigPolicy.cpp`、`RuntimeApplicationEngineStartupLifecycle.cpp`、`Engine.cpp`、`RendererSubsystem.cpp`、`RuntimeEngineVerificationReport.cpp` 与 `EngineDiagnosticsPanel.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine run mode enum header extraction，不改变 `EngineDesc`、`EngineContext`、`Engine::captureLifecycleSnapshot()`、diagnostics、runtime engine verification report、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续评估 `EngineDesc` / `EngineContext` 是否需要拆分，或回到低风险 application/shutdown bridge implementation include audit；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Engine Desc Header Extraction

本轮继续上一轮留下的 `EngineDesc` / `EngineContext` 数据模型拆分方向。审计确认：`RuntimeApplicationConfigPolicy.cpp` 和 `RuntimeApplicationEngineStartupLifecycle.cpp` 只需要构造或消费 `EngineDesc`，不需要完整的 per-frame `EngineContext`。此前它们为了 `EngineDesc` 直接 include `EngineContext.h`，会把 runtime context 数据模型不必要地传递到 application startup/config policy 实现中。

新增与修改：

- 新增 `engine/EngineDesc.h`，单独承载 `EngineDesc` 以及其需要的 `<string>` 和 `EngineRunMode.h` 依赖。
- `EngineContext.h` 移除 `EngineDesc` 定义，改为 include `EngineDesc.h`，继续只承载 per-frame `EngineContext`。
- `RuntimeApplicationConfigPolicy.cpp` 与 `RuntimeApplicationEngineStartupLifecycle.cpp` 改为 include `EngineDesc.h`，不再 include `EngineContext.h`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 `EngineDesc.h` 到 `include\GLengine`。

已完成验证：

- 静态检查确认 `EngineDesc` 只在 `EngineDesc.h` 定义；`EngineContext` 仍只在 `EngineContext.h` 定义；`EngineRunMode` 仍只在 `EngineRunMode.h` 定义；两个 application 调用点没有残留 `EngineContext.h` include。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 `RuntimeApplicationConfigPolicy.cpp`、`RuntimeApplicationEngineStartupLifecycle.cpp`、`RuntimeEngineLifecycle.cpp`、`Engine.cpp`、`RendererSubsystem.cpp` 与 `EngineDiagnosticsPanel.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine desc DTO header extraction，不改变 `EngineDesc` 字段、`EngineContext` 字段、Engine 初始化行为、runtime lifecycle、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续低风险 application/shutdown bridge implementation include audit，或继续审计 Engine public header 对 `EngineContext.h` 的暴露边界；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Application Shutdown Verification Config Boundary Cleanup

本轮继续上一轮建议中的 application/shutdown bridge implementation include audit。审计确认：`RuntimeApplicationShutdownVerificationBridge` 只需要 verification 子配置，但此前 API 接收完整 `RuntimeApplicationShellConfig` 并在 implementation 中 include `RuntimeApplicationConfig.h` 读取 `config.verification`。这会让 shutdown verification bridge 依赖完整 shell config、window config、frame clock config、renderer backend key 等无关字段。

新增与修改：

- `RuntimeApplicationConfigPolicy` 新增 `verificationConfig(const RuntimeApplicationShellConfig&) -> const RuntimeVerificationConfig&`，把 shell config 字段读取集中到 config policy implementation。
- `RuntimeApplicationShutdownCleanupBridge.cpp` 通过 `RuntimeApplicationConfigPolicy::verificationConfig(config)` 传递 verification 子配置。
- `RuntimeApplicationShutdownVerificationBridge.h/.cpp` 将 `reportRendererSubsystemCleanup(...)` 与 `reportEngineCleanup(...)` 的 config 参数从完整 `RuntimeApplicationShellConfig` 收敛为 `RuntimeVerificationConfig`。
- `RuntimeApplicationShutdownVerificationBridge.cpp` 移除 `RuntimeApplicationConfig.h` include，只保留 cleanup refs 与 verification cleanup lifecycle 依赖。

已完成验证：

- 静态检查确认 `RuntimeApplicationShutdownVerificationBridge` 不再 include 或 forward declare `RuntimeApplicationShellConfig`，也不再 include `RuntimeApplicationConfig.h`；完整 shell config 字段读取仅保留在 `RuntimeApplicationConfigPolicy.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 `RuntimeApplicationConfigPolicy.cpp`、`RuntimeApplicationShutdownCleanupBridge.cpp` 与 `RuntimeApplicationShutdownVerificationBridge.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 shutdown verification config boundary cleanup，不改变 shutdown cleanup order、camera cleanup、runtime context detach、Engine shutdown、renderer backend contract cleanup、Engine cleanup verification output 或 PBR pass。
- 后续建议继续审计 application/shutdown bridge implementation include surface 的剩余调用点，或转向 Engine public header 对 `EngineContext.h` 的暴露边界。

### 2026-05-31 Engine Public Header Context Ownership Boundary Cleanup

本轮按上一轮建议转向 Engine public header 暴露边界。审计确认：`Engine.h` 此前为了按值持有 `EngineContext` 和 `std::unique_ptr<World>` 直接 include `EngineContext.h` / `World.h`，会把完整 per-frame context 和 World 行为头传递给所有只需要 Engine API 的调用点。

新增与修改：

- `Engine.h` 移除 `EngineContext.h` 与 `World.h` include，改为 forward declare `EngineContext`、`EngineDesc` 和 `World`。
- `Engine` 将 `EngineContext` 从按值成员改为 `std::unique_ptr<EngineContext>`，构造函数改为 out-of-line，由 `Engine.cpp` 初始化完整 context。
- `Engine.cpp` 显式 include `EngineContext.h` 和 `World.h`，集中 context 字段访问、World 生命周期、subsystem initialize/tick/shutdown 和 lifecycle snapshot 字段读取。
- `addSubsystem(...)` 行为不变：Engine 已初始化时仍立即用当前 `EngineContext` 初始化新 subsystem。

并行只读 sidecar：

- `Erdos` 审计 `Engine.h` / `Engine.cpp`，未修改文件。
- 结论：forward declarations 足够；`std::unique_ptr<EngineContext>` 在 out-of-line 构造/析构下安全；inline `addSubsystem(...)` 当前只形成并传递 `EngineContext&`，MSVC 构建也已验证通过。
- 非阻塞建议：如后续希望做到 public header 完全不解引用 context，可再把 subsystem 初始化调用移入 `.cpp` helper；本轮不增加额外行为改动。

已完成验证：

- 静态检查确认 `Engine.h` 不再 include `EngineContext.h` / `World.h`，只保留 forward declarations 和 `std::unique_ptr<EngineContext>` ownership。
- 静态检查确认完整 `EngineContext.h` / `World.h` 依赖集中在 `Engine.cpp`，Engine 初始化、tick、shutdown、snapshot、World create/get 路径继续通过完整类型实现。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 Engine 初始化、World ownership、renderer backend contract、Engine World package 和 cleanup。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine public header include / ownership boundary cleanup，不改变 Engine startup desc、EngineContext 字段、subsystem lifecycle、active World lifecycle、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续审计 Engine public header 中 inline template 对 context 的边界是否需要 helper 下沉，或回到 application/shutdown bridge include audit；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Engine AddSubsystem Context Helper Boundary Cleanup

本轮承接上一轮留下的非阻塞 follow-up：虽然 `Engine.h` 已不再 include `EngineContext.h` / `World.h`，但 public template `Engine::addSubsystem(...)` 仍直接执行 `subsystemRef.initialize(*mContext)`。这会让 public header 的模板体继续触碰 context 存储细节，不利于后续维持 “header 只声明边界，implementation 承载完整类型行为” 的方向。

新增与修改：

- `Engine.h` 新增私有 `initializeSubsystemIfNeeded(EngineSubsystem&)` 声明。
- `Engine::addSubsystem(...)` 在 Engine 已初始化时改为调用 `initializeSubsystemIfNeeded(subsystemRef)`，不再直接解引用 `mContext`。
- `Engine.cpp` 新增 `Engine::initializeSubsystemIfNeeded(...)`，在 `.cpp` 内通过完整 `EngineContext.h` 依赖执行 `subsystem.initialize(*mContext)`。
- 现有行为保持不变：如果 Engine 已初始化，新增 subsystem 仍立即接收当前 Engine context 并执行 initialize。

已完成验证：

- 静态检查确认 `Engine.h` 没有 `EngineContext.h` / `World.h` include，也没有 `*mContext` 或 `mContext->` 直接访问；public template 只调用私有 helper。
- 静态检查确认 context 解引用集中到 `Engine.cpp`，其中包含完整 `EngineContext.h` / `World.h`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 Engine 初始化、subsystem attach、World ownership、renderer backend contract、Engine World package 和 cleanup。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine public template implementation-detail cleanup，不改变 Engine startup desc、EngineContext 字段、subsystem lifecycle、active World lifecycle、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议回到 application/shutdown bridge include audit，或继续审计 Engine public header 里是否还有可下沉的 implementation 细节；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Application Shutdown Cleanup Verification Config Boundary Cleanup

本轮回到 application/shutdown bridge include audit。审计确认：`RuntimeApplicationShutdownCleanupBridge` 仍接收完整 `RuntimeApplicationShellConfig`，但它自身只需要把 verification 子配置传给 shutdown verification bridge。完整 shell config 字段读取可以上移到 shutdown lifecycle facade，由 `RuntimeApplicationConfigPolicy` 做映射。

新增与修改：

- `RuntimeApplicationShutdownCleanupBridge.h/.cpp` 的 `cleanup(...)` 参数从 `RuntimeApplicationShellConfig` 收窄为 `RuntimeVerificationConfig`。
- `RuntimeApplicationShutdownCleanupBridge.cpp` 移除 `RuntimeApplicationConfigPolicy.h` include，不再调用 `verificationConfig(...)`，也不再知道完整 shell config。
- `RuntimeApplicationShutdownLifecycle.cpp` 显式 include `RuntimeApplicationConfigPolicy.h`，在 facade 层通过 `RuntimeApplicationConfigPolicy::verificationConfig(config)` 把 shell config 映射为 verification config，再传入 cleanup bridge。
- shutdown cleanup 顺序不变：begin cleanup -> renderer subsystem cleanup report -> camera/runtime context detach -> Engine shutdown -> Engine cleanup report。

已完成验证：

- 静态检查确认 shutdown cleanup bridge 不再引用 `RuntimeApplicationShellConfig`、`RuntimeApplicationConfigPolicy` 或 `RuntimeApplicationConfig.h`。
- 静态检查确认 `RuntimeApplicationShutdownLifecycle.cpp` 是本轮新增的 shell config 到 verification config 映射点。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；编译覆盖 `RuntimeApplicationShutdownCleanupBridge.cpp` 与 `RuntimeApplicationShutdownLifecycle.cpp`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 shutdown cleanup bridge config boundary cleanup，不改变 shutdown cleanup order、camera cleanup、runtime context detach、Engine shutdown、renderer backend contract cleanup、Engine cleanup verification output 或 PBR pass。
- 后续建议继续 application/shutdown bridge include audit，或继续审计 Engine public header 里是否还有可下沉的 implementation 细节；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Application Shutdown Lifecycle Verification Config Boundary Cleanup

本轮继续 shutdown config boundary audit。上一轮已经让 cleanup bridge 只接收 `RuntimeVerificationConfig`，但 `RuntimeApplicationShutdownLifecycle` facade 仍暴露完整 `RuntimeApplicationShellConfig`，它实际也只负责把 verification config 继续传给 cleanup bridge。更合适的边界是：callback binder 作为 shell callback 组装层读取 shell config 并完成 mapping，shutdown lifecycle / cleanup bridge 只看 verification 子配置。

新增与修改：

- `RuntimeApplicationShutdownLifecycle.h/.cpp` 的 `cleanup(...)` 参数从 `RuntimeApplicationShellConfig` 收窄为 `RuntimeVerificationConfig`。
- `RuntimeApplicationShutdownLifecycle.cpp` 移除 `RuntimeApplicationConfigPolicy.h` include，不再做 shell config 到 verification config 的映射。
- `RuntimeApplicationCallbackBinder.cpp` 新增 `RuntimeApplicationConfigPolicy.h` include，并在 cleanup callback 中通过 `RuntimeApplicationConfigPolicy::verificationConfig(config)` 传入 `RuntimeApplicationShutdownLifecycle::cleanup(...)`。
- `RuntimeApplicationShutdownLifecycle` 与 `RuntimeApplicationShutdownCleanupBridge` 当前都不再引用完整 shell config、config policy 或 `RuntimeApplicationConfig.h`。
- shutdown cleanup 顺序不变：begin cleanup -> renderer subsystem cleanup report -> camera/runtime context detach -> Engine shutdown -> Engine cleanup report。

已完成验证：

- 静态检查确认 shutdown lifecycle 和 cleanup bridge 都不再引用 `RuntimeApplicationShellConfig`、`RuntimeApplicationConfigPolicy` 或 `RuntimeApplicationConfig.h`。
- 静态检查确认 cleanup callback 是 shell config 到 verification config 的唯一新增映射点。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；四条 focused verification mode 全部通过；覆盖 callback binder、shutdown lifecycle、cleanup bridge、Engine World 和 renderer backend cleanup 合同。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 shutdown lifecycle config boundary cleanup，不改变 shutdown cleanup order、camera cleanup、runtime context detach、Engine shutdown、renderer backend contract cleanup、Engine cleanup verification output、runtime frame behavior 或 PBR pass。
- 后续建议继续 application shutdown bridge / callback binder include audit，或回到 Engine public header 低风险 implementation detail audit；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Application Callback Binder Shutdown Bridge Boundary Cleanup

本轮继续 application shutdown bridge / callback binder include audit。上一轮把 shell config 到 verification config 的映射上移到了 `RuntimeApplicationCallbackBinder.cpp`，但这让 generic callback binder implementation 直接 include `RuntimeApplicationConfigPolicy.h` 和 `RuntimeApplicationShutdownLifecycle.h`。更合适的边界是：callback binder 只组装 bootstrap callbacks，shutdown callback 的 config mapping 与 lifecycle forwarding 下沉到专用 shutdown callback bridge。

新增与修改：

- 新增 `RuntimeApplicationShutdownCallbackBridge.h/.cpp`，负责 cleanup callback 的 shell config 到 verification config 映射，并负责 destroy callback 的 shutdown lifecycle 转发。
- `RuntimeApplicationCallbackBinder.cpp` 移除 `RuntimeApplicationConfigPolicy.h` 和 `RuntimeApplicationShutdownLifecycle.h` include，改为 include `RuntimeApplicationShutdownCallbackBridge.h`。
- cleanup lambda 改为调用 `RuntimeApplicationShutdownCallbackBridge::cleanup(state, config)`；destroy lambda 改为调用 `RuntimeApplicationShutdownCallbackBridge::destroy()`。
- `RuntimeApplicationShutdownCallbackBridge.cpp` 是本轮唯一持有 `RuntimeApplicationConfigPolicy::verificationConfig(config)` 与 `RuntimeApplicationShutdownLifecycle::cleanup(...)` 组合逻辑的 callback 层。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 shutdown callback bridge source/header 到 Application 分组。

已完成验证：

- 静态检查确认 `RuntimeApplicationCallbackBinder.cpp` 不再直接引用 `RuntimeApplicationConfigPolicy` 或 `RuntimeApplicationShutdownLifecycle`。
- 静态检查确认 `RuntimeApplicationShutdownLifecycle` 与 `RuntimeApplicationShutdownCleanupBridge` 仍不引用完整 shell config、config policy 或 `RuntimeApplicationConfig.h`。
- 静态检查确认 `RuntimeApplicationShutdownCallbackBridge.h/.cpp` 已注册到 VS project/filter。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeApplicationCallbackBinder.cpp` 与 `RuntimeApplicationShutdownCallbackBridge.cpp`；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 callback binder shutdown bridge boundary cleanup，不改变 startup/frame callback 顺序、shutdown cleanup order、camera cleanup、runtime context detach、Engine shutdown、renderer backend contract cleanup、Engine cleanup verification output、runtime frame behavior 或 PBR pass。
- 后续建议继续 callback binder startup/frame include audit，或回到 Engine public header 低风险 implementation detail audit；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Application Callback Binder Frame Bridge Boundary Cleanup

本轮继续 callback binder startup/frame include audit。上一轮已经把 shutdown callback 的 mapping/forwarding 下沉到 shutdown callback bridge；当前 `RuntimeApplicationCallbackBinder.cpp` 仍直接 include `RuntimeApplicationFrameLifecycle.h` 并直接调用 shouldContinue / runFrame。更合适的边界是：generic callback binder 只组装 bootstrap callbacks，frame callback 的 lifecycle forwarding 下沉到专用 frame callback bridge。

新增与修改：

- 新增 `RuntimeApplicationFrameCallbackBridge.h/.cpp`，负责 bootstrapper shouldContinue / runFrame callback 到 `RuntimeApplicationFrameLifecycle` 的转发。
- `RuntimeApplicationCallbackBinder.cpp` 移除 `RuntimeApplicationFrameLifecycle.h` include，改为 include `RuntimeApplicationFrameCallbackBridge.h`。
- shouldContinue lambda 改为调用 `RuntimeApplicationFrameCallbackBridge::shouldContinue(config, state)`。
- runFrame lambda 改为调用 `RuntimeApplicationFrameCallbackBridge::runFrame(state, config)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 frame callback bridge source/header 到 Application 分组。

已完成验证：

- 静态检查确认 `RuntimeApplicationCallbackBinder.cpp` 不再直接引用 `RuntimeApplicationFrameLifecycle`。
- 静态检查确认 `RuntimeApplicationCallbackBinder.cpp` 仍不直接引用 shutdown lifecycle 或 config policy。
- 静态检查确认 `RuntimeApplicationFrameCallbackBridge.h/.cpp` 已注册到 VS project/filter。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeApplicationCallbackBinder.cpp` 与 `RuntimeApplicationFrameCallbackBridge.cpp`；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 callback binder frame bridge boundary cleanup，不改变 startup/frame/shutdown callback 顺序、frame continue 判断、frame execution、editor callback 执行、legacy experiment ticking、verification capture timing、renderer backend contract、Engine cleanup verification output 或 PBR pass。
- 后续建议继续 callback binder startup include audit，或回到 Engine public header 低风险 implementation detail audit；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Application Callback Binder Startup Bridge Boundary Cleanup

本轮继续 callback binder startup include audit。上一轮已经把 frame callback forwarding 下沉到 frame callback bridge；当前 `RuntimeApplicationCallbackBinder.cpp` 仍直接 include `RuntimeApplicationStartupLifecycle.h` 并直接调用 initialize。更合适的边界是：generic callback binder 只组装 bootstrap callbacks，startup callback 的 lifecycle forwarding 也下沉到专用 startup callback bridge。

新增与修改：

- 新增 `RuntimeApplicationStartupCallbackBridge.h/.cpp`，负责 bootstrapper initialize callback 到 `RuntimeApplicationStartupLifecycle` 的转发。
- `RuntimeApplicationCallbackBinder.cpp` 移除 `RuntimeApplicationStartupLifecycle.h` include，改为 include `RuntimeApplicationStartupCallbackBridge.h`。
- startup lambda 改为调用 `RuntimeApplicationStartupCallbackBridge::initialize(state, config)`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 startup callback bridge source/header 到 Application 分组。
- 当前 `RuntimeApplicationCallbackBinder.cpp` 只直接依赖 startup/frame/shutdown callback bridge headers 和 `RuntimeBootstrapper.h` 的 callback DTO 完整定义。

已完成验证：

- 静态检查确认 `RuntimeApplicationCallbackBinder.cpp` 不再直接引用 startup/frame/shutdown lifecycle 或 config policy。
- 静态检查确认 `RuntimeApplicationStartupCallbackBridge.h/.cpp` 已注册到 VS project/filter。
- 静态检查确认 startup lifecycle forwarding 只在 startup callback bridge implementation 中发生。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeApplicationCallbackBinder.cpp` 与 `RuntimeApplicationStartupCallbackBridge.cpp`；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

只读 sidecar 审计：

- `Singer` 审计 callback binder、startup callback bridge、VS project/filter 注册和当前架构文档；未修改文件，未发现代码或工程注册阻塞问题。
- `Singer` 报告的接口设计文档 stale wording 已由父 agent 修正为 startup bridge 已完成，后续转向 callback bridge / bootstrapper include surface audit。

结论：

- 这是 callback binder startup bridge boundary cleanup，不改变 startup/frame/shutdown callback 顺序、startup initialize result propagation、Engine startup、window/graphics/content/editor/frame startup 顺序、renderer backend contract、Engine cleanup verification output 或 PBR pass。
- 后续建议继续审计 `RuntimeApplicationCallbackBinder.cpp` 是否还能隐藏 `RuntimeBootstrapper.h` callback DTO 完整依赖，或回到 Engine public header 低风险 implementation detail audit；当前不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Bootstrapper Callbacks Header Extraction

本轮承接上一轮后续建议，继续 callback/bootstrapper include surface audit。审计确认：`RuntimeApplicationCallbackBinder.cpp` 与 `RuntimeApplicationShell.cpp` 只需要构造或返回 `RuntimeBootstrapperCallbacks`，不需要完整 `RuntimeBootstrapper` runner facade；`RuntimeApplicationRunner.cpp` 才是真正同时调用 bootstrapper runner 并消费 callback DTO 的 composition root。

新增与修改：

- 新增 `RuntimeBootstrapperCallbacks.h`，集中定义 `RuntimeBootstrapperCallbacks` 和其 `std::function` 字段。
- `RuntimeBootstrapper.h` 移除 `<functional>` 与 callback DTO 定义，只 forward declare `RuntimeBootstrapperCallbacks` 并保留 `RuntimeBootstrapper::run(...)` runner facade。
- `RuntimeBootstrapper.cpp` 显式 include `RuntimeBootstrapperCallbacks.h`，因为 implementation 会读取 callback 字段并执行启动/循环/cleanup/destroy 顺序。
- `RuntimeApplicationCallbackBinder.cpp` 和 `RuntimeApplicationShell.cpp` 改为 include `RuntimeBootstrapperCallbacks.h`，不再 include `RuntimeBootstrapper.h`。
- `RuntimeApplicationRunner.cpp` 显式 include `RuntimeBootstrapper.h` 与 `RuntimeBootstrapperCallbacks.h`，因为它既调用 runner facade，也消费 `shell.makeCallbacks()` 返回的完整 DTO。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 callback DTO header 到 Application 分组。

已完成验证：

- 静态检查确认 `RuntimeBootstrapperCallbacks` 只有 `RuntimeBootstrapperCallbacks.h` 定义，`RuntimeBootstrapper.h` 只 forward declare DTO。
- 静态检查确认 `RuntimeApplicationCallbackBinder.cpp` 与 `RuntimeApplicationShell.cpp` 不再 include `RuntimeBootstrapper.h`，只依赖 callback DTO 头。
- 静态检查确认 `RuntimeApplicationRunner.cpp` 是同时 include runner facade 与 callback DTO 的 composition root，VS project/filter 已注册新增 header。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeApplicationCallbackBinder.cpp`、`RuntimeApplicationRunner.cpp`、`RuntimeApplicationShell.cpp` 与 `RuntimeBootstrapper.cpp`；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 bootstrapper callback DTO header extraction，不改变 initialize / shouldContinue / runFrame / cleanup / destroy 执行顺序、callback result propagation、runtime frame loop、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续审计 callback/bootstrapper include surface 的剩余可下沉点，或回到 Engine public header 低风险 implementation detail audit；当前不建议继续扩张 PBR 功能。

### 2026-05-31 RendererSubsystem Implementation State Header Boundary Cleanup

本轮回到 Engine public header 低风险 implementation detail audit。审计确认：`RendererSubsystem.h` 仍通过按值成员暴露 backend slot、frame execution bridge 和 frame bridge state 的完整实现头。这个边界会让任何 include `RendererSubsystem.h` 的调用点间接依赖 renderer subsystem 内部实现，和当前“public header 只保留稳定 API / implementation headers 下沉到 `.cpp`”的方向不一致。

新增与修改：

- `RendererSubsystem.h` 移除 `RendererBackend.h`、`RendererBackendRegistryTypes.h`、`RendererSubsystemBackendSlot.h`、`RendererSubsystemFrameExecutionBridge.h` 与 `RendererSubsystemFrameBridgeState.h` 的 public include，改为 forward declare `RendererBackend`、`RendererBackendAttachmentDesc`、`RendererFrameIntent`、`RendererSubsystemFrameBridgeStats` 和三个内部 implementation type。
- `RendererSubsystem` 的 backend slot、frame execution bridge、frame bridge state 从按值成员改为 private `std::unique_ptr` owning pointers，并把 destructor 改为 out-of-line，以保证 incomplete type 下的 `unique_ptr` 析构安全。
- `RendererSubsystem.cpp` 显式 include 完整 implementation headers，并在 constructor 中通过 `std::make_unique` 构造三个内部组件；所有内部访问从 `.` 调整为 `->`。
- `setRendererBackend(std::unique_ptr<RendererBackend>)` 新增一参 overload，用 `RendererBackendAttachmentDesc{}` 保持原先 default attachment desc 语义；带 desc 的 overload 保留。
- `RuntimeRendererBackendAttachmentLifecycle.cpp` 显式 include `RendererBackend.h`，因为该路径创建并转移 `std::unique_ptr<RendererBackend>`，需要完整 backend 类型。
- `EngineDiagnosticsPanel.cpp` 显式 include `RendererSubsystemFrameBridgeStats.h`，因为 diagnostics UI 读取 stats 字段，不能再依赖 `RendererSubsystem.h` 的传递 include。
- 响应 `/subagents` 要求，启动只读 sidecar `Einstein` 审计本轮 header boundary、incomplete-type safety 和文档一致性；`Einstein` 未修改文件，未运行写输出测试，确认源码无 blocker，并指出 `work.md` / `worked.md` 需要补本轮记录。

已完成验证：

- 初次 focused build 暴露两个合理的 include 边界 fallout：`RuntimeRendererBackendAttachmentLifecycle.cpp` 需要完整 `RendererBackend`，`EngineDiagnosticsPanel.cpp` 需要完整 `RendererSubsystemFrameBridgeStats`；已改为显式 include 对应 owner header。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：修正 include 后构建通过；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 RendererSubsystem public header implementation-state cleanup，不改变 renderer backend contract、frame bridge stats 语义、frame execution、Engine World verification 或 PBR pass。
- 后续建议继续 Engine public header 的低风险 implementation detail audit，或回到 callback/bootstrapper include surface audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Frame Callback Default Argument Header Boundary Cleanup

本轮继续 callback/bootstrapper include surface audit。审计确认：`RuntimeFrameRunner.h` 与 `RuntimeFrameLifecycle.h` 仍为了 `const RuntimeFrameCallbacks& callbacks = {}` 默认参数 include 完整 `RuntimeFrameCallbacks.h`。这会让只需要 runner/lifecycle facade 的调用点被 callback DTO 的完整定义污染。更窄的边界是：public headers 暴露无 callback overload 与显式 callback overload，完整 callback DTO 只在 implementation 中构造或读取。

新增与修改：

- `RuntimeFrameRunner.h` 移除 `RuntimeFrameCallbacks.h` include，改为 forward declare `RuntimeFrameCallbacks`。
- `RuntimeFrameRunner.h/.cpp` 新增无 callback overload：无 callback 入口在 `.cpp` 内构造空 `RuntimeFrameCallbacks{}` 后转发到显式 callback overload。
- `RuntimeFrameLifecycle.h` 移除 `RuntimeFrameCallbacks.h` include，改为 forward declare `RuntimeFrameCallbacks`。
- `RuntimeFrameLifecycle.h/.cpp` 新增无 callback `runFrame(...)` overload：无 callback 入口在 `.cpp` 内构造空 callback DTO 后转发到显式 callback overload。
- `RuntimeFrameRunner.cpp` 与 `RuntimeFrameLifecycle.cpp` 显式 include `RuntimeFrameCallbacks.h`，因为 implementation 需要构造和读取 callback DTO。
- 初次 focused build 暴露 `RuntimeApplicationFrameRunBridge.cpp` 需要完整 `RuntimeFrameCallbacks`：该实现文件通过 `RuntimeApplicationFrameEditorCallbackBridge::makeFrameCallbacks(...)` 构造 editor callback DTO 并传给 frame lifecycle；已补显式 include `RuntimeFrameCallbacks.h`。

已完成验证：

- 静态检查确认 `RuntimeFrameRunner.h` 与 `RuntimeFrameLifecycle.h` 不再 include `RuntimeFrameCallbacks.h`。
- 静态检查确认 `RuntimeFrameRunner.h` 与 `RuntimeFrameLifecycle.h` 不再使用 `callbacks = {}` 默认参数。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：修正 frame run bridge 显式 include 后构建通过；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime frame callback DTO header boundary cleanup，不改变 frame loop、GUI callback gating、editor callback construction、runtime renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续 callback/bootstrapper include surface audit，或回到 Engine public header 低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Application Shell Config Header Boundary Cleanup

本轮继续 callback/bootstrapper include surface audit。审计确认：`RuntimeApplicationShell.h` 仍为了按值持有 `RuntimeApplicationShellConfig` include 完整 `RuntimeApplicationConfig.h`，导致 shell facade 用户被完整 application config aggregate 污染。更窄边界是：shell header forward declare config，shell implementation 拥有完整 config 依赖。

新增与修改：

- `RuntimeApplicationShell.h` 移除 `RuntimeApplicationConfig.h` include，新增 `RuntimeApplicationShellConfig` forward declaration。
- `RuntimeApplicationShell` 的 config 成员从按值持有改为 `std::unique_ptr<RuntimeApplicationShellConfig>`，析构/移动仍保持 out-of-line。
- `RuntimeApplicationShell.cpp` 显式 include `RuntimeApplicationConfig.h`，default ctor 构造默认 config，config ctor move 到 owning pointer。
- `makeCallbacks()` 解引用 `*mConfig` 传给 `RuntimeApplicationCallbackBinder::makeCallbacks(...)`。
- `RuntimeApplicationRunner.cpp` 仍显式 include `RuntimeApplicationConfig.h`，因为 composition root 按值接收并移动 shell config。

已完成验证：

- 静态检查确认 `RuntimeApplicationShell.h` 不再 include `RuntimeApplicationConfig.h`。
- 静态检查确认 `RuntimeApplicationShell.h` 通过 `std::unique_ptr<RuntimeApplicationShellConfig>` 隐藏完整 config 类型，`RuntimeApplicationShell.cpp` 负责 `std::make_unique<RuntimeApplicationShellConfig>` 构造。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeApplicationRunner.cpp` 与 `RuntimeApplicationShell.cpp`；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime application shell config header boundary cleanup，不改变 bootstrapper callback construction、startup/frame/shutdown callback 顺序、runtime frame loop、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续 callback/bootstrapper include surface audit，或回到 Engine public header 低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Engine World Persistent Level Header Boundary Cleanup

本轮转向 Engine public header 的低风险 implementation detail audit。审计确认：`World.h` 仍为了 `std::unique_ptr<Level>` 持有 persistent level 而 include 完整 `Level.h`，这会把 `Level` / `Actor` / `SceneComponent` 的模板创建表面传递给所有只需要 World facade 的调用点。更窄边界是：`World.h` 只 forward declare `Level`，完整 Level 依赖由 `World.cpp` 和实际读取 level 字段/方法的调用点显式持有。

新增与修改：

- `World.h` 移除 `Level.h` include，新增 `class Level;` forward declaration。
- `World` destructor 从 header inline default 改为 out-of-line `~World() override;`，保证 incomplete `std::unique_ptr<Level>` 析构安全。
- `World.cpp` 显式 include `Level.h`，并定义 `World::~World() = default;`。
- `RuntimeImportedAssetVerification.cpp` 显式 include `Level.h`，因为该文件通过 `getPersistentLevel()` 访问 Level API，不再依赖 `World.h` 的传递 include。

已完成验证：

- 静态检查确认 `World.h` 不再 include `Level.h`，只保留 `class Level` 和 `std::unique_ptr<Level>`。
- 初次 focused verification 命令使用了不存在的 mode 名 `verify-pbr-import`，脚本明确正确 mode 为 `import`；该命令不作为验证证据。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `World.cpp`、`Level.cpp`、`ScenePackage.cpp`、`RuntimeImportedAssetVerification.cpp`、Engine World/editor/scene setup 相关实现；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine World persistent level public header boundary cleanup，不改变 World ownership、persistent level lifecycle、scene package/import verification、Engine World verification、renderer backend contract 或 PBR pass。
- 后续建议继续 Engine public header 的低风险 implementation detail audit，或回到 callback/bootstrapper include surface audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Engine Actor Root SceneComponent Header Boundary Cleanup

本轮继续 Engine public header 的低风险 implementation detail audit。审计确认：`Actor.h` 只通过 `SceneComponent*` 暴露 root component API，但仍 public include 完整 `SceneComponent.h`，导致只需要 Actor facade 的调用点也获得 SceneComponent/Transform surface。更窄边界是：`Actor.h` forward declare `SceneComponent`，完整类型依赖只留在 `Actor.cpp` 的 register/dynamic_cast 实现和真正访问 SceneComponent API 的调用点。

新增与修改：

- `Actor.h` 移除 `SceneComponent.h` include，新增 `class SceneComponent;` forward declaration。
- `Actor.h` 保持 root component pointer API 不变：`getRootComponent()`、`setRootComponent(...)` 和 `mRootComponent` 仍使用 `SceneComponent*`。
- `Actor.cpp` 显式 include `SceneComponent.h`，因为 `registerComponent(...)` 内部需要 `dynamic_cast<SceneComponent*>`。

已完成验证：

- 静态检查确认 `Actor.h` 不再 include `SceneComponent.h`，只保留 forward declaration 与 pointer API。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `Actor.cpp`、`ActorAdapters.cpp`、`ScenePackage.cpp`、Engine World/editor/scene setup 相关实现；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine Actor root SceneComponent public header boundary cleanup，不改变 Actor/component ownership、root component registration、scene package/import verification、Engine World verification、renderer backend contract 或 PBR pass。
- 后续建议继续 Engine public header 的低风险 implementation detail audit，或回到 callback/bootstrapper include surface audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Engine AssetSubsystem Registry Header Boundary Cleanup

本轮继续 Engine public header 的低风险 implementation detail audit。审计确认：`AssetSubsystem.h` 为了按值持有 `AssetRegistry` public include 完整 `AssetRegistry.h`，导致只需要 AssetSubsystem facade 的调用点也获得 registry storage/API surface。更窄边界是：`AssetSubsystem.h` forward declare `AssetRegistry`，registry 由 private owning pointer 持有，完整 registry 依赖只留在 subsystem implementation 和真正访问 registry API 的调用点。

新增与修改：

- `AssetSubsystem.h` 移除 `AssetRegistry.h` include，新增 `class AssetRegistry;` forward declaration。
- `AssetSubsystem` 的 registry 成员从按值持有改为 `std::unique_ptr<AssetRegistry>`，构造/析构改为 out-of-line，保证 incomplete type 析构安全。
- `AssetSubsystem.cpp` 显式 include `AssetRegistry.h`，负责 `std::make_unique<AssetRegistry>()` 构造、`getRegistry()` 解引用和 `clear()` cleanup。
- `RuntimeVerificationReport.cpp` 显式 include `AssetRegistry.h`，因为 cleanup report 读取 `assetSubsystem->getRegistry().count()`，不再依赖 `AssetSubsystem.h` 的传递 include。

已完成验证：

- 静态检查确认 `AssetSubsystem.h` 不再 include `AssetRegistry.h`，只保留 forward declaration、`std::unique_ptr<AssetRegistry>` 和 registry facade API。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：构建通过；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine AssetSubsystem registry public header boundary cleanup，不改变 asset registry ownership semantics、lifecycle cleanup、import/package verification、Engine subsystem cleanup、renderer backend contract 或 PBR pass。
- 后续建议继续 Engine public header 的低风险 implementation detail audit，或回到 callback/bootstrapper include surface audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Renderer Backend Frame Types Header Extraction

本轮转向通用 renderer backend contract/header surface audit。审计确认：`RendererFrameIntent` 与 `RendererFrameResult` 仍定义在完整 `RendererBackend.h` 中，导致 frame execution bridge 等只需要 frame DTO 的 public headers 必须 include 完整 backend interface；同时 `RendererSubsystemFrameBridgeState.h` 为了 `RendererSubsystemBackendSlotSnapshot` reference 传递了完整 backend slot header。更窄边界是：frame DTO 进入轻量 types 头，bridge public headers 只依赖轻量 DTO 或 forward declaration，完整 backend/slot 依赖下沉到 implementation。

新增与修改：

- 新增 `RendererBackendFrameTypes.h`，承载 `RendererFrameIntent` 与 `RendererFrameResult`。
- `RendererBackend.h` 改为 include `RendererBackendFrameTypes.h`，继续只负责 `RendererBackend` interface。
- `RendererSubsystemFrameExecutionBridge.h` 不再 include 完整 `RendererBackend.h`，只 include frame types 并 forward declare `RendererBackend`。
- `RendererSubsystemFrameExecutionBridge.cpp` 显式 include `RendererBackend.h`，因为执行阶段需要调用 virtual `renderFrame(...)`。
- `RendererSubsystemFrameBridgeState.h` 不再 include `RendererSubsystemBackendSlot.h`，改为 forward declare `RendererSubsystemBackendSlotSnapshot` 和 frame DTO；完整 slot/frame DTO 依赖下沉到 `.cpp`。
- `RuntimeFrameRunner.cpp` 显式 include `RendererBackendFrameTypes.h`，因为它实际构造 `RendererFrameIntent`。
- `text2.vcxproj` 与 `text2.vcxproj.filters` 注册新增 header。

已完成验证：

- 静态检查确认 `RendererBackendFrameTypes.h` 已注册，`RendererSubsystemFrameExecutionBridge.h` 与 `RendererSubsystemFrameBridgeState.h` 不再 include 完整 `RendererBackend.h`。
- 静态检查确认 `RendererSubsystemFrameBridgeState.h` 不再 include `RendererSubsystemBackendSlot.h`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeFrameRunner.cpp`、`RuntimeNoOpRendererBackend.cpp`、`RuntimeRendererFrameBridgeAdapter.cpp`、`RendererSubsystem.cpp`、`RendererSubsystemBackendSlot.cpp`、`RendererSubsystemFrameBridgeState.cpp` 和 `RendererSubsystemFrameExecutionBridge.cpp`；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 renderer backend frame DTO header extraction，不改变 renderer backend virtual contract、runtime frame bridge execution、backend registry/no-op backend behavior、Engine World verification 或 PBR pass。
- 后续建议继续通用 renderer backend contract/header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 RendererSubsystem Backend Slot Header Boundary Cleanup

本轮继续通用 renderer backend contract/header surface audit。审计确认：`RendererSubsystemBackendSlot.h` 为了 `std::unique_ptr<RendererBackend>` 持有 backend owner public include 完整 `RendererBackend.h`，导致只需要 backend slot facade/snapshot 的调用点也获得完整 backend virtual interface。更窄边界是：slot header forward declare `RendererBackend`，析构下沉到 `.cpp`，完整 backend 依赖只留在实际调用 `getBackendKey()` / `isBackendReady()` / ownership handoff 的 implementation。

新增与修改：

- `RendererSubsystemBackendSlot.h` 移除 `RendererBackend.h` include，新增 `class RendererBackend;` forward declaration。
- `RendererSubsystemBackendSlot` 新增 out-of-line destructor，保证 `std::unique_ptr<RendererBackend>` incomplete type 析构安全。
- `RendererSubsystemBackendSlot.cpp` 显式 include `RendererBackend.h`，因为 implementation 读取 backend key、ready state 并销毁 backend owner。
- `RendererSubsystem.cpp` 显式 include `RendererBackend.h`，因为该 implementation 按值接收 `std::unique_ptr<RendererBackend>` 并触发参数析构。

已完成验证：

- 静态检查确认 `RendererSubsystemBackendSlot.h` 不再 include `RendererBackend.h`，只保留 forward declaration、owning pointer 和 slot facade API。
- 初次 focused verification 暴露合理 include fallout：`RendererSubsystem.cpp` 按值接收 `std::unique_ptr<RendererBackend>`，需要完整 `RendererBackend`；已补显式 include 后重跑通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：修正 implementation include 后构建通过；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 RendererSubsystem backend slot header boundary cleanup，不改变 renderer backend virtual contract、backend ownership semantics、runtime frame bridge execution、backend registry/no-op backend behavior、Engine World verification 或 PBR pass。
- 后续建议继续通用 renderer backend contract/header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Renderer Backend Catalog Registry Header Boundary Cleanup

本轮继续通用 renderer backend contract/header surface audit。审计确认：`RuntimeRendererBackendCatalog.h` 为了 backend catalog facade public include 完整 `RendererBackendRegistry.h`，但外部调用方主要需要 backend key、registered backend DTO、selection 和 attachment desc；完整 registry 构造与查询只发生在 catalog implementation 内。更窄边界是：catalog header 只 include `RendererBackendRegistryTypes.h` 并 forward declare `RendererBackendRegistry`，完整 registry 依赖下沉到 `.cpp`。

新增与修改：

- `RuntimeRendererBackendCatalog.h` 移除 `RendererBackendRegistry.h` include，改为 include `RendererBackendRegistryTypes.h`。
- `RuntimeRendererBackendCatalog.h` 新增 `class RendererBackendRegistry;` forward declaration，保留现有 catalog public API。
- `RuntimeRendererBackendCatalog.cpp` 显式 include `RendererBackendRegistry.h`，因为 implementation 构造 registry 并调用 registry 查询 API。

已完成验证：

- 静态检查确认 `RuntimeRendererBackendCatalog.h` 不再 include `RendererBackendRegistry.h`，只传播 registry DTO/types 与 registry forward declaration。
- 静态检查确认 `RuntimeRendererBackendCatalog.cpp` 保留完整 `RendererBackendRegistry.h` include，attachment lifecycle 调用方继续只使用 catalog facade。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeRendererBackendCatalog.cpp` 和 `RuntimeRendererBackendAttachmentLifecycle.cpp`；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Runtime renderer backend catalog registry header boundary cleanup，不改变 backend key、selection fallback、attachment desc、registry/no-op backend behavior、Engine World verification 或 PBR pass。
- 后续建议继续通用 renderer backend contract/header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Renderer Backend Catalog Registry Object API Cleanup

本轮继续上一轮 catalog header surface cleanup。审计确认：`RuntimeRendererBackendCatalog::makeRegistry()` 只在 `RuntimeRendererBackendCatalog.cpp` 内部被 `registeredBackends()`、`isRegisteredBackendKey(...)`、`resolveBackendSelection(...)` 和 `makeRendererSubsystemAttachmentDesc(...)` 使用，外部源代码没有调用该 API。继续把它留在 public header 会让 catalog facade 公开具体 `RendererBackendRegistry` object，边界仍偏宽。更窄边界是：`makeRegistry()` 变为 implementation-local helper，public header 只保留 key/query/selection/attachment facade。

新增与修改：

- `RuntimeRendererBackendCatalog.h` 删除 `RuntimeRendererBackendCatalog::makeRegistry()` public declaration。
- `RuntimeRendererBackendCatalog.h` 删除 `RendererBackendRegistry` forward declaration，header 不再暴露 concrete registry object type。
- `RuntimeRendererBackendCatalog.cpp` 新增 anonymous-namespace `makeRegistry()` helper，保留原有 backend registration 列表、selection fallback 和 attachment desc 行为。

已完成验证：

- 静态检查确认 `RuntimeRendererBackendCatalog.h` 不再暴露 `makeRegistry()` 或 `RendererBackendRegistry` object type，只保留轻量 registry DTO/types facade。
- 静态检查确认外部源代码没有 `RuntimeRendererBackendCatalog::makeRegistry` 调用；`registeredBackends()` 与 `isRegisteredBackendKey(...)` 仍作为轻量 query facade 保留。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeRendererBackendCatalog.cpp` 和 `RuntimeRendererBackendAttachmentLifecycle.cpp`；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Runtime renderer backend catalog registry object API cleanup，不改变 backend key、selection fallback、attachment desc、registry/no-op backend behavior、Engine World verification 或 PBR pass。
- 后续建议继续通用 renderer backend contract/header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Engine Level Actor Header Boundary Cleanup

本轮回到 Engine public header 的低风险 include audit。审计确认：`Level.h` 为了 actor owner list 和 `spawnActor<T>` public include 完整 `Actor.h`，导致只需要 Level facade 或 actor count 的调用点也继承完整 Actor API。更窄边界是：`Level.h` forward declare `Actor`，析构下沉到 `.cpp`，完整 actor lifecycle/ownership 操作只留在 `Level.cpp` 和实际构造具体 actor 类型的调用点。

新增与修改：

- `Level.h` 移除 `Actor.h` include，新增 `class Actor;` forward declaration。
- `Level` destructor 从 inline default 改为 out-of-line，保证 `std::vector<std::unique_ptr<Actor>>` incomplete type 析构安全。
- `Level.cpp` 显式 include `Actor.h`，因为 implementation 调用 actor lifecycle、ownership handoff 和 actor method。

已完成验证：

- 静态检查确认 `Level.h` 不再 include `Actor.h`，只保留 forward declaration、`std::unique_ptr<Actor>` owner list 和 `spawnActor<T>` facade。
- 静态检查确认当前 `spawnActor<T>` 调用点已经显式 include `Actor.h` 或 `ActorAdapters.h`，不依赖 `Level.h` 的传递 include。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `Level.cpp`、`World.cpp`、`ScenePackage.cpp`、`LegacySceneWorldBuilder.cpp`、Engine World verification 和 editor world/snapshot 相关实现；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine Level actor public header boundary cleanup，不改变 World/Level/Actor ownership semantics、spawnActor 行为、scene package/import verification、Engine World verification、renderer backend contract 或 PBR pass。
- 后续建议继续 Engine public header 的低风险 implementation detail audit，或回到通用 renderer backend contract/header surface audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Engine Legacy Scene Transform Header Boundary Cleanup

本轮继续 Engine public header 的低风险 include audit。审计确认：`LegacySceneWorldBuilder.h` 和 `WorldLegacySceneExporter.h` 为了声明 transform 参数/返回类型直接 include 完整 `Transform.h`，但完整 transform 字段访问只发生在 import/export implementation 内。更窄边界是：public headers 只 forward declare `Transform`，完整 `Transform.h` 依赖下沉到 `.cpp`。

新增与修改：

- `LegacySceneWorldBuilder.h` 移除 `Transform.h` include，新增 `struct Transform;` forward declaration，保留 `makeTransform(...)` facade。
- `WorldLegacySceneExporter.h` 移除 `Transform.h` include，新增 `struct Transform;` forward declaration，保留 `applyTransform(...)` facade。
- `LegacySceneWorldBuilder.cpp` 与 `WorldLegacySceneExporter.cpp` 显式 include `Transform.h`，因为 implementation 读取/写入 transform location/rotation/scale 字段。

已完成验证：

- 静态检查确认 legacy scene public headers 不再 include `Transform.h`，implementation 显式 include 完整 `Transform.h`。
- 静态检查确认当前 `WorldDrivenSceneSetup.cpp` 和 `RuntimeEngineWorldVerification.cpp` 等实际构造 `Transform` 的调用点仍显式 include `Transform.h`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `LegacySceneWorldBuilder.cpp`、`WorldLegacySceneExporter.cpp`、scene setup pipeline 和 Engine World verification 相关实现；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine legacy scene transform public header boundary cleanup，不改变 legacy scene import/export semantics、Transform 数据结构、Engine World minimal scene、scene package/import verification、renderer backend contract 或 PBR pass。
- 后续建议继续 Engine public header 的低风险 implementation detail audit，或回到通用 renderer backend contract/header surface audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Engine ScenePackage Load Result World Owner Boundary Cleanup

本轮继续 Engine public header 的低风险 ownership boundary audit。审计确认：`ScenePackage.h` 已 forward declare `World`，但 `ScenePackageLoadResult` 直接按值持有 `std::unique_ptr<World>` 且使用隐式析构/移动 special members。更窄边界是：在 header 中显式声明 move-only special members，把析构和 move default 定义下沉到 `ScenePackage.cpp`，由 implementation 侧完整 include `World.h`。

新增与修改：

- `ScenePackageLoadResult` 显式声明 default constructor、out-of-line destructor、move constructor 和 move assignment。
- `ScenePackageLoadResult` 显式删除 copy constructor 和 copy assignment，保持 load result 的 move-only ownership 语义。
- `ScenePackage.cpp` 在包含完整 `World.h` 的 translation unit 中 default `ScenePackageLoadResult` special members，避免调用方通过 header 承担 incomplete `World` owner 析构。

已完成验证：

- 静态检查确认 `ScenePackage.h` 不 include `World.h`，`ScenePackage.cpp` 显式 include 完整 `World.h`。
- 静态检查确认 `loadScenePackage(...)` 调用点继续以返回值或局部 `ScenePackageLoadResult` 使用 move-only result。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,engine-world-minimal-scene,engine-world-scene-package,import,renderer-backend-registry-noop -DiscardCaptures`：构建通过；MSBuild 明确编译 `ScenePackage.cpp`、Engine World verification 和 imported asset verification；五条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine ScenePackage load result World owner boundary cleanup，不改变 scene package save/load schema、World ownership transfer、package graph validation、imported asset package verification、renderer backend contract 或 PBR pass。
- 后续建议继续 Engine public header 的低风险 implementation detail audit，或回到通用 renderer backend contract/header surface audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Renderer Backend Contract Frame DTO Header Boundary Cleanup

本轮回到通用 renderer backend contract/header surface audit。审计确认：`RendererBackend.h` 为了声明 backend virtual API 直接 include 完整 `RendererBackendFrameTypes.h`，导致只需要 backend interface 的调用点也继承 frame intent/result DTO 的完整定义。更窄边界是：backend contract header 只 forward declare `RendererFrameIntent` / `RendererFrameResult`，实际读取 DTO 字段或构造结果的 concrete backend implementation 显式 include DTO 头。

新增与修改：

- `RendererBackend.h` 移除 `RendererBackendFrameTypes.h` include，改为 forward declare `RendererFrameIntent` 与 `RendererFrameResult`。
- `RuntimeNoOpRendererBackend.cpp` 显式 include `RendererBackendFrameTypes.h`，因为 no-op backend 读取 framebuffer width/height 并返回 frame result aggregate。
- `RuntimeRendererFrameBridgeAdapter.cpp` 显式 include `RendererBackendFrameTypes.h`，因为 runtime adapter 读取 frame intent 并构造 neutral frame result。

已完成验证：

- 静态检查确认 `RendererBackend.h` 不再 include `RendererBackendFrameTypes.h`，完整 DTO include 只保留在 concrete backend implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop,engine-world-minimal-scene,engine-world-scene-package -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeNoOpRendererBackend.cpp`、`RuntimeRendererFrameBridgeAdapter.cpp`、`RendererSubsystem.cpp`、`RendererSubsystemBackendSlot.cpp` 与 `RendererSubsystemFrameExecutionBridge.cpp`；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 renderer backend contract frame DTO include boundary cleanup，不改变 backend virtual API、runtime frame pipeline adapter、no-op backend behavior、renderer backend registry/no-op verification、Engine World verification 或 PBR pass。
- 后续建议继续通用 renderer backend contract/header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Frame Pipeline Context Header Boundary Cleanup

本轮继续 runtime/renderer header surface audit。审计确认：`RuntimeFramePipeline.h` 与 `RuntimeFramePasses.h` 为了 frame pipeline/pass 函数签名直接 include 完整 `AppRuntimeContext.h`，但 public header 只需要引用类型声明；真正读取 `AppRuntimeContext` 与 `RuntimeFramePipelineConfig` 字段的是 `.cpp`。更窄边界是：public headers 只 forward declare context/config，implementation 显式 include 完整依赖。

新增与修改：

- `RuntimeFramePipeline.h` 移除 `AppRuntimeContext.h` include，改为 forward declare `GLframework::AppRuntimeContext`。
- `RuntimeFramePipeline.cpp` 显式 include `AppRuntimeContext.h`，因为 implementation 读取 `context.profiles.framePipelineProfile`。
- `RuntimeFramePasses.h` 移除 `AppRuntimeContext.h` 与 `RuntimeFramePipeline.h` include，改为 forward declare `GLframework::AppRuntimeContext` 与 `RuntimeFramePipelineConfig`。
- `RuntimeFramePasses.cpp` 显式 include `AppRuntimeContext.h` 与 `RuntimeFramePipeline.h`，因为 implementation 读取 context resources/profile 字段和 config framebuffer size。
- `RuntimeFramePassRegistry.cpp` 移除不再需要的 `RuntimeFramePipeline.h` include，只保留 registry implementation 实际读取 context profile 所需的完整 context include。

已完成验证：

- 静态检查确认 `RuntimeFramePipeline.h` 与 `RuntimeFramePasses.h` 不再 include 完整 `AppRuntimeContext.h`，完整 context/config include 已下沉到 implementation。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop,engine-world-minimal-scene,engine-world-scene-package -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeFramePassRegistry.cpp`、`RuntimeFramePasses.cpp`、`RuntimeFramePipeline.cpp` 与 `RuntimeRendererFrameBridgeAdapter.cpp`；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime frame pipeline/pass context header boundary cleanup，不改变 runtime frame pass order、frame pass execution、runtime backend adapter frame plan key、renderer backend registry/no-op verification、Engine World verification 或 PBR pass。
- 后续建议继续 runtime/renderer header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Runtime Frame Pass Registry Key String View Boundary Cleanup

本轮继续 runtime/renderer header surface audit。审计确认：`RuntimeFramePassRegistry.h` 的 `findPassByKey(...)` 只做只读 key 查询，却用 `const std::string&` 迫使 registry public header include `<string>`。更窄边界是：public API 使用 `std::string_view` 表达只读非拥有 key view，具体 trim/token 字符串处理保留在 `.cpp`。

新增与修改：

- `RuntimeFramePassRegistry.h` 将 `<string>` include 替换为 `<string_view>`。
- `RuntimeFramePassRegistry::findPassByKey(...)` 参数从 `const std::string&` 改为 `std::string_view`，调用方仍可传入 `std::string` token 或字符串字面量。
- `RuntimeFramePassRegistry.cpp` 保留 `<string>`，并把 anonymous-namespace `trim(...)` 改为接收 `std::string_view` 后在 implementation 内构造 normalized string。

已完成验证：

- 静态检查确认 `RuntimeFramePassRegistry.h` 不再 include `<string>`，只传播 `<string_view>` 与 `<vector>`。
- 静态检查确认现有 `findPassByKey(...)` 调用点只有 registry implementation 内的 pass-order token 路径，`std::string` token 可隐式转换为 `std::string_view`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop,engine-world-minimal-scene,engine-world-scene-package -DiscardCaptures`：构建通过；MSBuild 明确编译 `RuntimeFramePassRegistry.cpp`、`RuntimeFramePipeline.cpp` 与 `RuntimeRendererFrameBridgeAdapter.cpp`；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 runtime frame pass registry key lookup header boundary cleanup，不改变 pass key matching、pass order parsing、runtime backend adapter frame plan key、renderer backend registry/no-op verification、Engine World verification 或 PBR pass。
- 后续建议继续 runtime/renderer header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Engine Lifecycle Snapshot Header Boundary Cleanup

本轮转向 Engine public header 的低风险 include audit。审计确认：`Engine.h` 只是在 public API 中按值返回 `EngineLifecycleSnapshot`，但仍直接 include 完整 `EngineLifecycleSnapshot.h`，导致任何包含 Engine facade 的调用点都会继承 lifecycle snapshot DTO 的完整定义。更窄边界是：`Engine.h` 只 forward declare 返回 DTO，实际构造或读取快照字段的 translation unit 显式 include 完整快照头。

新增与修改：

- `Engine.h` 移除 `EngineLifecycleSnapshot.h` include，改为 forward declare `EngineLifecycleSnapshot`。
- `Engine.cpp` 显式 include `EngineLifecycleSnapshot.h`，因为 `captureLifecycleSnapshot()` 实际构造并填充快照字段。
- `EngineDiagnosticsPanel.cpp` 显式 include `EngineLifecycleSnapshot.h`，因为 diagnostics UI 读取快照字段并构造默认快照。
- `RuntimeVerificationReport.cpp` 显式 include `EngineLifecycleSnapshot.h`，因为 runtime verification report 获取快照并传递给 formatter。

已完成验证：

- 静态检查确认 `Engine.h` 不再 include 完整 `EngineLifecycleSnapshot.h`，只保留 forward declaration 与 `captureLifecycleSnapshot()` 返回声明。
- 静态检查确认 `Engine.cpp`、Engine diagnostics panel 和 runtime verification report 都显式 include 完整快照头。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop,engine-world-minimal-scene,engine-world-scene-package -DiscardCaptures`：构建通过；MSBuild 明确编译 `Engine.cpp`、`EngineDiagnosticsPanel.cpp` 和 `RuntimeVerificationReport.cpp`；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Engine lifecycle snapshot DTO include boundary cleanup，不改变 `Engine::captureLifecycleSnapshot()` API、Engine diagnostics panel、runtime verification output、renderer backend contract、Engine World verification 或 PBR pass。
- 后续建议继续 Engine public header 的低风险 implementation detail audit，或回到 runtime/renderer header surface audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Frame Render Targets Framebuffer Header Boundary Cleanup

本轮回到 runtime/renderer header surface audit。审计确认：`FrameRenderTargets.h` 只需要声明 `std::shared_ptr<Framebuffer>` 成员和 `std::shared_ptr<Texture>` 返回值，却直接 include 完整 `framebuffer/framebuffer.h`，进而把 framebuffer/texture/OpenGL core 细节传播到 `RuntimeRenderResourceState.h`、`RuntimeViewport.h` 和 scene setup headers。更窄边界是：Frame render targets header 只 forward declare `Framebuffer` / `Texture`，实际 FBO 创建和 attachment 访问留在 implementation。

新增与修改：

- `FrameRenderTargets.h` 移除 `framebuffer/framebuffer.h` include。
- `FrameRenderTargets.h` 新增 `Framebuffer` 与 `Texture` forward declarations，继续保持现有 shared pointer API 和 value-state 语义。
- `FrameRenderTargets.cpp` 显式 include `framebuffer/framebuffer.h`，因为 implementation 调用 FBO factory、`getFBO()` 和 attachment accessors。

已完成验证：

- 静态检查确认 `FrameRenderTargets.h` 不再 include 完整 framebuffer header，`FrameRenderTargets.cpp` 显式 include 完整 framebuffer implementation header。
- 静态检查确认 `FrameRenderTargets` 调用点仍覆盖 runtime resource state、viewport 和 scene setup 路径。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop,engine-world-minimal-scene,engine-world-scene-package -DiscardCaptures`：构建通过；MSBuild 明确编译 `FrameRenderTargets.cpp`、runtime frame passes、runtime viewport、scene setup 和 runtime renderer bridge；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 FrameRenderTargets framebuffer/texture include boundary cleanup，不改变 framebuffer 创建/resize、Bloom attachment access、runtime frame pipeline frame plan、renderer backend registry/no-op verification、Engine World verification 或 PBR pass。
- 后续建议继续 runtime/renderer header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 PostProcess Pass Header Boundary Cleanup

本轮继续 runtime/renderer header surface audit。审计确认：`PostProcessPass.h` 为了引用参数直接 include 完整 `framework/shader.h`、`framebuffer/framebuffer.h`、`mesh/mesh.h` 和 `PostProcessSettings.h`，导致 `RuntimeRenderResourceState.h` 只为了按值持有一个 `PostProcessPass` 也被迫继承 framebuffer/mesh/shader/settings 实现依赖。更窄边界是：post-process pass header 只 forward declare 引用参数类型，实际 resolve/composite/bloom 执行依赖留在 implementation。

新增与修改：

- `PostProcessPass.h` 移除完整 framebuffer、mesh、shader 和 post-process settings includes。
- `PostProcessPass.h` 新增 `Framebuffer`、`Mesh`、`Shader`、`Bloom` 和 `PostProcessSettings` forward declarations，保留现有 shared pointer/reference API。
- `PostProcessPass.cpp` 显式 include `framebuffer/framebuffer.h`、`mesh/mesh.h`、`framework/shader.h` 与 `PostProcessSettings.h`，因为 implementation 调用 FBO、mesh material/geometry、shader uniforms 和 settings 字段。

已完成验证：

- 静态检查确认 `PostProcessPass.h` 不再传播完整 renderer implementation headers，`PostProcessPass.cpp` 显式 include 所需完整依赖。
- 静态检查确认现有调用路径集中在 `RuntimeFramePasses.cpp` 和 `RuntimeRenderResourceState.h`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop,engine-world-minimal-scene,engine-world-scene-package -DiscardCaptures`：构建通过；MSBuild 明确编译 `PostProcessPass.cpp`、runtime frame passes 和 runtime renderer bridge；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 PostProcessPass public include boundary cleanup，不改变 resolve/composite/bloom 执行逻辑、runtime frame pipeline frame plan、renderer backend registry/no-op verification、Engine World verification 或 PBR pass。
- 后续建议继续 runtime/renderer header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Bloom Header Framebuffer Boundary Cleanup

本轮继续 runtime/renderer header surface audit。审计确认：`Bloom.h` 只需要声明 Bloom 对象内部持有的 `std::shared_ptr<Framebuffer>` / `std::shared_ptr<Texture>` / `std::shared_ptr<Shader>` / `std::shared_ptr<Geometry>`，但它直接 include 完整 `core.h`、`framebuffer/framebuffer.h`、`framework/geometry.h` 和 `framework/shader.h`，导致任何包含 Bloom facade 的调用点都会继承 OpenGL core、framebuffer、shader 和 geometry 实现细节。更窄边界是：Bloom header 只 forward declare 引用类型，实际 FBO 创建、texture binding、shader uniform 和 quad draw 依赖留在 implementation。

新增与修改：

- `Bloom.h` 移除完整 core、framebuffer、geometry 和 shader includes。
- `Bloom.h` 新增 `Framebuffer`、`Texture`、`Shader` 和 `Geometry` forward declarations，保留现有 shared pointer 成员与 public API。
- `Bloom.cpp` 显式 include `core.h`、`framebuffer/framebuffer.h`、`framework/geometry.h` 与 `framework/shader.h`，因为 implementation 执行 Bloom FBO setup、blur ping-pong、bright extraction、texture binding 和 shader/quad 操作。

已完成验证：

- 静态检查确认 `Bloom.h` 不再 include 完整 Bloom implementation dependency headers，`Bloom.cpp` 显式 include 所需完整依赖。
- 静态检查确认 Bloom 使用路径仍覆盖 scene setup、post-process pass、runtime frame registry/profile/adapter 和 runtime resource state。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop,engine-world-minimal-scene,engine-world-scene-package -DiscardCaptures`：构建通过；MSBuild 明确编译 `Bloom.cpp`、`PostProcessPass.cpp`、scene setup 和 runtime scene setup lifecycle/report；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Bloom public include boundary cleanup，不改变 Bloom FBO 创建、blur ping-pong、bright extraction、post-process composite、runtime frame pipeline frame plan、renderer backend registry/no-op verification、Engine World verification 或 PBR pass。
- 后续建议继续 runtime/renderer header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 Environment Texture Header Boundary Cleanup

本轮继续 runtime/renderer header surface audit。审计确认：`EnvironmentRenderTargets.h` 和 `EnvironmentProfile.h` 只需要在 public API 中声明 `std::shared_ptr<Texture>`，但直接 include 完整 `framework/texture.h`，导致 runtime profile state、scene setup、renderer facade 和 IBL/PBR 调用路径继承 texture/OpenGL 实现细节。更窄边界是：环境资源 public headers 只 forward declare `Texture`，实际创建、绑定和读取 texture id/unit/target 的 implementation 显式 include 完整 texture 头。

新增与修改：

- `EnvironmentRenderTargets.h` 移除 `framework/texture.h` include，新增 `Texture` forward declaration，保留现有 environment map / irradiance / prefilter / BRDF LUT shared pointer API。
- `EnvironmentProfile.h` 移除 `framework/texture.h` include，新增 `Texture` forward declaration，保留 `EnvironmentTextureLoader` 返回 texture handle 的 public API。
- `EnvironmentRenderTargets.cpp` 与 `EnvironmentProfile.cpp` 显式 include `framework/texture.h`，因为 implementation 创建 HDR cubemap、float texture、HDR/procedural equirectangular texture 并写入 texture metadata。
- `IBLDebugPass.cpp`、`IBLPrecomputePass.cpp`、`PBRDeferredLightingPass.cpp` 和 `PBRIBLResourceBinder.cpp` 显式 include `framework/texture.h`，因为这些 consumer 实际解引用 environment texture handle、读取 unit/target/id 或调用 `Bind()`。

已完成验证：

- 静态检查确认 `EnvironmentRenderTargets.h` 与 `EnvironmentProfile.h` 不再 include 完整 texture header，只保留 `Texture` forward declaration。
- 第一次 focused verification 暴露 `IBLDebugPass.cpp` 与 `PBRDeferredLightingPass.cpp` 仍依赖从 environment headers 间接获得完整 `Texture`；已改为在实际 consumer `.cpp` 中显式 include `framework/texture.h`，没有退回宽头。
- 修正后 focused verification 通过：`powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop,engine-world-minimal-scene,engine-world-scene-package -DiscardCaptures`；MSBuild 明确编译 `EnvironmentProfile.cpp`、`EnvironmentRenderTargets.cpp`、`IBLDebugPass.cpp`、`IBLPrecomputePass.cpp`、`PBRDeferredLightingPass.cpp`、`PBRIBLResourceBinder.cpp`、runtime profile loader 和 scene setup 相关 translation units；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 Environment texture public include boundary cleanup，不改变 environment target allocation、HDR/procedural texture loading、IBL debug/precompute、PBR IBL binding、deferred lighting、runtime frame pipeline frame plan、renderer backend registry/no-op verification、Engine World verification 或 PBR pass。
- 后续建议继续 runtime/renderer header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

### 2026-05-31 IBL Precompute Pass Header Boundary Cleanup

本轮继续 runtime/renderer header surface audit。审计确认：`IBLPrecomputePass.h` 为了引用参数和一个 private capture-view helper 直接 include 完整 `EnvironmentRenderTargets.h`、`framework/texture.h`、`mesh/mesh.h` 与 `ShaderLibrary.h`，并通过 private `std::array<glm::mat4, 6>` helper 把 GLM capture-view 类型暴露到 public header。更窄边界是：IBL precompute pass header 只 forward declare 参数类型，capture projection/view helper 和实际 environment/texture/shader/mesh 执行依赖留在 implementation。

新增与修改：

- `IBLPrecomputePass.h` 移除 `<array>`、`EnvironmentRenderTargets.h`、`framework/texture.h`、`mesh/mesh.h` 与 `ShaderLibrary.h` includes。
- `IBLPrecomputePass.h` 新增 `EnvironmentRenderTargets`、`Texture`、`Mesh`、`Shader` 与 `ShaderLibrary` forward declarations，保留现有 precompute public API。
- `IBLPrecomputePass.h` 移除 private `createCaptureViews()` 声明，避免 header 暴露 `glm::mat4`。
- `IBLPrecomputePass.cpp` 显式 include `<array>`、`framework/shader.h`、`framework/texture.h`、`renderer/EnvironmentRenderTargets.h`、`renderer/MeshDraw.h` 与 `renderer/ShaderLibrary.h`，并把 `createCaptureViews()` 改为 anonymous-namespace helper。

已完成验证：

- 静态检查确认 `IBLPrecomputePass.h` 不再 include 完整 environment targets、texture、mesh、shader library headers，也不再出现 `<array>` 或 `glm::mat4` private helper。
- 静态检查确认 `IBLPrecomputePass.cpp` 显式持有完整 precompute execution dependencies，renderer facade 仍按值持有 `IBLPrecomputePass`。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -NoLinkDebugInfo -Modes forward,renderer-backend-registry-noop,engine-world-minimal-scene,engine-world-scene-package -DiscardCaptures`：构建通过；MSBuild 明确编译 `IBLPrecomputePass.cpp`、`renderer.cpp`、runtime verification、scene setup 和 RendererSubsystem 相关 translation units；四条 focused verification mode 全部通过。
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools\verify_pbr.ps1 -SkipBuild -DiscardCaptures`：默认 34 个 verification mode 全部通过。

结论：

- 这是 IBL precompute pass public include boundary cleanup，不改变 environment cubemap capture、irradiance convolution、prefilter map、BRDF LUT、runtime frame pipeline frame plan、renderer backend registry/no-op verification、Engine World verification 或 PBR pass。
- 后续建议继续 runtime/renderer header surface audit，或回到 Engine public header 的低风险 implementation detail audit；当前仍不建议继续扩张 PBR 功能。

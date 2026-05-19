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

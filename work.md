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

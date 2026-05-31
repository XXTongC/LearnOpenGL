# AIGC 引擎推进范式

## 当前策略

用户已明确要求：本阶段暂不走人工审核阻塞流程，由 AIGC 自主完成 UE5 启发式框架设计并开始推进。

因此本项目后续 AIGC 规则调整为：

- 可以自主设计和实现。
- 必须每轮检查当前 worktree。
- 必须保持小步可验证。
- 必须更新 `worked.md`。
- 发现方向性冲突时记录在文档中，而不是停在口头讨论。
- 仍然不得提交 `imgui.ini`。
- 仍然不得覆盖用户未提交的无关改动。

## 自主推进边界

AIGC 可以自主执行：

- 新增 UE5 启发式框架骨架。
- 新增文档。
- 小范围接口调整。
- 工程文件注册。
- 构建修复。
- 将旧 renderer 逐步包入 subsystem。

AIGC 暂不应自主执行：

- 删除旧 renderer。
- 删除旧 `Object / Mesh / Material`。
- 大规模重写 `main.cpp`。
- 把 PBR 扩张为项目中心。
- 修改 Git 远程或覆盖 master。

## 每轮必须记录

`worked.md` 至少记录：

- 本轮目标。
- 修改文件。
- 验证方式。
- 未完成项。
- 是否存在本地不应提交文件。

## 验证优先级

文档修改：

- 文件存在。
- 引用一致。
- `worked.md` 已记录。

代码骨架：

- 工程文件包含新增 `.cpp/.h`。
- `Debug|x64` 构建通过。

Renderer 相关：

- 至少跑 focused verification。
- 必要时跑 `tools/verify_pbr.ps1 -SkipBuild -DiscardCaptures`。

## 设计基准

后续所有架构判断以 UE5 启发式 framework 为基准：

- Engine lifecycle。
- World / Level ownership。
- Actor placement and ticking。
- Component composition。
- Subsystem service model。

Renderer 是 subsystem，不是项目中心。


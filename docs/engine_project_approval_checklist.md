# 自主执行检查清单

## 状态

```text
Superseded by autonomous execution
```

本文件原本用于等待用户审核。根据最新目标，本阶段不再等待审核，直接按 UE5 启发式框架推进。

## 当前必须满足的执行条件

| 编号 | 条件 | 状态 |
| --- | --- | --- |
| C-01 | 项目方向改为 UE5 启发式框架 | Active |
| C-02 | Renderer 不再作为项目中心 | Active |
| C-03 | 第一批代码只做非侵入式 framework skeleton | Active |
| C-04 | 不修改现有启动行为 | Active |
| C-05 | 不提交 `imgui.ini` | Active |
| C-06 | `RuntimePBRVerificationArgs.cpp` 本地改动暂不覆盖 | Active |

## 当前执行中的 Phase 1

Phase 1 目标：

- `EngineObject`
- `Engine`
- `World`
- `Level`
- `Actor`
- `ActorComponent`
- `SceneComponent`
- `EngineSubsystem`

通过条件：

- 工程构建通过。
- `worked.md` 记录。
- 现有 renderer 未被重写。


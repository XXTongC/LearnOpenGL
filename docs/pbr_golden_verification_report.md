# PBR Golden Verification Report

生成时间：2026-05-21 23:23:25
仓库：`C:\Code\CodeOfC++\OpenGL_test\text2-refactor`
提交：`cda4f74`
配置：`Debug|x64`
Baseline：`docs/pbr_golden_baselines.json`
UpdateBaseline：`False`
StrictHash：`False`

## 结果

| Mode | Size | Non-black | Mean RGB | SHA256 | Result | Capture | Log |
| --- | --- | ---: | --- | --- | --- | --- | --- |
| import | 1280x720 | 100% | 160.97 / 123.68 / 83.48 | `bd56359a0ae1` | passed | `out/pbr_import_verification.ppm` | `out/pbr_verify_import.log` |
| texture-set | 1280x720 | 99.9951% | 148.27 / 114.58 / 78.64 | `be7a22e8e875` | passed | `out/pbr_texture_set_verification.ppm` | `out/pbr_verify_texture-set.log` |
| deferred-texture-set | 1280x720 | 29.2184% | 29.04 / 24.01 / 19.84 | `7d6f486c61d2` | passed | `out/pbr_deferred_texture_set_verification.ppm` | `out/pbr_verify_deferred-texture-set.log` |

## Baseline 覆盖

- `import`：真实 Assimp PBR asset probe，检查 imported mesh 与 forward PBR draw path。
- `texture-set`：真实 PBR texture set probe，检查 albedo / metallic / roughness / AO / normal 贴图链路。
- `deferred-texture-set`：同一 texture set 进入 deferred G-buffer + deferred lighting。

## 失败项

- 无

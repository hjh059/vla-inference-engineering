# Configuration ID：`a10-cuda-smolvla-20260831-r1`

> 状态：已冻结。本文记录 2026-08-31 正式基线的冻结条件，不重复维护后续实验进度；基线与后续比较见 [RESULTS.md](RESULTS.md)，profile 证据见 [PROFILE-ANALYSIS.md](PROFILE-ANALYSIS.md)。任何冻结字段改变均须创建新的 `Configuration ID`。

## 适用范围

本记录的正式目标是当前主机上的单张 NVIDIA A10/CUDA 推理。

## 冻结条件

| 字段 | 冻结值 |
|---|---|
| 模型与 checkpoint | `vrfai/smolvla-libero-gguf@349851cd4e872d199c75e1802f037c8026767190`；本地 `models/smolvla-libero.gguf`；SHA-256 `6fb2d475c98b4c2cef3e27c4eff4e67b483740cbf983fff320a3b8a5e5f74fe8` |
| 部署路径与实现 | `vla.cpp@e9e57ea734692bbcb34528fee16318e230ba1100`；Release C++/CUDA `vla-server` |
| 依赖 | CMake FetchContent `llama.cpp@7ba604f1cb61cd14898138e9abc0b4ff2601f180`，以及 vla.cpp 构建期 CUDA hook 补丁；补丁 diff SHA-256 `2cf6db3ddb175ed4968ddddee3e2401e473d66f03c004390f7deb62123f533d0` |
| 目标设备 | NVIDIA A10，23,028 MiB，Compute Capability 8.6，persistence mode Enabled；驱动 `580.126.09` |
| 主机 | Intel Xeon Platinum 8369B，8 核 / 16 逻辑 CPU，约 58 GiB RAM，无 Swap；Ubuntu 24.04.4，内核 `6.8.0-111-generic` |
| CUDA | 驱动报告 CUDA runtime 13.0；构建 Toolkit `12.8.93`（`nvcc`） |
| 基线采集时的构建目录 | `/root/vla-smoke-c01/build-formal-a10-20260831`；制品身份以本配置记录的 SHA-256 为准，目录后来可能被 profile 构建复用，不能仅凭路径认定为基线制品；不得使用 `build-cpu/` 或历史 `build-cuda/` |
| 构建选项 | Ninja、Release、`GGML_CUDA=ON`、`GGML_CUDA_GRAPHS=ON`、`CMAKE_CUDA_ARCHITECTURES=86`、`--parallel 16` |
| 输入图像 | `src/vla.cpp/assets/front.jpg`；SHA-256 `e6f0738ce9184be421dbade5a73900fb33e44c288c475d3985054fbbc4dfb7cf` |
| 文本输入 | 固定 token IDs：`1,100,200,2` |
| 状态输入 | 32 个 `0.0f`；模型实际状态维度为 8，其余是接口填充 |
| 初始 noise | 1,600 个 `float`；第 `i` 项为 `0.001 * ((uint32_t(i) * 2654435761) % 1000) - 0.5`，`i=0…1599` |
| 服务与线程 | 单进程 `vla-server`，单客户端顺序 REQ/REP，`VLA_N_THREADS=16` |

完整环境、构建缓存摘要及基线采集时的构建制品快照见 [raw/environment-20260831.md](raw/environment-20260831.md)。

## 正确性标准

在同一个已加载服务中，固定请求的 reference 输出必须为 50×32、全部有限值；每行前 7 列是有效动作，后 25 列必须为 `0.0`。其后每个测量请求的 1,600 个 `float` action 输出必须与 reference 按位相同。

该标准只验证固定输入下的数值可重复与输出结构；不主张动作语义、LIBERO 任务成功率或机器人控制正确性。

## 性能与 profile 协议

- 常驻服务加载模型一次；模型加载和服务启动不计入稳态样本。
- 固定请求先执行 5 次未计时 warm-up，再顺序采集 30 次；不排除样本。
- 主指标：客户端循环回路 wall time（包含 loopback ZeroMQ、Protobuf、JPEG decode 与服务端推理）。次指标：服务响应返回的 total、vision、inference、prefill、denoise 毫秒。
- 保存每次请求的原始值至 `results/samples.csv`、reference 输出至 `results/reference-actions.txt`、一致性判断至 `results/summary.json`，再从原始样本计算 min/p50/p90/max/mean。
- profile 使用同一服务协议、同一固定请求与相同输入/noise；profile 运行不混入 30 个正式性能样本。项目生成的 `.nsys-rep`、`.ncu-rep`、日志、导出数据及其 SHA-256 清单直接保存在本配置目录中。

## 已完成的非正式 smoke

2026-08-31 已以新 `vla-cli` 在当前主机加载该模型，日志显示 CUDA device 0（A10, CC 8.6）及 50 行 × 32 列的有限输出。该 CLI 没有传入 noise，因此本次观察仅为环境 smoke，不计入上述正确性或性能基线；正式固定请求的 smoke、正确性和性能工件已按本配置归档。

## 执行入口

[`BASELINE-RUNBOOK.md`](BASELINE-RUNBOOK.md) 是正式基线的唯一复现流程，固定了基线 checkout、构建、服务启动、客户端参数、输出位置和验收步骤。`scripts/fixed_request_client.cpp` 是本配置唯一的固定请求客户端；其编译入口为 `scripts/build-client.sh`。客户端会拒绝非 50×32 输出、非有限值或与 reference 不同的 action。

优化后只能更换明确记录的实现 revision；模型、设备、驱动、构建选项、输入、noise、线程数、warm-up、重复次数及统计方法均保持不变。

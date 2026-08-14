# C-01 CUDA smoke（2026-08-11）

> 分类：可行性 smoke，不是正式 `Configuration ID`、正确性基线或性能基线。

## 目的与结论

本记录验证 `SmolVLA-LIBERO GGUF + vla.cpp` 能在阿里云 A10/CUDA 上加载模型并完成一次 CLI 推理。相同命令独立执行两次，均得到 50 行、每行 32 个有限值；每行前 7 个值为有效动作，后 25 个值为零填充。

两次有效动作输出不相同。`vla-cli` 未提供 noise，固定 revision 的实现会从 `std::random_device` 初始化的 RNG 采样 `N(0,1)` 初始 noise。因此本记录只支持部署链路和输出结构可重复，不支持数值逐值相同、动作语义正确、LIBERO 成功率或正式性能结论。

## 固定来源与制品身份

| 项目 | 记录值 |
|---|---|
| vla.cpp | `VinRobotics/vla.cpp@7710596ecf2349a7dcbb41b4f9ce042025cbd435`；后续源码由个人 fork 管理 |
| GGUF | `vrfai/smolvla-libero-gguf@349851cd4e872d199c75e1802f037c8026767190` |
| 上游 checkpoint | `HuggingFaceVLA/smolvla_libero@6721902bc4d61e50a3bfdb11dfb4cb626f05d102` |
| 模型 | `smolvla-libero.gguf`，1,126,360,736 bytes，SHA-256 `6fb2d475c98b4c2cef3e27c4eff4e67b483740cbf983fff320a3b8a5e5f74fe8` |
| 输入图像 | vla.cpp `assets/front.jpg`，89,112 bytes，SHA-256 `e6f0738ce9184be421dbade5a73900fb33e44c288c475d3985054fbbc4dfb7cf` |
| 可执行文件 | `build-cuda/vla-cli`，155,168 bytes，SHA-256 `126c401f92252e5f3e688c70815ccf2da2346a59423eabdf284ff6dddd3e4579` |
| 许可证状态 | vla.cpp、上游和 GGUF 模型卡均标注 Apache-2.0；GGUF 模型卡中的运行时许可文字与仓库不一致，发布或分发前仍需复核 |

模型和可执行文件仅记录身份，不提交本仓库。

## 已知构建与运行条件

项目负责人已补充当时执行的依赖安装、CMake 构建和证据采集命令；当前工作区确认其源目录、变量值、并行度及解析后的 CMake 配置。历史命令、补齐依据、当前软件包版本和后续显式复现形式见 [`commands.md`](commands.md)。构建缓存确认以下选项：Release、Ninja、`GGML_CUDA=ON`、`GGML_CUDA_GRAPHS=ON`、`CMAKE_CUDA_ARCHITECTURES=86`；其中 CUDA graphs 没有在历史配置命令中显式传入，但解析后的缓存值为 `ON`。工具链和设备原始记录见 [`raw/environment.txt`](raw/environment.txt)。

两次日志记录的实际运行命令相同：

```bash
timeout 10m /root/vla-smoke-c01/build-cuda/vla-cli \
  --ckpt /root/vla-smoke-c01/models/smolvla-libero.gguf \
  --image /root/vla-smoke-c01/src/vla.cpp/assets/front.jpg \
  --tokens 1,100,200,2 \
  --pretty
```

CLI 将未提供的 state 补为默认零状态；未提供 noise，因而每个进程使用新采样的初始 noise。

## 原始证据与复核结果

| 文件 | 用途 |
|---|---|
| [`raw/environment.txt`](raw/environment.txt) | 当时的环境、revision 和制品 SHA-256 |
| [`raw/run-1.log`](raw/run-1.log) | 第一次完整 stdout/stderr 与 `/usr/bin/time -v` 输出 |
| [`raw/run-2.log`](raw/run-2.log) | 第二次完整 stdout/stderr 与 `/usr/bin/time -v` 输出 |
| [`raw/run-2-exit-status.txt`](raw/run-2-exit-status.txt) | 第二次命令退出状态 |
| [`verification.json`](verification.json) | 对原始日志重新计算的结构与差异摘要 |
| [`commands.md`](commands.md) | 历史环境准备、构建、校验和采集命令及当前工作区补齐值 |

原始日志 SHA-256 和未提交制品的身份统一记录在 [`artifact-manifest.json`](artifact-manifest.json) 中。

两次进程墙钟时间分别约 1.79 s 和 1.76 s，包含进程启动、模型加载与推理；它们不是稳态延迟样本，也不进入正式性能基线。

## 未解决项

- 正式实验必须固定 initial noise 或明确随机种子/样本协议，并在查看优化结果前冻结数值比较方式和容差；
- 必须使用模型只加载一次、同一进程重复请求的入口确定预热和稳态计时边界；
- 2026-08-11 安装时的 apt 软件包精确版本没有原始输出；当前工作区版本只作为补充快照；
- 环境 profiling smoke 的 `.ncu-rep`、`.nsys-rep` 和最小 kernel 源码未包含在本记录中，仍需单独归档或标记为不可恢复。

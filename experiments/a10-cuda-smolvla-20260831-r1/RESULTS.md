# 实验结果：`a10-cuda-smolvla-20260831-r1`

> 基线采集日期：2026-08-31。本文只维护无 profiler 条件下的正式正确性与性能基线，以及之后的同条件优化前后比较。2026-09-01 的 profile 采集、分析与结论边界见 [PROFILE-ANALYSIS.md](PROFILE-ANALYSIS.md)。所有结论只适用于本文件记录的冻结模型、输入、固定 noise、A10、驱动、单客户端和测量协议。

## 正确性基线

常驻 `vla-server` 在模型加载后接受同一固定 JPEG、token、32 维零状态和 1,600 维固定 noise。执行 5 次未计时 warm-up 后采集 30 次顺序请求。

| 检查 | 结果 |
|---|---|
| 输出形状 | 30 次均为 50×32 |
| 有限性 | 30 次均无 `NaN`/`Inf` |
| 填充契约 | reference 输出每行后 25 列均为 `0.0` |
| 数值一致性 | 29 个后续样本均与第一个计时样本的 1,600 个 `float` action 值按位相同；客户端摘要为 `all_bitwise_equal_reference=true` |

这只建立固定输入下的结构与数值可重复性，不支持动作语义、LIBERO 成功率或机器人控制正确性结论。

## 稳态性能基线

统计来自 `results/samples.csv` 的全部 30 个样本；未排除样本。单位均为 ms。

| 指标 | min | p50 | p90 | max | mean | stddev |
|---|---:|---:|---:|---:|---:|---:|
| 客户端端到端 wall time | 69.994 | 70.499 | 71.665 | 72.232 | 70.751 | 0.623 |
| 服务端 total | 67.285 | 67.786 | 68.946 | 69.506 | 68.027 | 0.609 |
| 服务端 vision | 18.696 | 18.749 | 18.895 | 19.184 | 18.793 | 0.108 |
| 服务端 inference | 48.062 | 48.494 | 49.646 | 50.288 | 48.735 | 0.588 |

按均值分解，服务端 inference 占 total 约 71.6%，vision 约 27.6%，其余服务端开销约 0.5 ms；客户端 loopback 协议、序列化和请求边界相对服务端 total 的均值差约 2.723 ms。这是端到端分解观察，不是主瓶颈归因。

## 优化前后比较：优化 01 原生 BF16 GEMM

优化 01 移除了 SmolVLA VLM/action-expert 矩阵乘的默认 `GGML_PREC_F32` 强制，使 BF16 权重使用原生 BF16 GEMM；设定 `VLA_MM_PREC=f32` 可回退至旧路径。变更详情、源码身份、机制、风险和未归档工件见 [OPTIMIZATION-01.md](OPTIMIZATION-01.md)，按既有通用流程重跑时的差异见其[复现步骤](OPTIMIZATION-01.md#复现步骤)。

优化后使用相同模型、A10、驱动、CUDA、构建选项、单客户端、`VLA_N_THREADS=16`、固定 JPEG/token/state/noise、5 次预热、30 次顺序样本和相同统计方法。原始样本和摘要位于 [`results/optimization-01-native-bf16/`](results/optimization-01-native-bf16/)，字节身份见 [`raw/optimization-01-artifacts.sha256`](raw/optimization-01-artifacts.sha256)。单位均为 ms，未排除样本。

| 指标 | 基线 mean | 优化后 mean | 变化 | 基线 p50 | 优化后 p50 | 基线 p90 | 优化后 p90 |
|---|---:|---:|---:|---:|---:|---:|---:|
| 客户端端到端 wall time | 70.751 | 57.476 | -18.76% | 70.499 | 57.380 | 71.665 | 57.823 |
| 服务端 total | 68.027 | 54.789 | -19.46% | 67.786 | 54.684 | 68.946 | 55.143 |
| 服务端 vision | 18.793 | 18.625 | -0.89% | 18.749 | 18.621 | 18.895 | 18.678 |
| 服务端 inference | 48.735 | 35.707 | -26.73% | 48.494 | 35.585 | 49.646 | 36.001 |

优化后的固定请求仍满足当前配置的正确性标准：30 次输出均为 50×32、有限，后续 29 次均与该次运行的 reference action 按位一致。它不等价于跨实现数值等价：和基线 reference 的文本比较中，1,600 个值有 350 个在打印精度下不同，最大绝对差约为 `0.002875342`。因此上述结果只能说明固定输入下的可重复性与延迟变化，不能支持动作语义、LIBERO 成功率或真实机器人控制正确性不变的结论。

## 工件索引

基线的原始样本、摘要和 reference action 位于 `results/`，字节身份由 [`raw/baseline-artifacts.sha256`](raw/baseline-artifacts.sha256) 维护。优化 01 的小型原始结果和 reference capture 位于 `results/optimization-01-native-bf16/`，字节身份由 [`raw/optimization-01-artifacts.sha256`](raw/optimization-01-artifacts.sha256) 维护。基线 profile 的原始工件索引、用途和校验和见 [PROFILE-ANALYSIS.md](PROFILE-ANALYSIS.md) 与 [`raw/vla-steady-r2/README.md`](raw/vla-steady-r2/README.md)；优化后大型 profile 工件尚未归档。

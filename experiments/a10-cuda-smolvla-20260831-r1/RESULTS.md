# 基线结果：`a10-cuda-smolvla-20260831-r1`

> 采集日期：2026-08-31。所有结论只适用于本配置冻结的模型、输入、固定 noise、A10、驱动、构建产物、单客户端和测量协议。

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

## 历史 profile 工件

已生成 `raw/nsys-baseline.nsys-rep`（1.6 MiB，SHA-256 `c493d2eeb4c5fa00bb8fa88081c803a9b4d98dc655037ae689c0e356ca107a83`）和由 Nsight Systems 导出的 `raw/nsys-baseline.sqlite`（4.2 MiB，SHA-256 `3dbd9a5c1f21ca9c6e46f39b3325a6dd2f069799eb33bff24c16c8a19cd0da9c`）。

该 trace 覆盖模型加载、5 次 warm-up 和 1 次固定请求；因而 CUDA API 与 kernel 汇总混入初始化、CUDA graph 建立和稳态请求，**不能单独用于确认稳态主瓶颈**。它只证明当前环境可生成并解析模型 trace。

## 隔离稳态 profile：`vla-steady-r2`

采集日期：2026-09-01。此 profile 保持冻结的模型、固定 JPEG、token、32 维零状态、固定 noise、单客户端顺序 REQ/REP 与 `VLA_N_THREADS=16`。为建立可切片的请求边界，profile 使用加入 NVTX 观测标记的派生 CUDA `vla-server` 构建；其二进制 SHA-256 为 `7025bf3df2afc9213ac26c744f18e2ec7bfdc6564f64ca6a7fa79114a62cf124`。该构建仅用于 profile，不能替代正式 30 样本性能基线的冻结构建产物。

服务加载完成后，在 collection 外执行 5 次 warm-up；开始 Nsight Systems collection 后，额外发送 2 个丢弃请求以消除 collection 启动阶段的扰动，再发送 10 个正式固定请求。`vla.profile` domain 为每个请求记录 `request rid=<id>`，并为模型预测记录嵌套 `predict rid=<id>`。

| 检查 | 结果 |
|---|---|
| NVTX 范围完整性 | 12 个 `request` 与 12 个 `predict` 范围均闭合，且 12 个 `predict` 均完全嵌套于同 ID 的 `request` |
| 丢弃范围 | `rid=1–2`；request 时长分别为 111.406 ms、178.478 ms，不参与稳态汇总 |
| 正式范围 | `rid=3–12`，共 10 次 |
| 正确性 | 10 次均为 50×32、无 `NaN`/`Inf`，并与本次 reference action 按位一致 |
| 正式 request 时长 | 76.628–85.040 ms；这是 profile 观测值，不是性能基线 |
| CUDA Runtime 与 kernel 覆盖 | `rid=3–12` 每次均有 36 个 CUDA Runtime 调用与 11,384 个 kernel；范围内 kernel 时间为 64.603–66.780 ms |

该 profile 已满足“稳定请求可精确切片”的证据要求。后续分析只使用 `predict rid=3–12`，不得混入 collection 启动阶段的 `rid=1–2`。

### 范围内汇总与瓶颈假设

下表来自 `nsys.sqlite` 中与 `predict rid=3–12` 相交的 CUDA Runtime 和 kernel 活动；GPU busy 是同一请求内 GPU kernel 时间区间的并集，不重复累计重叠 stream 的时间。

| 指标 | 10 次正式范围的结果 | 解释 |
|---|---:|---|
| `predict` 时长 | 均值 77.960 ms；范围 74.022–82.493 ms | profile 观测值，不能替代正式性能基线 |
| GPU unique busy | 均值 65.261 ms，占 `predict` 的 83.87% | 主要时间在 GPU kernel 执行，而不是请求协议或 CPU 空闲 |
| `cudaGraphLaunch` | 每请求 3 次；CPU API 时间均值 11.787 ms | 与 GPU 执行重叠，不能同 GPU busy 相加 |
| `cudaStreamSynchronize` | 每请求 18 次；CPU API 时间均值 61.739 ms | 主要表示主机等待已提交的 GPU 工作，不是独立 CPU 计算热点 |

按 kernel `shortName` 聚合的范围内 GPU 时间前六项如下。百分比以所有 kernel 累积时间为分母；它不等同于 wall time，stream 并发时不能与上表 GPU busy 直接相加。

| kernel 类别 | GPU 时间占比 |
|---|---:|
| CUTLASS `Kernel2` GEMM | 28.55% |
| `convert_unary` | 27.52% |
| `soft_max_f32` | 8.03% |
| `splitKreduce_kernel` | 7.38% |
| `k_bin_bcast` | 6.96% |
| `flash_attn_ext_f16` | 6.67% |

因此，本轮 **确认的主要瓶颈层级** 是 `vla::predict()` 内的 GPU kernel 执行；CPU 端同步是在等待这些 GPU 工作完成。尚未确认的更细粒度根因是：BF16 权重与 F32 激活之间的转换是否是最具性价比的优化点。代码检查显示 SmolVLA 默认将权重解析为 BF16，而主图中的输入和若干投影仍为 F32；通用 BF16 activation helper 说明，标准 `ggml_mul_mat` 会在 BF16 权重与 F32 激活组合中引入 F32↔BF16 转换。该机制与 `convert_unary` 的 27.52% 聚合热点一致，但仍只是 **可证伪假设**，不能据此直接修改模型精度路径。

### Nsight Compute 首次尝试：未采到 kernel

采集日期：2026-09-01。已对 `convert_unary` 执行一次 NCU launch/attach 尝试。NCU 成功连接 `vla-server`；固定客户端的 5 次 warm-up 加 1 次请求仍输出 50×32 有限值，并与 reference action 按位一致。该请求的客户端 wall time 为 230.744 ms、服务端 inference 为 204.930 ms，均受 NCU 扰动，**不得**用于正式性能基线或优化比较。

本次没有生成 `.ncu-rep`，且 NCU 日志报告“没有 kernel 被采集”。日志给出两个待修正条件：启动时的 `--target-processes application-only` 未覆盖发起 kernel 的子进程；`vla.profile@predict rid=6` 被解释为 start/end 范围，但服务端实际通过 `nvtxDomainRangePushEx/Pop` 创建 push/pop 范围。故本次只证明 launch/attach 连接与模型请求正确性，**不提供任何硬件限制或细粒度根因结论**。

下次 NCU 采集应从 fresh server 开始，并使用 `--target-processes all`、`--nvtx-include 'vla.profile@predict rid=6/'` 和 `--nvtx-push-pop-scope process`；后两项分别匹配实际 push/pop 语义并将范围关联扩展到进程作用域。该修正命令已记录在[执行计划](../../docs/plans/phase-0.md#nsight-compute-细粒度验证方案)，但尚未以模型工作负载验证。

下一步应在修正上述 NCU 条件后，对稳定范围内最耗时的 `convert_unary` 和代表性 CUTLASS `Kernel2` GEMM 分别采集一次 Nsight Compute，确认前者是否受全局内存带宽限制、后者是否受计算吞吐/形状限制；随后仅选择其中一个有硬件证据支持且能保持固定输出正确性的最小优化。

`CUDA_GRAPH_NODE_EVENTS` 已导出，但其事件时长均为 0，不能用于 node 时间汇总；本轮以 NVTX 范围内的 CUDA Runtime 和 `CUPTI_ACTIVITY_KIND_KERNEL` 时间为准。

| 已归档工件 | SHA-256 | 仓库路径 |
|---|---|---|
| Nsight Systems 报告 | `a75a1c62b4397fa1fb56eea6a4e335967c215f3e2a36366bea6a02bc13b827e2` | [`raw/vla-steady-r2/nsys.nsys-rep`](raw/vla-steady-r2/nsys.nsys-rep) |
| 导出 SQLite | `b8998f238099dbb6687b7fcd8216f50ce3e5c28a0c782b1475de7b4b0c55d129` | [`raw/vla-steady-r2/nsys.sqlite`](raw/vla-steady-r2/nsys.sqlite) |
| 正式请求样本 | `8dd0f27c992afe41ed69bbee0e44d4462341593262a2601b82fd0bb3f481f7be` | [`raw/vla-steady-r2/samples.csv`](raw/vla-steady-r2/samples.csv) |
| NCU `convert_unary` 失败尝试日志（无 `.ncu-rep`） | `b1bcd81af63dc411b47ebf1990ce482499f2cd5b4664d2b431d715213308b36f` | [`raw/vla-steady-r2/ncu-convert-rid6.log`](raw/vla-steady-r2/ncu-convert-rid6.log) |
| NCU 尝试的正确性摘要 | `14ffa8156c4df6d68886160b8f79bf82f3ed9a6db53644650fea43f54dfa6622` | [`raw/vla-steady-r2/ncu-convert-client/summary.json`](raw/vla-steady-r2/ncu-convert-client/summary.json) |

## 工件

| 工件 | SHA-256 |
|---|---|
| 固定请求客户端 | `7554c4f7d5532eeb64e740a821ca8e96960aadbafaed4874d5fd917f271a0267` |
| 逐请求样本 | `513a9d79297312049239c59e5277e49b1b79ecbfa60287bfef5a0dfafee983ab` |
| reference actions | `f86a912942bfe5b8777c0c039a824a509468dcec94f21ffe0b7c9a852bbf77ab` |
| 正确性摘要 | `a6a0b3e3dc12a16a251e6f83fd5f08c8fc57d9baac0d9151d7d9d428903d9e21` |

原始文本日志、CSV、摘要、reference actions、二进制 `.nsys-rep` 与派生 `.sqlite` 均已随本配置保存在仓库中；完整索引、大小与校验和见 [`raw/vla-steady-r2/`](raw/vla-steady-r2/) 和 `raw/profile-artifacts.sha256`。

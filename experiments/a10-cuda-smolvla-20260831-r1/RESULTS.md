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

## Profile 工件

已生成 `raw/nsys-baseline.nsys-rep`（1.6 MiB，SHA-256 `c493d2eeb4c5fa00bb8fa88081c803a9b4d98dc655037ae689c0e356ca107a83`）和由 Nsight Systems 导出的 `raw/nsys-baseline.sqlite`（4.2 MiB，SHA-256 `3dbd9a5c1f21ca9c6e46f39b3325a6dd2f069799eb33bff24c16c8a19cd0da9c`）。

该 trace 覆盖模型加载、5 次 warm-up 和 1 次固定请求；因而 CUDA API 与 kernel 汇总混入初始化、CUDA graph 建立和稳态请求，**不能单独用于确认稳态主瓶颈**。它只证明当前环境可生成并解析模型 trace。后续做主瓶颈判断前，应采集只覆盖稳定请求范围的 trace 或 Compute 报告，并与本基线配置保持相同条件。

## 工件

| 工件 | SHA-256 |
|---|---|
| 固定请求客户端 | `7554c4f7d5532eeb64e740a821ca8e96960aadbafaed4874d5fd917f271a0267` |
| 逐请求样本 | `513a9d79297312049239c59e5277e49b1b79ecbfa60287bfef5a0dfafee983ab` |
| reference actions | `f86a912942bfe5b8777c0c039a824a509468dcec94f21ffe0b7c9a852bbf77ab` |
| 正确性摘要 | `a6a0b3e3dc12a16a251e6f83fd5f08c8fc57d9baac0d9151d7d9d428903d9e21` |

原始文本日志、CSV、摘要、reference actions、二进制 `.nsys-rep` 与派生 `.sqlite` 均已随本配置保存；后两者的索引和校验和见本页及 `raw/profile-artifacts.sha256`。后续新增的大型 profile 工件按实验工件规则使用 Git LFS 或外部存储。

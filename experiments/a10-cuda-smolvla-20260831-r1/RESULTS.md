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

该 profile 已满足“稳定请求可精确切片”的证据要求。范围内 GPU kernel 汇总显示 `convert_unary<__nv_bfloat16, float>` 与 CUTLASS BF16 GEMM `64x64x16` 是当前最大的两个 kernel 类别（约 24% 与 19% 的范围内 kernel 时间）；这只是待验证的热点观察，**尚未构成主要瓶颈或优化结论**。后续必须只针对 `predict rid=3–12` 汇总 kernel/CUDA API，并结合必要的 Nsight Compute 证据提出可证伪的主瓶颈判断。

`CUDA_GRAPH_NODE_EVENTS` 已导出，但其事件时长均为 0，不能用于 node 时间汇总；本轮以 NVTX 范围内的 CUDA Runtime 和 `CUPTI_ACTIVITY_KIND_KERNEL` 时间为准。

| 已归档工件 | SHA-256 | 仓库路径 |
|---|---|---|
| Nsight Systems 报告 | `a75a1c62b4397fa1fb56eea6a4e335967c215f3e2a36366bea6a02bc13b827e2` | [`raw/vla-steady-r2/nsys.nsys-rep`](raw/vla-steady-r2/nsys.nsys-rep) |
| 导出 SQLite | `b8998f238099dbb6687b7fcd8216f50ce3e5c28a0c782b1475de7b4b0c55d129` | [`raw/vla-steady-r2/nsys.sqlite`](raw/vla-steady-r2/nsys.sqlite) |
| 正式请求样本 | `8dd0f27c992afe41ed69bbee0e44d4462341593262a2601b82fd0bb3f481f7be` | [`raw/vla-steady-r2/samples.csv`](raw/vla-steady-r2/samples.csv) |

## 工件

| 工件 | SHA-256 |
|---|---|
| 固定请求客户端 | `7554c4f7d5532eeb64e740a821ca8e96960aadbafaed4874d5fd917f271a0267` |
| 逐请求样本 | `513a9d79297312049239c59e5277e49b1b79ecbfa60287bfef5a0dfafee983ab` |
| reference actions | `f86a912942bfe5b8777c0c039a824a509468dcec94f21ffe0b7c9a852bbf77ab` |
| 正确性摘要 | `a6a0b3e3dc12a16a251e6f83fd5f08c8fc57d9baac0d9151d7d9d428903d9e21` |

原始文本日志、CSV、摘要、reference actions、二进制 `.nsys-rep` 与派生 `.sqlite` 均已随本配置保存在仓库中；完整索引、大小与校验和见 [`raw/vla-steady-r2/`](raw/vla-steady-r2/) 和 `raw/profile-artifacts.sha256`。

# `vla-steady-r2`：隔离稳态 profile 工件

采集于 2026-09-01，用于正式配置 `a10-cuda-smolvla-20260831-r1` 的稳定请求范围切片。完整条件、范围验收、关键数据和结论边界见上级目录的 [PROFILE-ANALYSIS.md](../../PROFILE-ANALYSIS.md)；观测构建、Nsight Systems 和 Nsight Compute 的复现入口分别见 [PROFILE-BUILD.md](../../PROFILE-BUILD.md)、[NSYS-RUNBOOK.md](../../NSYS-RUNBOOK.md) 和 [NCU-RUNBOOK.md](../../NCU-RUNBOOK.md)。

服务加载完成后在 collection 外执行 5 次 warm-up；collection 开始后，`rid=1–2` 是丢弃请求，`rid=3–12` 是用于后续 kernel/CUDA API 汇总的 10 个正式稳定请求。每个请求包含 `vla.profile` NVTX domain 下的 `request rid=<id>` 和嵌套 `predict rid=<id>` 范围。

## Nsight Systems 与请求证据

| 工件 | 大小（bytes） | 用途 |
|---|---:|---|
| `nsys.nsys-rep` | 6,861,051 | 原始 Nsight Systems 报告 |
| `nsys.sqlite` | 18,415,616 | Nsight Systems 导出的可查询数据库 |
| `samples.csv` | 671 | 10 个正式请求的原始客户端与服务端时间 |
| `summary.json` | 121 | 本次固定请求的形状、有限性和按位一致性摘要 |
| `reference-actions.txt` | 7,258 | 本次固定请求的 reference action |

## NCU `convert_unary`

| 工件 | 大小（bytes） | 用途 |
|---|---:|---|
| `ncu-convert-rid6.ncu-rep` | 508,999 | NCU `convert_unary` 的 18-pass `detailed` 硬件计数器报告 |
| `ncu-convert-rid6.log` | 609 | NCU `convert_unary` 采集日志 |
| `ncu-convert-client/samples.csv` | 190 | `convert_unary` 采集期间的唯一计时请求；仅作正确性/执行记录，不可作性能比较 |
| `ncu-convert-client/summary.json` | 120 | `convert_unary` 采集期间的形状、有限性和按位一致性摘要 |
| `ncu-convert-client/reference-actions.txt` | 7,258 | `convert_unary` 采集使用的 reference action |

## NCU CUTLASS `Kernel2` GEMM

| 工件 | 大小（bytes） | 用途 |
|---|---:|---|
| `ncu-gemm-rid6.ncu-rep` | 9,011,541 | NCU CUTLASS `Kernel2` GEMM 的 18-pass `detailed` 硬件计数器报告 |
| `ncu-gemm-rid6.log` | 595 | NCU CUTLASS `Kernel2` GEMM 采集日志 |

两个 NCU 报告都以 `vla.profile@predict rid=6/` 的 push/pop 范围过滤目标 kernel。硬件计数器结论、适用边界及后续工作见上级 [PROFILE-ANALYSIS.md](../../PROFILE-ANALYSIS.md)。所有 profile 时间仅支持热点与硬件限制分析，不是正式性能基线，也不单独支持优化结论。全部 profile 工件的字节身份只在 [`../profile-artifacts.sha256`](../profile-artifacts.sha256) 中维护。

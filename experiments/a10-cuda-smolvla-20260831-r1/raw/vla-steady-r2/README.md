# `vla-steady-r2`：隔离稳态 Nsight Systems 工件

采集于 2026-09-01，用于正式配置 `a10-cuda-smolvla-20260831-r1` 的稳定请求范围切片。完整条件、范围验收、关键数据和结论边界见上级目录的 [RESULTS.md](../../RESULTS.md)。

服务加载完成后在 collection 外执行 5 次 warm-up；collection 开始后，`rid=1–2` 是丢弃请求，`rid=3–12` 是用于后续 kernel/CUDA API 汇总的 10 个正式稳定请求。每个请求包含 `vla.profile` NVTX domain 下的 `request rid=<id>` 和嵌套 `predict rid=<id>` 范围。

| 工件 | 用途 | SHA-256 |
|---|---|---|
| `nsys.nsys-rep` | 原始 Nsight Systems 报告 | `a75a1c62b4397fa1fb56eea6a4e335967c215f3e2a36366bea6a02bc13b827e2` |
| `nsys.sqlite` | Nsight Systems 导出的可查询数据库 | `b8998f238099dbb6687b7fcd8216f50ce3e5c28a0c782b1475de7b4b0c55d129` |
| `samples.csv` | 10 个正式请求的原始客户端与服务端时间 | `8dd0f27c992afe41ed69bbee0e44d4462341593262a2601b82fd0bb3f481f7be` |
| `summary.json` | 本次固定请求的形状、有限性和按位一致性摘要 | `c632d6079708d67cf950a8199393e5b8229e8d25ff55b2b966c0e2fd83f042ec` |
| `reference-actions.txt` | 本次固定请求的 reference action | `f86a912942bfe5b8777c0c039a824a509468dcec94f21ffe0b7c9a852bbf77ab` |

这些 profile 时间仅支持热点与等待关系分析，不是正式性能基线，也尚未单独支持主要瓶颈或优化结论。

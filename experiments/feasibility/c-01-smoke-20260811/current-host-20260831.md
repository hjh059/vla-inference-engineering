# 当前主机资源快照（2026-08-31）

> 采集时间：`2026-08-31T10:02:38+08:00`。本文件记录当前工作区主机的资源状态，供后续创建正式 `Configuration ID` 时核对；不是 2026-08-11 C-01 CUDA smoke 的原始环境证据，也不构成性能测量。

## 系统

```text
Linux iZf8zdhlngbp3arkmh70ooZ 6.8.0-111-generic #111-Ubuntu SMP PREEMPT_DYNAMIC Sat Apr 11 23:16:02 UTC 2026 x86_64 x86_64 x86_64 GNU/Linux
```

## CPU

| 项目 | 值 |
|---|---|
| 型号 | `Intel(R) Xeon(R) Platinum 8369B CPU @ 2.90GHz` |
| 架构 | `x86_64` |
| 逻辑 CPU / `nproc` | `16` |
| 拓扑 | 1 socket，8 cores，2 threads per core |
| NUMA | 1 node（CPU `0-15`） |
| L3 cache | 48 MiB |
| 虚拟化 | KVM full virtualization |

## 内存

| 项目 | 值 |
|---|---|
| 总内存 | 62,794,043,392 bytes（约 58 GiB） |
| 采集时可用内存 | 61,009,412,096 bytes（约 56.8 GiB） |
| Swap | 0 bytes |

## 解释边界

- 旧的 `nproc=8` 是 2026-08-14 工作区快照值；它及当前值都不能反向证明 2026-08-11 历史构建的实际并行度。
- 若建立正式性能基线，必须在同一次实验中重新采集完整主机、GPU/驱动、CUDA、软件版本、命令和制品校验和；CPU/内存变化后的结果不得与不同主机配置的结果进行优化前后比较。

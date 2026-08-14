# 阿里云 A10 环境与 profiling 可行性记录

> 日期：2026-08-09。
>
> 证据来源：项目负责人提供的云实例执行记录。本文只证明候选环境能够运行 CUDA 与 profiling 工具，不构成模型部署、正确性或性能基线。
>
> 后续状态：2026-08-11，C-01 已在同一类 A10 环境通过重复模型 smoke；模型路径的最新证据与结论边界以[路径选择记录](issue-candidates.md#c-01smolvlalibero-gguf--vlacpp)为准。本文下方的 `Unknown` 只表示 2026-08-09 环境检查结束时的状态。

## 实例与软件环境

| 项目 | 已验证值 | 证据 |
|---|---|---|
| 实例 | 阿里云 `gn7i`，`ecs.gn7i-c8g1.2xlarge` | [阿里云官方规格](https://help.aliyun.com/en/ecs/user-guide/gpu-accelerated-compute-optimized-and-vgpu-accelerated-instance-families-1)列出 8 vCPU、30 GiB、NVIDIA A10 × 1、24 GB × 1；项目负责人确认所用实例规格 |
| GPU | NVIDIA A10，`23028 MiB`，功耗上限 `150 W` | 项目负责人提供的 `nvidia-smi` 检查结果；[NVIDIA A10 数据表](https://www.nvidia.com/content/dam/en-zz/Solutions/Data-Center/a10/pdf/datasheet-new/nvidia-a10-datasheet.pdf)确认 Ampere、24 GB GDDR6 和 150 W 规格 |
| 操作系统 | Ubuntu 24.04 | 项目负责人提供的实例检查结果 |
| NVIDIA 驱动 | `580.126.09` | 项目负责人提供的实例检查结果 |
| CUDA Toolkit | `12.8` | 项目负责人提供的 Toolkit 检查结果；`nvidia-smi` 显示的 `CUDA Version: 13.0` 只表示驱动支持的最高 CUDA 版本 |
| Python | `3.12.3` | 项目负责人提供的实例检查结果 |
| Docker | `29.1.3` | 项目负责人提供的实例检查结果 |
| KeenTune | 关闭 | 项目负责人提供的实例配置记录 |

实例规格与实测显存均表明当前资源是完整 A10，不是 `sgn7i`/`vgn7i` 的部分 GPU 切片。

## Profiling smoke test

工具版本：

- Nsight Systems `2024.6.2`；
- Nsight Compute `2025.1.1.0`。

项目负责人已编译并执行最小 CUDA kernel：

```text
/root/profile-smoke-20260809ncu_test
```

Nsight Compute 完成 `fill(float *, int)` 的 8 个采集 pass，未出现 `ERR_NVGPUCTRPERM`，生成：

```text
/root/profile-smoke-20260809ncu_profile.ncu-rep
```

Nsight Systems 成功采集 CUDA API 与 GPU kernel 时间线，生成：

```text
/root/profile-smoke-20260809nsys_test.nsys-rep
```

本次 smoke test 的观察值：

| 事件 | 观察值 |
|---|---:|
| `cudaMalloc` | `92.045 ms` |
| `cudaLaunchKernel` | `0.381 ms` |
| `cudaDeviceSynchronize` | `0.006 ms` |
| `fill` kernel | `6.528 μs` |

这些数值只验证工具链与权限可用。`cudaMalloc` 包含首次 CUDA 上下文初始化成本；所有观察值均不得作为本项目的模型性能基线。

## 证据结论与保留项

- `Locally reproduced`：该受控云环境能够编译并运行最小 CUDA 程序；
- `Profiled`：Nsight Systems 能导出系统级 trace，Nsight Compute 能采集 kernel 硬件计数器；
- `Decision`：该环境满足候选 VLA 路径的部署、测量和 GPU profiling 前提；
- `Unknown`（截至 2026-08-09）：当时尚未验证具体模型/checkpoint 的显存占用、兼容性、正确性和性能；后续 C-01 smoke 只消除了其中的部署兼容性与输出结构不确定性，正式正确性和性能仍未建立。

截至本次检查结束时，原始 `.ncu-rep` 和 `.nsys-rep` 位于云实例的上述路径；当前可访问性尚未复核，且这些文件、最小 kernel 源码和完整复现命令均未纳入主项目仓库。若能恢复，大型报告应使用 Git LFS 或外部存储，并在 Git 中保存索引、大小和 SHA-256；若无法恢复，必须明确标记为不可复查的证据缺口，不能仅凭本摘要形成模型 profile 结论。
